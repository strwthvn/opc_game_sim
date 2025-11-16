#pragma once

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/System/Vector2.hpp>
#include <unordered_map>
#include <cstddef>

namespace sf {
    class Event;
}

namespace core {

/**
 * @brief Менеджер обработки пользовательского ввода
 *
 * Предоставляет удобный интерфейс для работы с клавиатурой и мышью.
 * Отслеживает не только текущее состояние клавиш, но и моменты
 * нажатия/отпускания (just pressed/released).
 *
 * Использование:
 * @code
 * // В начале кадра
 * inputManager.update();
 *
 * // Обработка событий
 * inputManager.handleEvent(event);
 *
 * // Проверка состояний
 * if (inputManager.isKeyPressed(sf::Keyboard::Key::W)) {
 *     // Клавиша W зажата
 * }
 * if (inputManager.isKeyJustPressed(sf::Keyboard::Key::Space)) {
 *     // Space была только что нажата (один раз)
 * }
 * @endcode
 */
class InputManager {
public:
    /**
     * @brief Конструктор
     */
    InputManager();

    /**
     * @brief Деструктор
     */
    ~InputManager() = default;

    // Запрет копирования и перемещения
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;
    InputManager(InputManager&&) = delete;
    InputManager& operator=(InputManager&&) = delete;

    /**
     * @brief Обновляет состояние менеджера ввода
     *
     * Должно вызываться в начале каждого кадра перед обработкой событий.
     * Сбрасывает флаги "just pressed/released" и обновляет предыдущие состояния.
     */
    void update();

    /**
     * @brief Обрабатывает событие SFML
     * @param event Событие для обработки
     */
    void handleEvent(const sf::Event& event);

    // ========== Методы для клавиатуры ==========

    /**
     * @brief Проверяет, нажата ли клавиша в данный момент
     * @param key Код клавиши
     * @return true если клавиша нажата
     */
    bool isKeyPressed(sf::Keyboard::Key key) const;

    /**
     * @brief Проверяет, была ли клавиша только что нажата
     * @param key Код клавиши
     * @return true если клавиша была нажата в этом кадре (однократное срабатывание)
     */
    bool isKeyJustPressed(sf::Keyboard::Key key) const;

    /**
     * @brief Проверяет, была ли клавиша только что отпущена
     * @param key Код клавиши
     * @return true если клавиша была отпущена в этом кадре
     */
    bool isKeyJustReleased(sf::Keyboard::Key key) const;

    // ========== Методы для мыши ==========

    /**
     * @brief Проверяет, нажата ли кнопка мыши в данный момент
     * @param button Кнопка мыши
     * @return true если кнопка нажата
     */
    bool isMouseButtonPressed(sf::Mouse::Button button) const;

    /**
     * @brief Проверяет, была ли кнопка мыши только что нажата
     * @param button Кнопка мыши
     * @return true если кнопка была нажата в этом кадре
     */
    bool isMouseButtonJustPressed(sf::Mouse::Button button) const;

    /**
     * @brief Проверяет, была ли кнопка мыши только что отпущена
     * @param button Кнопка мыши
     * @return true если кнопка была отпущена в этом кадре
     */
    bool isMouseButtonJustReleased(sf::Mouse::Button button) const;

    /**
     * @brief Возвращает текущую позицию мыши в окне
     * @return Позиция мыши в пикселях (целочисленная)
     */
    sf::Vector2i getMousePosition() const;

    /**
     * @brief Возвращает текущую позицию мыши в окне
     * @return Позиция мыши (вещественная, для удобства расчетов)
     */
    sf::Vector2f getMousePositionF() const;

    /**
     * @brief Возвращает смещение колеса мыши за текущий кадр
     * @return Смещение колеса (положительное - вверх, отрицательное - вниз)
     */
    float getMouseWheelDelta() const;

    /**
     * @brief Сбрасывает все состояния ввода
     *
     * Полезно при смене состояния приложения (например, переход из игры в меню)
     * чтобы избежать "залипших" клавиш
     */
    void reset();

private:
    // Состояния клавиатуры
    std::unordered_map<sf::Keyboard::Key, bool> m_keyStates;
    std::unordered_map<sf::Keyboard::Key, bool> m_previousKeyStates;

    // Состояния мыши
    std::unordered_map<sf::Mouse::Button, bool> m_mouseButtonStates;
    std::unordered_map<sf::Mouse::Button, bool> m_previousMouseButtonStates;

    // Позиция мыши
    sf::Vector2i m_mousePosition;

    // Смещение колеса мыши
    float m_mouseWheelDelta;

    /**
     * @brief Проверяет валидность кода клавиши
     * @param key Код клавиши
     * @return true если код валиден
     */
    static bool isKeyValid(sf::Keyboard::Key key);

    /**
     * @brief Проверяет валидность кода кнопки мыши
     * @param button Код кнопки
     * @return true если код валиден
     */
    static bool isMouseButtonValid(sf::Mouse::Button button);
};

} // namespace core
