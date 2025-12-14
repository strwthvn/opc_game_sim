#pragma once

#include <boost/signals2.hpp>
#include <entt/entt.hpp>
#include <memory>
#include <string>
#include <typeindex>
#include <unordered_map>
#include <SFML/System/Vector2.hpp>

/**
 * @file EventBus.h
 * @brief Система событий для развязки модулей
 *
 * EventBus использует boost::signals2 для потокобезопасной доставки событий.
 * Позволяет модулям взаимодействовать без прямой зависимости друг от друга.
 *
 * Пример использования:
 * @code
 * // Подписка на событие
 * EventBus::getInstance().subscribe<CollisionEvent>([](const CollisionEvent& evt) {
 *     LOG_INFO("Collision: {} <-> {}", evt.entityA, evt.entityB);
 * });
 *
 * // Публикация события
 * CollisionEvent evt{entityA, entityB, contactPoint};
 * EventBus::getInstance().publish(evt);
 * @endcode
 */

namespace core {

// ============== БАЗОВЫЙ КЛАСС СОБЫТИЯ ==============

/**
 * @brief Базовый класс для всех событий
 *
 * Все события должны наследоваться от Event.
 * Не является полиморфным (без виртуальных функций) для эффективности.
 */
struct Event {
    double timestamp = 0.0;  ///< Время возникновения события (секунды с начала игры)

    Event() = default;
    virtual ~Event() = default;

    /**
     * @brief Получить имя типа события для отладки
     * @return Имя типа события
     */
    virtual const char* getTypeName() const { return "Event"; }
};

// ============== ОПРЕДЕЛЕНИЕ БАЗОВЫХ СОБЫТИЙ ==============

/**
 * @brief Событие создания сущности
 *
 * Публикуется когда создается новая сущность в реестре.
 */
struct EntityCreatedEvent : public Event {
    entt::entity entity;  ///< Созданная сущность

    explicit EntityCreatedEvent(entt::entity e) : entity(e) {}

    const char* getTypeName() const override { return "EntityCreatedEvent"; }
};

/**
 * @brief Событие уничтожения сущности
 *
 * Публикуется перед уничтожением сущности из реестра.
 */
struct EntityDestroyedEvent : public Event {
    entt::entity entity;  ///< Уничтожаемая сущность

    explicit EntityDestroyedEvent(entt::entity e) : entity(e) {}

    const char* getTypeName() const override { return "EntityDestroyedEvent"; }
};

/**
 * @brief Событие коллизии между сущностями
 *
 * Публикуется системой CollisionSystem при обнаружении столкновения.
 */
struct CollisionEvent : public Event {
    /**
     * @brief Тип коллизии
     */
    enum class Type {
        Enter,  ///< Начало коллизии
        Stay,   ///< Продолжение коллизии
        Exit    ///< Окончание коллизии
    };

    entt::entity entityA;       ///< Первая сущность
    entt::entity entityB;       ///< Вторая сущность
    Type type;                  ///< Тип события коллизии
    sf::Vector2f contactPoint;  ///< Точка контакта (центр пересечения AABB)

    CollisionEvent(entt::entity a, entt::entity b, Type t, sf::Vector2f contact = sf::Vector2f(0.0f, 0.0f))
        : entityA(a), entityB(b), type(t), contactPoint(contact) {}

    const char* getTypeName() const override { return "CollisionEvent"; }
};

/**
 * @brief Событие изменения состояния FSM
 *
 * Публикуется системой FSMSystem при переходе между состояниями.
 */
struct StateChangedEvent : public Event {
    entt::entity entity;           ///< Сущность, изменившая состояние
    std::string previousState;     ///< Предыдущее состояние
    std::string newState;          ///< Новое состояние

    StateChangedEvent(entt::entity e, const std::string& prev, const std::string& next)
        : entity(e), previousState(prev), newState(next) {}

    const char* getTypeName() const override { return "StateChangedEvent"; }
};

/**
 * @brief Событие ввода (клавиатура, мышь)
 *
 * Публикуется InputManager при обработке пользовательского ввода.
 */
struct InputEvent : public Event {
    /**
     * @brief Тип события ввода
     */
    enum class Type {
        KeyPressed,     ///< Клавиша нажата
        KeyReleased,    ///< Клавиша отпущена
        MousePressed,   ///< Кнопка мыши нажата
        MouseReleased,  ///< Кнопка мыши отпущена
        MouseMoved      ///< Мышь перемещена
    };

    Type type;                  ///< Тип события ввода
    int keyCode = -1;           ///< Код клавиши (для Key* событий)
    int mouseButton = -1;       ///< Кнопка мыши (для Mouse* событий)
    sf::Vector2f mousePosition; ///< Позиция мыши в экранных координатах

    explicit InputEvent(Type t) : type(t) {}

    const char* getTypeName() const override { return "InputEvent"; }
};

// ============== EVENTBUS ==============

/**
 * @brief Шина событий для развязки модулей
 *
 * Singleton класс, предоставляющий глобальный доступ к системе событий.
 * Использует boost::signals2 для потокобезопасной подписки и публикации.
 *
 * Особенности:
 * - Потокобезопасность (boost::signals2 автоматически)
 * - Типобезопасность (шаблоны C++)
 * - Автоматическое управление временем жизни подписок (connection RAII)
 * - Поддержка приоритетов (group для boost::signals2)
 */
class EventBus {
public:
    /**
     * @brief Получить единственный экземпляр EventBus
     * @return Ссылка на singleton
     */
    static EventBus& getInstance();

    /**
     * @brief Тип коллбека для события
     */
    template<typename T>
    using EventCallback = std::function<void(const T&)>;

    /**
     * @brief Тип соединения (для управления подпиской)
     */
    using Connection = boost::signals2::connection;

    /**
     * @brief Подписаться на событие
     *
     * @tparam T Тип события (должен наследоваться от Event)
     * @param callback Функция, вызываемая при возникновении события
     * @return Connection объект для управления подпиской (автоматически отписывается при уничтожении)
     */
    template<typename T>
    Connection subscribe(EventCallback<T> callback) {
        static_assert(std::is_base_of<Event, T>::value, "T must inherit from Event");

        std::type_index typeIdx(typeid(T));

        // Создаем сигнал, если его еще нет
        if (m_signals.find(typeIdx) == m_signals.end()) {
            m_signals[typeIdx] = std::make_unique<Signal<T>>();
        }

        // Приводим к правильному типу сигнала и подключаем коллбек
        auto* signal = static_cast<Signal<T>*>(m_signals[typeIdx].get());
        return signal->connect(std::move(callback));
    }

    /**
     * @brief Опубликовать событие
     *
     * Все подписчики на данный тип события будут уведомлены.
     *
     * @tparam T Тип события
     * @param event Экземпляр события
     */
    template<typename T>
    void publish(const T& event) {
        static_assert(std::is_base_of<Event, T>::value, "T must inherit from Event");

        std::type_index typeIdx(typeid(T));

        // Проверяем, есть ли подписчики на это событие
        auto it = m_signals.find(typeIdx);
        if (it != m_signals.end()) {
            auto* signal = static_cast<Signal<T>*>(it->second.get());
            (*signal)(event);  // Вызываем все коллбеки
        }
    }

    /**
     * @brief Очистить все подписки
     *
     * Удаляет все зарегистрированные коллбеки.
     * Используется при завершении работы приложения.
     */
    void clear();

    /**
     * @brief Получить количество типов событий с подписчиками
     * @return Количество зарегистрированных типов событий
     */
    size_t getEventTypeCount() const {
        return m_signals.size();
    }

private:
    /**
     * @brief Приватный конструктор для Singleton
     */
    EventBus() = default;

    /**
     * @brief Запрет копирования
     */
    EventBus(const EventBus&) = delete;
    EventBus& operator=(const EventBus&) = delete;

    /**
     * @brief Базовый интерфейс для хранения разных типов сигналов
     */
    struct ISignalBase {
        virtual ~ISignalBase() = default;
    };

    /**
     * @brief Типизированный сигнал для конкретного типа события
     */
    template<typename T>
    struct Signal : public ISignalBase, public boost::signals2::signal<void(const T&)> {
        using boost::signals2::signal<void(const T&)>::signal;
    };

    /**
     * @brief Карта типов событий к их сигналам
     */
    std::unordered_map<std::type_index, std::unique_ptr<ISignalBase>> m_signals;
};

} // namespace core
