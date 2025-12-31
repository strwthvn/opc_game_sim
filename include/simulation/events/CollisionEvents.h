#pragma once

#include <entt/entt.hpp>
#include <SFML/System/Vector2.hpp>
#include <boost/signals2.hpp>
#include <box2d/box2d.h>

/**
 * @file CollisionEvents.h
 * @brief События коллизий для интеграции Box2D с ECS
 *
 * Определяет события, которые генерируются при столкновениях и триггерах
 * в физической симуляции Box2D 3.x.
 *
 * Box2D 3.x использует event-based подход вместо callback-based.
 * События собираются после каждого шага симуляции и обрабатываются
 * через boost::signals2.
 *
 * @note Используйте b2Shape_GetUserData() для получения entt::entity из Box2D.
 */

namespace simulation {

/**
 * @brief Информация о точке контакта
 */
struct ContactPoint {
    sf::Vector2f position;      ///< Позиция точки контакта в пикселях
    sf::Vector2f normal;        ///< Нормаль контакта (от A к B)
    float separation = 0.0f;    ///< Расстояние проникновения (отрицательное при пересечении)
};

/**
 * @brief Событие начала столкновения двух тел
 *
 * Генерируется когда два non-sensor shape начинают касаться друг друга.
 * Оба shape должны быть не-сенсорами для генерации этого события.
 */
struct CollisionBeginEvent {
    entt::entity entityA = entt::null;  ///< Первая сущность в коллизии
    entt::entity entityB = entt::null;  ///< Вторая сущность в коллизии
    b2ShapeId shapeIdA;                 ///< ID Box2D shape первой сущности
    b2ShapeId shapeIdB;                 ///< ID Box2D shape второй сущности
    ContactPoint contactPoint;          ///< Информация о точке контакта

    /**
     * @brief Проверить валидность события
     * @return true если обе сущности валидны
     */
    bool isValid() const {
        return entityA != entt::null && entityB != entt::null;
    }
};

/**
 * @brief Событие окончания столкновения двух тел
 *
 * Генерируется когда два non-sensor shape перестают касаться друг друга.
 */
struct CollisionEndEvent {
    entt::entity entityA = entt::null;  ///< Первая сущность в коллизии
    entt::entity entityB = entt::null;  ///< Вторая сущность в коллизии
    b2ShapeId shapeIdA;                 ///< ID Box2D shape первой сущности
    b2ShapeId shapeIdB;                 ///< ID Box2D shape второй сущности

    /**
     * @brief Проверить валидность события
     * @return true если обе сущности валидны
     */
    bool isValid() const {
        return entityA != entt::null && entityB != entt::null;
    }
};

/**
 * @brief Событие удара (высокоскоростное столкновение)
 *
 * Генерируется когда два тела сталкиваются со скоростью выше порога.
 * Полезно для воспроизведения звуков удара, нанесения урона и т.д.
 */
struct CollisionHitEvent {
    entt::entity entityA = entt::null;  ///< Первая сущность в столкновении
    entt::entity entityB = entt::null;  ///< Вторая сущность в столкновении
    b2ShapeId shapeIdA;                 ///< ID Box2D shape первой сущности
    b2ShapeId shapeIdB;                 ///< ID Box2D shape второй сущности
    sf::Vector2f point;                 ///< Точка столкновения в пикселях
    sf::Vector2f normal;                ///< Нормаль столкновения
    float approachSpeed = 0.0f;         ///< Скорость сближения в момент удара (пиксели/сек)

    /**
     * @brief Проверить валидность события
     * @return true если обе сущности валидны
     */
    bool isValid() const {
        return entityA != entt::null && entityB != entt::null;
    }
};

/**
 * @brief Событие входа в триггерную зону (сенсор)
 *
 * Генерируется когда любой shape начинает перекрываться с sensor shape.
 * Триггеры не создают физического отклика, только генерируют события.
 */
struct TriggerEnterEvent {
    entt::entity triggerEntity = entt::null;  ///< Сущность с триггером (isSensor = true)
    entt::entity otherEntity = entt::null;    ///< Сущность, вошедшая в триггер
    b2ShapeId triggerShapeId;                 ///< ID Box2D sensor shape
    b2ShapeId otherShapeId;                   ///< ID Box2D shape другой сущности

    /**
     * @brief Проверить валидность события
     * @return true если обе сущности валидны
     */
    bool isValid() const {
        return triggerEntity != entt::null && otherEntity != entt::null;
    }
};

/**
 * @brief Событие выхода из триггерной зоны (сенсор)
 *
 * Генерируется когда shape перестаёт перекрываться с sensor shape.
 */
struct TriggerExitEvent {
    entt::entity triggerEntity = entt::null;  ///< Сущность с триггером (isSensor = true)
    entt::entity otherEntity = entt::null;    ///< Сущность, вышедшая из триггера
    b2ShapeId triggerShapeId;                 ///< ID Box2D sensor shape
    b2ShapeId otherShapeId;                   ///< ID Box2D shape другой сущности

    /**
     * @brief Проверить валидность события
     * @return true если обе сущности валидны
     */
    bool isValid() const {
        return triggerEntity != entt::null && otherEntity != entt::null;
    }
};

/**
 * @brief Система сигналов для событий коллизий
 *
 * Использует boost::signals2 для thread-safe отправки событий.
 * Подписчики могут подключаться к сигналам для обработки событий.
 *
 * @code
 * CollisionSignals signals;
 *
 * // Подписка на событие начала коллизии
 * signals.onCollisionBegin.connect([](const CollisionBeginEvent& event) {
 *     LOG_INFO("Collision between {} and {}",
 *              static_cast<uint32_t>(event.entityA),
 *              static_cast<uint32_t>(event.entityB));
 * });
 *
 * // Подписка на вход в триггер
 * signals.onTriggerEnter.connect([](const TriggerEnterEvent& event) {
 *     LOG_INFO("Entity {} entered trigger {}",
 *              static_cast<uint32_t>(event.otherEntity),
 *              static_cast<uint32_t>(event.triggerEntity));
 * });
 * @endcode
 */
struct CollisionSignals {
    /// Сигнал начала столкновения
    boost::signals2::signal<void(const CollisionBeginEvent&)> onCollisionBegin;

    /// Сигнал окончания столкновения
    boost::signals2::signal<void(const CollisionEndEvent&)> onCollisionEnd;

    /// Сигнал удара (высокоскоростное столкновение)
    boost::signals2::signal<void(const CollisionHitEvent&)> onCollisionHit;

    /// Сигнал входа в триггерную зону
    boost::signals2::signal<void(const TriggerEnterEvent&)> onTriggerEnter;

    /// Сигнал выхода из триггерной зоны
    boost::signals2::signal<void(const TriggerExitEvent&)> onTriggerExit;

    /**
     * @brief Отключить все слоты от всех сигналов
     *
     * Полезно при очистке или деструкции.
     */
    void disconnectAll() {
        onCollisionBegin.disconnect_all_slots();
        onCollisionEnd.disconnect_all_slots();
        onCollisionHit.disconnect_all_slots();
        onTriggerEnter.disconnect_all_slots();
        onTriggerExit.disconnect_all_slots();
    }
};

} // namespace simulation
