#pragma once

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Window/Event.hpp>
#include <optional>
#include <string>

namespace core {

/**
 * @brief Обертка над SFML RenderWindow для управления окном приложения
 *
 * Отвечает за создание, настройку и обработку событий окна.
 * Предоставляет простой API для работы с окном SFML.
 */
class Window {
public:
    /**
     * @brief Конфигурация окна
     */
    struct Config {
        std::string title = "OPC Game Simulator";
        unsigned int width = 1280;
        unsigned int height = 720;
        unsigned int framerateLimit = 60;
        bool vsync = false;
        bool fullscreen = false;
    };

    /**
     * @brief Конструктор с конфигурацией по умолчанию
     */
    Window();

    /**
     * @brief Конструктор с пользовательской конфигурацией
     * @param config Конфигурация окна
     */
    explicit Window(const Config& config);

    /**
     * @brief Деструктор
     */
    ~Window() = default;

    // Запрет копирования
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    // Разрешаем перемещение
    Window(Window&&) noexcept = default;
    Window& operator=(Window&&) noexcept = default;

    /**
     * @brief Проверяет, открыто ли окно
     * @return true если окно открыто
     */
    bool isOpen() const;

    /**
     * @brief Обрабатывает события окна
     * @return std::optional с событием, если событие было получено
     */
    std::optional<sf::Event> pollEvent();

    /**
     * @brief Очищает окно указанным цветом
     * @param color Цвет фона (по умолчанию черный)
     */
    void clear(const sf::Color& color = sf::Color::Black);

    /**
     * @brief Отображает содержимое окна
     */
    void display();

    /**
     * @brief Закрывает окно
     */
    void close();

    /**
     * @brief Возвращает указатель на SFML окно
     * @return Указатель на sf::RenderWindow
     */
    sf::RenderWindow* getRenderWindow();

    /**
     * @brief Возвращает константный указатель на SFML окно
     * @return Константный указатель на sf::RenderWindow
     */
    const sf::RenderWindow* getRenderWindow() const;

    /**
     * @brief Возвращает размер окна
     * @return Размер окна в пикселях
     */
    sf::Vector2u getSize() const;

private:
    sf::RenderWindow m_window;      ///< SFML окно
    Config m_config;                 ///< Конфигурация окна
};

} // namespace core
