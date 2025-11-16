#include "core/Window.h"
#include "core/Logger.h"
#include <stdexcept>

namespace core {

Window::Window() : Window(Config{}) {
}

Window::Window(const Config& config) : m_config(config) {
    // Создаем окно с настройками
    m_window.create(
        sf::VideoMode{{m_config.width, m_config.height}},
        m_config.title
    );

    // Проверяем что окно успешно создалось
    if (!m_window.isOpen()) {
        LOG_CRITICAL("Failed to create SFML window ({}x{})", m_config.width, m_config.height);
        throw std::runtime_error("Failed to create SFML window");
    }

    LOG_DEBUG("SFML window created successfully ({}x{})", m_config.width, m_config.height);

    // Настройка окна
    if (m_config.vsync) {
        m_window.setVerticalSyncEnabled(true);
    } else {
        m_window.setFramerateLimit(m_config.framerateLimit);
    }
}

bool Window::isOpen() const {
    return m_window.isOpen();
}

std::optional<sf::Event> Window::pollEvent() {
    return m_window.pollEvent();
}

void Window::clear(const sf::Color& color) {
    m_window.clear(color);
}

void Window::display() {
    m_window.display();
}

void Window::close() {
    m_window.close();
}

sf::RenderWindow* Window::getRenderWindow() {
    return &m_window;
}

const sf::RenderWindow* Window::getRenderWindow() const {
    return &m_window;
}

sf::Vector2u Window::getSize() const {
    return m_window.getSize();
}

} // namespace core
