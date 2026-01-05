#pragma once

#include <simulation/PhysicsWorld.h>
#include <simulation/PhysicsTransformBuffer.h>
#include <simulation/systems/PhysicsSystem.h>
#include <entt/entt.hpp>

#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <functional>

/**
 * @file PhysicsThread.h
 * @brief Поток физической симуляции для Box2D
 *
 * PhysicsThread выносит физическую симуляцию в отдельный поток для
 * повышения производительности и обеспечения стабильного fixed timestep.
 */

namespace simulation {

/**
 * @brief Поток физической симуляции
 *
 * Выполняет Box2D симуляцию в отдельном потоке с фиксированным timestep (60 Hz).
 * Обеспечивает синхронизацию между потоком физики и главным потоком через
 * механизм double buffering для TransformComponent.
 *
 * **Использование:**
 * @code
 * // Создание и запуск
 * PhysicsWorld world;
 * PhysicsSystem physicsSystem(world);
 * entt::registry registry;
 *
 * PhysicsThread physicsThread(world, physicsSystem, registry);
 * physicsThread.start();
 *
 * // В главном цикле
 * while (running) {
 *     // Главный поток работает параллельно
 *     render();
 *
 *     // Синхронизация трансформаций (в безопасной точке)
 *     physicsThread.swapBuffers();
 * }
 *
 * // Остановка
 * physicsThread.stop();
 * @endcode
 *
 * **Архитектура многопоточности:**
 * - Физический поток: выполняет PhysicsSystem::update() с фиксированным timestep
 * - Главный поток: рендеринг, UI, обработка событий
 * - Синхронизация: через std::mutex при доступе к registry
 *
 * @warning Доступ к registry должен быть защищён мьютексом.
 */
class PhysicsThread {
public:
    /**
     * @brief Фиксированный timestep для физики (60 Hz = 16.67 мс)
     */
    static constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;

    /**
     * @brief Период физического цикла в миллисекундах
     */
    static constexpr int PHYSICS_PERIOD_MS = 16;

    /**
     * @brief Конструктор
     *
     * @param world Ссылка на PhysicsWorld
     * @param physicsSystem Ссылка на PhysicsSystem
     * @param registry Ссылка на EnTT registry
     *
     * @note Не запускает поток автоматически. Вызовите start() для запуска.
     */
    PhysicsThread(PhysicsWorld& world, PhysicsSystem& physicsSystem, entt::registry& registry);

    /**
     * @brief Деструктор
     *
     * Автоматически останавливает поток, если он запущен.
     */
    ~PhysicsThread();

    // Запрет копирования и перемещения
    PhysicsThread(const PhysicsThread&) = delete;
    PhysicsThread& operator=(const PhysicsThread&) = delete;
    PhysicsThread(PhysicsThread&&) = delete;
    PhysicsThread& operator=(PhysicsThread&&) = delete;

    /**
     * @brief Запустить поток физики
     *
     * Создаёт и запускает поток, который будет выполнять физическую симуляцию
     * с фиксированным timestep (60 Hz).
     *
     * @return true если поток успешно запущен, false если уже запущен
     *
     * @code
     * PhysicsThread physicsThread(world, system, registry);
     * if (physicsThread.start()) {
     *     spdlog::info("Physics thread started");
     * }
     * @endcode
     */
    bool start();

    /**
     * @brief Остановить поток физики
     *
     * Сигнализирует потоку о необходимости завершения и ожидает его окончания.
     * Метод блокирующий - возвращает управление только после завершения потока.
     *
     * @note Безопасно вызывать многократно или для незапущенного потока.
     */
    void stop();

    /**
     * @brief Проверить, запущен ли поток
     *
     * @return true если поток запущен и выполняется
     */
    bool isRunning() const { return m_running.load(std::memory_order_acquire); }

    /**
     * @brief Приостановить физическую симуляцию
     *
     * Поток остаётся активным, но не выполняет шаги физики.
     * Полезно для паузы в игре.
     */
    void pause();

    /**
     * @brief Возобновить физическую симуляцию
     *
     * Возобновляет выполнение физических шагов после паузы.
     */
    void resume();

    /**
     * @brief Проверить, на паузе ли симуляция
     *
     * @return true если симуляция приостановлена
     */
    bool isPaused() const { return m_paused.load(std::memory_order_acquire); }

    /**
     * @brief Получить мьютекс для синхронизации доступа к registry
     *
     * Используйте этот мьютекс при доступе к registry из главного потока,
     * пока физический поток запущен.
     *
     * @return Ссылка на std::mutex
     *
     * @code
     * // В главном потоке
     * {
     *     std::lock_guard lock(physicsThread.getRegistryMutex());
     *     // Безопасный доступ к registry
     *     auto& transform = registry.get<TransformComponent>(entity);
     * }
     * @endcode
     */
    std::mutex& getRegistryMutex() { return m_registryMutex; }

    /**
     * @brief Выполнить функцию с захваченным мьютексом
     *
     * Удобный метод для безопасного выполнения операций с registry.
     *
     * @tparam Func Тип функции
     * @param func Функция для выполнения
     * @return Результат выполнения функции
     *
     * @code
     * physicsThread.withLock([&registry, entity]() {
     *     auto& transform = registry.get<TransformComponent>(entity);
     *     // Работа с transform
     * });
     * @endcode
     */
    template<typename Func>
    auto withLock(Func&& func) -> decltype(func()) {
        std::lock_guard<std::mutex> lock(m_registryMutex);
        return func();
    }

    /**
     * @brief Получить количество выполненных шагов физики
     *
     * @return Общее количество шагов с момента запуска
     */
    uint64_t getStepCount() const { return m_stepCount.load(std::memory_order_relaxed); }

    /**
     * @brief Получить среднее время выполнения шага физики (мс)
     *
     * @return Среднее время в миллисекундах
     */
    float getAverageStepTime() const { return m_averageStepTimeMs.load(std::memory_order_relaxed); }

    /**
     * @brief Установить callback для обработки исключений в потоке
     *
     * Callback вызывается в контексте физического потока при возникновении исключения.
     *
     * @param callback Функция обработки исключений
     */
    void setExceptionHandler(std::function<void(const std::exception&)> callback);

    // ==================== DOUBLE BUFFERING API ====================

    /**
     * @brief Поменять буферы трансформаций местами
     *
     * Вызывается из главного потока в начале каждого кадра.
     * После вызова буфер чтения содержит актуальные данные из потока физики.
     *
     * @note Операция быстрая — только swap указателей.
     *
     * @code
     * // В главном цикле (перед рендерингом)
     * physicsThread.swapTransformBuffers();
     * physicsThread.applyTransformsToRegistry();
     * render();
     * @endcode
     */
    void swapTransformBuffers();

    /**
     * @brief Применить буферизованные трансформации к registry
     *
     * Обновляет TransformComponent всех физических сущностей
     * из буфера чтения. Вызывается после swapTransformBuffers().
     *
     * @note Не требует блокировки мьютекса — работает с буфером чтения.
     */
    void applyTransformsToRegistry();

    /**
     * @brief Получить ссылку на буфер трансформаций
     *
     * Для продвинутого использования (например, интерполяция).
     *
     * @return Ссылка на PhysicsTransformBuffer
     */
    PhysicsTransformBuffer& getTransformBuffer() { return m_transformBuffer; }

    /**
     * @brief Включить/выключить использование double buffering
     *
     * По умолчанию включено. При отключении используется прямая
     * синхронизация через мьютекс (менее эффективно, но проще).
     *
     * @param enabled true для включения double buffering
     */
    void setDoubleBufferingEnabled(bool enabled) {
        m_useDoubleBuffering.store(enabled, std::memory_order_release);
    }

    /**
     * @brief Проверить, включено ли double buffering
     *
     * @return true если double buffering включено
     */
    bool isDoubleBufferingEnabled() const {
        return m_useDoubleBuffering.load(std::memory_order_acquire);
    }

private:
    /**
     * @brief Основной цикл физического потока
     *
     * Выполняется в отдельном потоке. Использует fixed timestep (60 Hz)
     * с компенсацией времени сна для точного тайминга.
     */
    void physicsLoop();

    /**
     * @brief Выполнить один шаг физики
     *
     * Вызывает PhysicsSystem::update() и синхронизирует трансформации.
     */
    void doPhysicsStep();

    /**
     * @brief Записать трансформации в буфер после шага физики
     *
     * Вызывается в конце doPhysicsStep() когда включен double buffering.
     */
    void writeTransformsToBuffer();

    PhysicsWorld& m_world;                      ///< Ссылка на физический мир
    PhysicsSystem& m_physicsSystem;             ///< Ссылка на систему физики
    entt::registry& m_registry;                 ///< Ссылка на ECS registry

    std::thread m_thread;                       ///< Поток физики
    std::atomic<bool> m_running{false};         ///< Флаг работы потока
    std::atomic<bool> m_paused{false};          ///< Флаг паузы

    mutable std::mutex m_registryMutex;         ///< Мьютекс для синхронизации registry
    std::condition_variable m_pauseCondition;   ///< Условная переменная для паузы
    std::mutex m_pauseMutex;                    ///< Мьютекс для паузы

    std::atomic<uint64_t> m_stepCount{0};       ///< Счётчик шагов
    std::atomic<float> m_averageStepTimeMs{0.0f}; ///< Среднее время шага (мс)

    std::function<void(const std::exception&)> m_exceptionHandler; ///< Обработчик исключений
    std::mutex m_exceptionHandlerMutex;         ///< Мьютекс для обработчика

    // Double buffering
    PhysicsTransformBuffer m_transformBuffer;   ///< Буфер трансформаций
    std::atomic<bool> m_useDoubleBuffering{true}; ///< Флаг использования double buffering
};

} // namespace simulation
