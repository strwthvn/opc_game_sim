#include <simulation/PhysicsThread.h>
#include <simulation/PhysicsComponents.h>
#include <core/Components.h>
#include <spdlog/spdlog.h>

#include <chrono>

namespace simulation {

PhysicsThread::PhysicsThread(PhysicsWorld& world, PhysicsSystem& physicsSystem, entt::registry& registry)
    : m_world(world)
    , m_physicsSystem(physicsSystem)
    , m_registry(registry)
{
}

PhysicsThread::~PhysicsThread()
{
    stop();
}

bool PhysicsThread::start()
{
    // Проверяем, не запущен ли уже поток
    if (m_running.load(std::memory_order_acquire)) {
        spdlog::warn("PhysicsThread: already running");
        return false;
    }

    m_running.store(true, std::memory_order_release);
    m_paused.store(false, std::memory_order_release);
    m_stepCount.store(0, std::memory_order_relaxed);
    m_averageStepTimeMs.store(0.0f, std::memory_order_relaxed);

    m_thread = std::thread(&PhysicsThread::physicsLoop, this);

    spdlog::info("PhysicsThread: started (60 Hz fixed timestep)");
    return true;
}

void PhysicsThread::stop()
{
    if (!m_running.load(std::memory_order_acquire)) {
        return;
    }

    spdlog::info("PhysicsThread: stopping...");

    m_running.store(false, std::memory_order_release);

    // Разбудить поток, если он на паузе
    {
        std::lock_guard<std::mutex> lock(m_pauseMutex);
        m_paused.store(false, std::memory_order_release);
    }
    m_pauseCondition.notify_all();

    // Ждём завершения потока
    if (m_thread.joinable()) {
        m_thread.join();
    }

    spdlog::info("PhysicsThread: stopped (total steps: {})", m_stepCount.load());
}

void PhysicsThread::pause()
{
    m_paused.store(true, std::memory_order_release);
    spdlog::debug("PhysicsThread: paused");
}

void PhysicsThread::resume()
{
    {
        std::lock_guard<std::mutex> lock(m_pauseMutex);
        m_paused.store(false, std::memory_order_release);
    }
    m_pauseCondition.notify_all();
    spdlog::debug("PhysicsThread: resumed");
}

void PhysicsThread::setExceptionHandler(std::function<void(const std::exception&)> callback)
{
    std::lock_guard<std::mutex> lock(m_exceptionHandlerMutex);
    m_exceptionHandler = std::move(callback);
}

void PhysicsThread::physicsLoop()
{
    spdlog::debug("PhysicsThread: entering physics loop");

    using Clock = std::chrono::high_resolution_clock;
    using Duration = std::chrono::duration<float>;

    constexpr auto targetDuration = std::chrono::microseconds(static_cast<int>(FIXED_TIMESTEP * 1'000'000));

    // Скользящее среднее для времени шага
    constexpr float smoothingFactor = 0.1f;
    float smoothedStepTime = 0.0f;

    while (m_running.load(std::memory_order_acquire)) {
        // Обработка паузы
        if (m_paused.load(std::memory_order_acquire)) {
            std::unique_lock<std::mutex> lock(m_pauseMutex);
            m_pauseCondition.wait(lock, [this]() {
                return !m_paused.load(std::memory_order_acquire) ||
                       !m_running.load(std::memory_order_acquire);
            });

            // Если разбудили для остановки, выходим
            if (!m_running.load(std::memory_order_acquire)) {
                break;
            }
        }

        auto frameStart = Clock::now();

        try {
            doPhysicsStep();
        } catch (const std::exception& e) {
            spdlog::error("PhysicsThread: exception during physics step: {}", e.what());

            // Вызываем пользовательский обработчик
            std::lock_guard<std::mutex> lock(m_exceptionHandlerMutex);
            if (m_exceptionHandler) {
                m_exceptionHandler(e);
            }
        }

        auto frameEnd = Clock::now();
        auto frameDuration = std::chrono::duration_cast<std::chrono::microseconds>(frameEnd - frameStart);

        // Обновляем статистику
        float stepTimeMs = frameDuration.count() / 1000.0f;
        smoothedStepTime = smoothedStepTime * (1.0f - smoothingFactor) + stepTimeMs * smoothingFactor;
        m_averageStepTimeMs.store(smoothedStepTime, std::memory_order_relaxed);

        // Спим оставшееся время до следующего кадра
        if (frameDuration < targetDuration) {
            auto sleepDuration = targetDuration - frameDuration;
            std::this_thread::sleep_for(sleepDuration);
        } else {
            // Кадр занял больше времени, чем целевой период
            spdlog::trace("PhysicsThread: frame took {:.2f}ms (target: {:.2f}ms)",
                         stepTimeMs, FIXED_TIMESTEP * 1000.0f);
        }
    }

    spdlog::debug("PhysicsThread: exiting physics loop");
}

void PhysicsThread::doPhysicsStep()
{
    if (m_useDoubleBuffering.load(std::memory_order_acquire)) {
        // С double buffering: минимизируем время удержания мьютекса
        {
            std::lock_guard<std::mutex> lock(m_registryMutex);

            // Выполняем шаг физики
            m_physicsSystem.update(m_registry, static_cast<double>(FIXED_TIMESTEP));

            // Записываем трансформации в буфер (пока держим мьютекс)
            writeTransformsToBuffer();
        }
        // Мьютекс освобождён — главный поток может работать с registry
    } else {
        // Без double buffering: держим мьютекс на всё время шага
        std::lock_guard<std::mutex> lock(m_registryMutex);
        m_physicsSystem.update(m_registry, static_cast<double>(FIXED_TIMESTEP));
    }

    // Увеличиваем счётчик шагов
    m_stepCount.fetch_add(1, std::memory_order_relaxed);
}

void PhysicsThread::writeTransformsToBuffer()
{
    // Итерируем по всем сущностям с RigidbodyComponent и TransformComponent
    auto view = m_registry.view<RigidbodyComponent, core::TransformComponent>();

    for (auto entity : view) {
        const auto& rigidbody = view.get<RigidbodyComponent>(entity);

        // Пропускаем статические тела — они не двигаются
        if (rigidbody.isStatic()) {
            continue;
        }

        const auto& transform = view.get<core::TransformComponent>(entity);
        m_transformBuffer.writeTransform(entity, transform);
    }
}

void PhysicsThread::swapTransformBuffers()
{
    m_transformBuffer.swapBuffers();
}

void PhysicsThread::applyTransformsToRegistry()
{
    m_transformBuffer.applyToRegistry(m_registry);
}

} // namespace simulation
