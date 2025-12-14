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
    // Загружаем конфигурацию из YAML файла
    auto& globalConfig = core::Config::getInstance();
    globalConfig.load("config.yaml");

    // Обновляем конфигурацию приложения из глобальной конфигурации
    m_config.enableLogging = globalConfig.get("logging.enabled", true);
    m_config.logToFile = globalConfig.get("logging.logToFile", true);
    m_config.metricsLogInterval = globalConfig.get("game.metricsLogInterval", 5.0);
    m_config.targetFPS = globalConfig.get("window.frameRateLimit", 60.0);
    m_config.fixedTimestep = globalConfig.get("game.fixedTimestep", 1.0 / 60.0);

    // Обновляем window config
    m_config.windowConfig.width = globalConfig.get("window.width", 1280);
    m_config.windowConfig.height = globalConfig.get("window.height", 720);
    m_config.windowConfig.title = globalConfig.get<std::string>("window.title", "OPC Game Simulator");
    m_config.windowConfig.vsync = globalConfig.get("window.vsync", true);
    m_config.windowConfig.framerateLimit = globalConfig.get("window.frameRateLimit", 60);

    // Инициализация системы логирования
    if (m_config.enableLogging) {
        std::string logPath = globalConfig.get<std::string>("logging.logFilePath", "logs/opc_game_sim.log");
        Logger::initialize(m_config.logToFile, logPath);
        LOG_INFO("=== OPC Game Simulator Starting ===");
        LOG_INFO("Configuration loaded from: {}", globalConfig.getConfigPath());
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

    // Создаем менеджер аудио
    m_audioManager = std::make_unique<AudioManager>(m_resourceManager.get());
    LOG_INFO("AudioManager initialized");

    // Создаем менеджер состояний и устанавливаем начальное состояние
    m_stateManager = std::make_unique<StateManager>();
    m_stateManager->setInputManager(m_inputManager.get());
    m_stateManager->setResourceManager(m_resourceManager.get());
    m_stateManager->setAudioManager(m_audioManager.get());
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
        double maxFrameTime = core::Config::getInstance().get("game.maxFrameTime", 0.25);
        if (frameTime > maxFrameTime) {
            LOG_WARN("Frame time spike detected: {:.3f}s (capped at {:.2f}s)",
                     frameTime, maxFrameTime);
            frameTime = maxFrameTime;
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

        // Обработка событий фокуса окна
        if (event->is<sf::Event::FocusLost>()) {
            LOG_WARN("Window focus LOST");
        }
        if (event->is<sf::Event::FocusGained>()) {
            LOG_INFO("Window focus GAINED");
        }

        // Передаем событие в InputManager для отслеживания состояний
        m_inputManager->handleEvent(*event);

        // Передаем событие текущему состоянию
        m_stateManager->handleEvent(*event);
    }
}

void Application::update(double dt) {
    // Обновляем аудио систему
    m_audioManager->update();

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
