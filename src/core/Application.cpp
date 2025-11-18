#include "core/Application.h"
#include "core/states/MenuState.h"
#include <SFML/Window/Event.hpp>
#include <chrono>

namespace core {

Application::Application() : Application(Config{}) {
}

Application::Application(const Config& config)
    : m_config(config)
    , m_running(false)
    , m_metricsTimer(0.0) {
    // Инициализация системы логирования
    if (m_config.enableLogging) {
        Logger::initialize(m_config.logToFile, "logs/opc_game_sim.log");
        LOG_INFO("=== OPC Game Simulator Starting ===");
        LOG_INFO("Application configuration:");
        LOG_INFO("  Target FPS: {}", m_config.targetFPS);
        LOG_INFO("  Fixed timestep: {:.4f}s ({} UPS)",
                 m_config.fixedTimestep,
                 static_cast<int>(1.0 / m_config.fixedTimestep));
        LOG_INFO("  Metrics log interval: {}s", m_config.metricsLogInterval);
    }

    // Создаем окно
    LOG_INFO("Creating window: {}x{}",
             m_config.windowConfig.width,
             m_config.windowConfig.height);
    m_window = std::make_unique<Window>(m_config.windowConfig);

    // Создаем систему метрик производительности
    m_metrics = std::make_unique<PerformanceMetrics>(ApplicationConstants::METRICS_BUFFER_SIZE);
    LOG_INFO("Performance metrics system initialized");

    // Создаем менеджер ввода
    m_inputManager = std::make_unique<InputManager>();
    LOG_INFO("InputManager initialized");

    // Создаем менеджер ресурсов
    m_resourceManager = std::make_unique<ResourceManager>();
    LOG_INFO("ResourceManager initialized");

    // Создаем менеджер состояний и устанавливаем начальное состояние
    m_stateManager = std::make_unique<StateManager>();
    m_stateManager->setInputManager(m_inputManager.get());
    m_stateManager->setResourceManager(m_resourceManager.get());
    m_stateManager->setWindow(m_window.get());
    m_stateManager->pushState(std::make_unique<MenuState>(m_stateManager.get()));
    m_stateManager->applyPendingChanges(); // Применяем начальное состояние сразу
    LOG_INFO("StateManager initialized with MenuState");
}

int Application::run() {
    LOG_INFO("Starting main game loop");
    m_running = true;

    // Игровой цикл с фиксированным timestep
    // Реализация паттерна "Fix Your Timestep"
    // Источник: https://gafferongames.com/post/fix_your_timestep/

    using Clock = std::chrono::high_resolution_clock;
    using Duration = std::chrono::duration<double>;

    auto currentTime = Clock::now();
    double accumulator = 0.0;
    m_metricsTimer = 0.0;

    LOG_INFO("Game loop started successfully");

    while (m_running && m_window->isOpen() && !m_stateManager->isEmpty()) {
        // Вычисляем время кадра
        auto newTime = Clock::now();
        double frameTime = Duration(newTime - currentTime).count();
        currentTime = newTime;

        // Ограничиваем максимальное время кадра (предотвращаем "spiral of death")
        if (frameTime > ApplicationConstants::MAX_FRAME_TIME) {
            LOG_WARN("Frame time spike detected: {:.3f}s (capped at {:.2f}s)",
                     frameTime, ApplicationConstants::MAX_FRAME_TIME);
            frameTime = ApplicationConstants::MAX_FRAME_TIME;
        }

        accumulator += frameTime;
        m_metricsTimer += frameTime;

        // Обработка событий
        processEvents();

        // Обновление с фиксированным timestep
        while (accumulator >= m_config.fixedTimestep) {
            update(m_config.fixedTimestep);
            m_metrics->recordUpdate();
            accumulator -= m_config.fixedTimestep;
        }

        // Рендеринг (может происходить с переменной частотой)
        render();
        m_metrics->recordFrame();

        // Периодическое логирование метрик производительности
        if (m_metricsTimer >= m_config.metricsLogInterval) {
            LOG_INFO("Performance metrics: FPS={:.1f} (min={:.1f}, max={:.1f}), UPS={:.1f}, FrameTime={:.2f}ms",
                     m_metrics->getFPS(),
                     m_metrics->getMinFPS(),
                     m_metrics->getMaxFPS(),
                     m_metrics->getUPS(),
                     m_metrics->getAverageFrameTime());
            m_metricsTimer = 0.0;
        }
    }

    LOG_INFO("Game loop ended");
    LOG_INFO("Final performance metrics: FPS={:.1f}, UPS={:.1f}",
             m_metrics->getFPS(),
             m_metrics->getUPS());
    LOG_INFO("=== OPC Game Simulator Shutting Down ===");

    // Завершаем работу логгера
    Logger::shutdown();

    return 0;
}

void Application::stop() {
    LOG_INFO("Application stop requested");
    m_running = false;
}

void Application::processEvents() {
    // Обновляем InputManager в начале кадра (сбрасываем "just pressed/released")
    m_inputManager->update();

    while (const auto event = m_window->pollEvent()) {
        // Обработка события закрытия окна
        if (event->is<sf::Event::Closed>()) {
            LOG_DEBUG("Window close event received");
            m_window->close();
        }

        // Обработка изменения размера окна
        if (const auto* resized = event->getIf<sf::Event::Resized>()) {
            LOG_DEBUG("Window resized to {}x{}", resized->size.x, resized->size.y);
            // Уведомляем StateManager об изменении размера
            m_stateManager->handleWindowResize(resized->size);
        }

        // Передаем событие в InputManager для отслеживания состояний
        m_inputManager->handleEvent(*event);

        // Передаем событие текущему состоянию
        m_stateManager->handleEvent(*event);
    }
}

void Application::update(double dt) {
    // Обновляем текущее состояние
    m_stateManager->update(dt);
}

void Application::render() {
    static int renderCount = 0;
    if (renderCount == 0) {
        LOG_INFO("Application::render() called for the first time");
    }
    renderCount++;

    // Очистка экрана темно-серым цветом (промышленная тема)
    m_window->clear(ApplicationConstants::BACKGROUND_COLOR);

    // Рендерим текущее состояние
    m_stateManager->render(*m_window->getRenderWindow());

    // Отображение кадра
    m_window->display();

    if (renderCount % 300 == 0) {
        LOG_DEBUG("Application::render() called {} times", renderCount);
    }
}

} // namespace core
