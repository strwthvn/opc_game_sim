#pragma once

#include <SFML/Window/Event.hpp>
#include <memory>
#include <string>

namespace sf {
    class RenderWindow;
}

namespace core {

class StateManager;     // Forward declaration
class InputManager;     // Forward declaration
class ResourceManager;  // Forward declaration

/**
 * @brief Базовый интерфейс для состояний приложения
 *
 * Реализует паттерн State для управления различными режимами приложения
 * (меню, игра, пауза, настройки и т.д.)
 *
 * Состояния организованы в стек, что позволяет накладывать одно состояние
 * на другое (например, пауза поверх игры)
 */
class State {
public:
    /**
     * @brief Конструктор
     * @param stateManager Указатель на менеджер состояний
     */
    explicit State(StateManager* stateManager);

    /**
     * @brief Виртуальный деструктор
     */
    virtual ~State() = default;

    // Запрет копирования и перемещения
    State(const State&) = delete;
    State& operator=(const State&) = delete;
    State(State&&) = delete;
    State& operator=(State&&) = delete;

    /**
     * @brief Вызывается при входе в состояние
     *
     * Используется для инициализации ресурсов состояния
     */
    virtual void onEnter() = 0;

    /**
     * @brief Вызывается при выходе из состояния
     *
     * Используется для освобождения ресурсов состояния
     */
    virtual void onExit() = 0;

    /**
     * @brief Обрабатывает события
     * @param event Событие SFML
     * @return true если событие обработано и не должно передаваться дальше
     */
    virtual bool handleEvent(const sf::Event& event) = 0;

    /**
     * @brief Обновляет логику состояния
     * @param dt Время с последнего обновления в секундах
     */
    virtual void update(double dt) = 0;

    /**
     * @brief Отрисовывает состояние
     * @param window Окно для отрисовки
     */
    virtual void render(sf::RenderWindow& window) = 0;

    /**
     * @brief Возвращает имя состояния (для отладки)
     * @return Имя состояния
     */
    virtual std::string getName() const = 0;

    /**
     * @brief Определяет, должно ли состояние обновляться когда оно не на вершине стека
     * @return true если состояние должно обновляться в фоне
     */
    virtual bool updateBelow() const { return false; }

    /**
     * @brief Определяет, должно ли состояние рендериться когда оно не на вершине стека
     * @return true если состояние должно рендериться под другими
     */
    virtual bool renderBelow() const { return false; }

    /**
     * @brief Вызывается при изменении размера окна
     * @param newSize Новый размер окна
     */
    virtual void onWindowResize(const sf::Vector2u& newSize) { (void)newSize; }

protected:
    StateManager* m_stateManager;  ///< Указатель на менеджер состояний

    /**
     * @brief Возвращает InputManager для удобного доступа к вводу
     * @return Указатель на InputManager
     */
    InputManager* getInputManager() const;

    /**
     * @brief Возвращает размер окна приложения
     * @return Размер окна в пикселях
     */
    sf::Vector2u getWindowSize() const;

    /**
     * @brief Возвращает ResourceManager для доступа к ресурсам
     * @return Указатель на ResourceManager
     */
    ResourceManager* getResourceManager() const;
};

} // namespace core
