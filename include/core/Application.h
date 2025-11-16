#pragma once

#include "Window.h"
#include "Logger.h"
#include "PerformanceMetrics.h"
#include "InputManager.h"
#include "ResourceManager.h"
#include "StateManager.h"
#include <memory>

namespace core {

/**
 * @brief Константы приложения
 */
namespace ApplicationConstants {
    /// Максимальное время кадра (предотвращение "spiral of death")
    constexpr double MAX_FRAME_TIME = 0.25;

    /// Размер буфера метрик производительности
    constexpr size_t METRICS_BUFFER_SIZE = 60;

    /// Цвет фона окна (темно-серый для промышленной темы)
    constexpr sf::Color BACKGROUND_COLOR = sf::Color(45, 45, 48);
}

/**
 * @brief Главный класс приложения с игровым циклом
 *
 * Управляет жизненным циклом приложения, включая:
 * - Создание и управление окном
 * - Игровой цикл с фиксированным timestep
 * - Обработку событий
 * - Рендеринг
 *
 * Использует паттерн "Fix Your Timestep" для стабильной физики
 * и обновлений независимо от FPS.
 */
class Application {
public:
    /**
     * @brief Конфигурация приложения
     */
    struct Config {
        Window::Config windowConfig;
        double targetFPS = 60.0;
        double fixedTimestep = 1.0 / 60.0;  // 60 UPS (Updates Per Second)
        bool enableLogging = true;
        bool logToFile = true;
        double metricsLogInterval = 5.0;  // Интервал логирования метрик в секундах
    };

    /**
     * @brief Конструктор с конфигурацией по умолчанию
     */
    Application();

    /**
     * @brief Конструктор с пользовательской конфигурацией
     * @param config Конфигурация приложения
     */
    explicit Application(const Config& config);

    /**
     * @brief Деструктор
     */
    ~Application() = default;

    // Запрет копирования и перемещения
    Application(const Application&) = delete;
    Application& operator=(const Application&) = delete;
    Application(Application&&) = delete;
    Application& operator=(Application&&) = delete;

    /**
     * @brief Запускает приложение
     * @return Код возврата (0 при успехе)
     */
    int run();

    /**
     * @brief Останавливает приложение
     */
    void stop();

private:
    /**
     * @brief Обрабатывает события окна и ввода
     */
    void processEvents();

    /**
     * @brief Обновляет логику приложения
     * @param dt Время с последнего обновления в секундах
     */
    void update(double dt);

    /**
     * @brief Отрисовывает кадр
     */
    void render();

    std::unique_ptr<Window> m_window;            ///< Окно приложения
    std::unique_ptr<PerformanceMetrics> m_metrics;  ///< Метрики производительности
    std::unique_ptr<InputManager> m_inputManager;   ///< Менеджер ввода
    std::unique_ptr<ResourceManager> m_resourceManager; ///< Менеджер ресурсов
    std::unique_ptr<StateManager> m_stateManager;   ///< Менеджер состояний
    Config m_config;                             ///< Конфигурация
    bool m_running;                              ///< Флаг работы приложения
    double m_metricsTimer;                       ///< Таймер для периодического логирования метрик
};

} // namespace core
