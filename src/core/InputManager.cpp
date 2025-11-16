#include "core/InputManager.h"
#include "core/Logger.h"
#include <SFML/Window/Event.hpp>
#include <algorithm>

namespace core {

InputManager::InputManager()
    : m_mousePosition(0, 0)
    , m_mouseWheelDelta(0.0f) {
    // Maps are initialized empty, states will be added on first use
    LOG_DEBUG("InputManager initialized");
}

void InputManager::update() {
    // Сохраняем текущее состояние как предыдущее
    m_previousKeyStates = m_keyStates;
    m_previousMouseButtonStates = m_mouseButtonStates;

    // Сбрасываем смещение колеса мыши (оно применяется только в кадре события)
    m_mouseWheelDelta = 0.0f;
}

void InputManager::handleEvent(const sf::Event& event) {
    // Обработка событий клавиатуры
    if (const auto* keyPressed = event.getIf<sf::Event::KeyPressed>()) {
        if (isKeyValid(keyPressed->code)) {
            m_keyStates[keyPressed->code] = true;
        }
    }
    else if (const auto* keyReleased = event.getIf<sf::Event::KeyReleased>()) {
        if (isKeyValid(keyReleased->code)) {
            m_keyStates[keyReleased->code] = false;
        }
    }

    // Обработка событий мыши
    else if (const auto* mousePressed = event.getIf<sf::Event::MouseButtonPressed>()) {
        if (isMouseButtonValid(mousePressed->button)) {
            m_mouseButtonStates[mousePressed->button] = true;
        }
    }
    else if (const auto* mouseReleased = event.getIf<sf::Event::MouseButtonReleased>()) {
        if (isMouseButtonValid(mouseReleased->button)) {
            m_mouseButtonStates[mouseReleased->button] = false;
        }
    }

    // Обработка движения мыши
    else if (const auto* mouseMoved = event.getIf<sf::Event::MouseMoved>()) {
        m_mousePosition = mouseMoved->position;
    }

    // Обработка колеса мыши
    else if (const auto* mouseWheel = event.getIf<sf::Event::MouseWheelScrolled>()) {
        // SFML 3 использует MouseWheelScrolled вместо MouseWheelMoved
        if (mouseWheel->wheel == sf::Mouse::Wheel::Vertical) {
            m_mouseWheelDelta = mouseWheel->delta;
        }
    }
}

bool InputManager::isKeyPressed(sf::Keyboard::Key key) const {
    if (!isKeyValid(key)) {
        return false;
    }
    auto it = m_keyStates.find(key);
    return it != m_keyStates.end() && it->second;
}

bool InputManager::isKeyJustPressed(sf::Keyboard::Key key) const {
    if (!isKeyValid(key)) {
        return false;
    }
    auto current = m_keyStates.find(key);
    auto previous = m_previousKeyStates.find(key);

    bool isCurrentlyPressed = (current != m_keyStates.end() && current->second);
    bool wasPreviouslyPressed = (previous != m_previousKeyStates.end() && previous->second);

    // Нажата сейчас, но не была нажата в предыдущем кадре
    return isCurrentlyPressed && !wasPreviouslyPressed;
}

bool InputManager::isKeyJustReleased(sf::Keyboard::Key key) const {
    if (!isKeyValid(key)) {
        return false;
    }
    auto current = m_keyStates.find(key);
    auto previous = m_previousKeyStates.find(key);

    bool isCurrentlyPressed = (current != m_keyStates.end() && current->second);
    bool wasPreviouslyPressed = (previous != m_previousKeyStates.end() && previous->second);

    // Не нажата сейчас, но была нажата в предыдущем кадре
    return !isCurrentlyPressed && wasPreviouslyPressed;
}

bool InputManager::isMouseButtonPressed(sf::Mouse::Button button) const {
    if (!isMouseButtonValid(button)) {
        return false;
    }
    auto it = m_mouseButtonStates.find(button);
    return it != m_mouseButtonStates.end() && it->second;
}

bool InputManager::isMouseButtonJustPressed(sf::Mouse::Button button) const {
    if (!isMouseButtonValid(button)) {
        return false;
    }
    auto current = m_mouseButtonStates.find(button);
    auto previous = m_previousMouseButtonStates.find(button);

    bool isCurrentlyPressed = (current != m_mouseButtonStates.end() && current->second);
    bool wasPreviouslyPressed = (previous != m_previousMouseButtonStates.end() && previous->second);

    return isCurrentlyPressed && !wasPreviouslyPressed;
}

bool InputManager::isMouseButtonJustReleased(sf::Mouse::Button button) const {
    if (!isMouseButtonValid(button)) {
        return false;
    }
    auto current = m_mouseButtonStates.find(button);
    auto previous = m_previousMouseButtonStates.find(button);

    bool isCurrentlyPressed = (current != m_mouseButtonStates.end() && current->second);
    bool wasPreviouslyPressed = (previous != m_previousMouseButtonStates.end() && previous->second);

    return !isCurrentlyPressed && wasPreviouslyPressed;
}

sf::Vector2i InputManager::getMousePosition() const {
    return m_mousePosition;
}

sf::Vector2f InputManager::getMousePositionF() const {
    return sf::Vector2f(static_cast<float>(m_mousePosition.x),
                        static_cast<float>(m_mousePosition.y));
}

float InputManager::getMouseWheelDelta() const {
    return m_mouseWheelDelta;
}

void InputManager::reset() {
    LOG_DEBUG("InputManager reset");
    m_keyStates.clear();
    m_previousKeyStates.clear();
    m_mouseButtonStates.clear();
    m_previousMouseButtonStates.clear();
    m_mousePosition = sf::Vector2i(0, 0);
    m_mouseWheelDelta = 0.0f;
}

bool InputManager::isKeyValid(sf::Keyboard::Key key) {
    // В SFML 3 просто проверяем, что это не Unknown
    // Все остальные значения считаются валидными
    return key != sf::Keyboard::Key::Unknown;
}

bool InputManager::isMouseButtonValid(sf::Mouse::Button button) {
    // В SFML 3 все значения Button валидны
    // Можно добавить дополнительные проверки если нужно
    (void)button;  // Подавление предупреждения о неиспользуемом параметре
    return true;
}

} // namespace core
