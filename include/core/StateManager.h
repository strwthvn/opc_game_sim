#pragma once

#include "State.h"
#include <memory>
#include <vector>
#include <functional>

namespace sf {
    class RenderWindow;
    class Event;
}

namespace core {

class InputManager;
class ResourceManager;
class Window;

/**
 * @brief Менеджер состояний приложения
 *
 * Управляет стеком состояний приложения (меню, игра, пауза и т.д.)
 * Использует паттерн State Machine с поддержкой стека состояний
 *
 * Особенности:
 * - Стековая организация состояний (можно накладывать одно на другое)
 * - Отложенные операции со стеком (применяются после обновления)
 * - Поддержка обновления/рендеринга нижних состояний
 */
class StateManager {
public:
    /**
     * @brief Конструктор
     */
    StateManager();

    /**
     * @brief Деструктор
     */
    ~StateManager() = default;

    // Запрет копирования и перемещения
    StateManager(const StateManager&) = delete;
    StateManager& operator=(const StateManager&) = delete;
    StateManager(StateManager&&) = delete;
    StateManager& operator=(StateManager&&) = delete;

    /**
     * @brief Добавляет новое состояние на вершину стека
     * @param state Уникальный указатель на состояние
     */
    void pushState(std::unique_ptr<State> state);

    /**
     * @brief Удаляет верхнее состояние из стека
     */
    void popState();

    /**
     * @brief Заменяет верхнее состояние на новое
     * @param state Уникальный указатель на новое состояние
     */
    void changeState(std::unique_ptr<State> state);

    /**
     * @brief Очищает весь стек состояний
     */
    void clearStates();

    /**
     * @brief Обрабатывает события
     * @param event Событие SFML
     */
    void handleEvent(const sf::Event& event);

    /**
     * @brief Обновляет активные состояния
     * @param dt Время с последнего обновления в секундах
     */
    void update(double dt);

    /**
     * @brief Отрисовывает активные состояния
     * @param window Окно для отрисовки
     */
    void render(sf::RenderWindow& window);

    /**
     * @brief Возвращает текущее (верхнее) состояние
     * @return Указатель на текущее состояние или nullptr если стек пуст
     */
    State* getCurrentState() const;

    /**
     * @brief Проверяет, пуст ли стек состояний
     * @return true если стек пуст
     */
    bool isEmpty() const;

    /**
     * @brief Возвращает количество состояний в стеке
     * @return Размер стека
     */
    size_t getStateCount() const;

    /**
     * @brief Применяет отложенные операции со стеком
     *
     * Обычно вызывается автоматически после update(), но может быть вызван
     * вручную для немедленного применения изменений
     */
    void applyPendingChanges();

    /**
     * @brief Устанавливает менеджер ввода
     * @param inputManager Указатель на менеджер ввода
     */
    void setInputManager(InputManager* inputManager);

    /**
     * @brief Возвращает менеджер ввода
     * @return Указатель на менеджер ввода
     */
    InputManager* getInputManager() const;

    /**
     * @brief Устанавливает указатель на окно приложения
     * @param window Указатель на окно
     */
    void setWindow(Window* window);

    /**
     * @brief Возвращает размер окна приложения
     * @return Размер окна в пикселях
     */
    sf::Vector2u getWindowSize() const;

    /**
     * @brief Устанавливает менеджер ресурсов
     * @param resourceManager Указатель на менеджер ресурсов
     */
    void setResourceManager(ResourceManager* resourceManager);

    /**
     * @brief Возвращает менеджер ресурсов
     * @return Указатель на менеджер ресурсов
     */
    ResourceManager* getResourceManager() const;

private:

    /**
     * @brief Перечисление типов операций со стеком
     */
    enum class Action {
        Push,    ///< Добавить состояние
        Pop,     ///< Удалить состояние
        Change,  ///< Заменить состояние
        Clear    ///< Очистить стек
    };

    /**
     * @brief Структура для отложенной операции
     */
    struct PendingChange {
        Action action;
        std::unique_ptr<State> state;
    };

    std::vector<std::unique_ptr<State>> m_states;      ///< Стек состояний
    std::vector<PendingChange> m_pendingChanges;       ///< Отложенные операции
    bool m_isProcessingChanges;                        ///< Флаг обработки изменений
    InputManager* m_inputManager;                      ///< Указатель на менеджер ввода
    ResourceManager* m_resourceManager;                ///< Указатель на менеджер ресурсов
    Window* m_window;                                  ///< Указатель на окно приложения
};

} // namespace core
