#pragma once

#include "events/CollisionEvents.h"
#include "PhysicsWorld.h"
#include <entt/entt.hpp>
#include <box2d/box2d.h>

/**
 * @file PhysicsEventProcessor.h
 * @brief Обработчик событий Box2D 3.x для ECS
 *
 * PhysicsEventProcessor извлекает события коллизий и сенсоров из Box2D
 * после каждого шага симуляции и преобразует их в ECS события.
 *
 * Box2D 3.x использует event-based подход: события накапливаются во время
 * симуляции и доступны для обработки после вызова b2World_Step().
 *
 * @note Для связи Box2D shape с entt::entity используется userData в b2BodyDef.
 */

namespace simulation {

/**
 * @brief Обработчик событий физики Box2D 3.x
 *
 * Обрабатывает события контактов и сенсоров после каждого шага симуляции.
 * Преобразует Box2D события в ECS события и отправляет их через CollisionSignals.
 *
 * **Использование:**
 * @code
 * PhysicsWorld world(b2Vec2{0.0f, 9.8f});
 * entt::registry registry;
 * PhysicsEventProcessor processor(world, registry);
 *
 * // Подписка на события
 * processor.getSignals().onCollisionBegin.connect([](const CollisionBeginEvent& e) {
 *     // Обработка начала коллизии
 * });
 *
 * // После каждого шага симуляции:
 * world.step(deltaTime);
 * processor.processEvents();
 * @endcode
 *
 * **Типы событий:**
 * - CollisionBeginEvent: начало контакта между non-sensor shapes
 * - CollisionEndEvent: конец контакта между non-sensor shapes
 * - CollisionHitEvent: высокоскоростное столкновение (выше порога)
 * - TriggerEnterEvent: начало перекрытия с sensor shape
 * - TriggerExitEvent: конец перекрытия с sensor shape
 */
class PhysicsEventProcessor {
public:
    /**
     * @brief Порог скорости для генерации CollisionHitEvent (в м/с)
     *
     * Столкновения со скоростью ниже этого порога не генерируют HitEvent.
     * По умолчанию: 1.0 м/с
     */
    static constexpr float HIT_SPEED_THRESHOLD = 1.0f;

    /**
     * @brief Конструктор
     *
     * @param world Ссылка на PhysicsWorld
     * @param registry Ссылка на EnTT registry для получения entity из userData
     */
    PhysicsEventProcessor(PhysicsWorld& world, entt::registry& registry);

    /**
     * @brief Деструктор
     *
     * Отключает все слоты сигналов.
     */
    ~PhysicsEventProcessor();

    // Запретить копирование
    PhysicsEventProcessor(const PhysicsEventProcessor&) = delete;
    PhysicsEventProcessor& operator=(const PhysicsEventProcessor&) = delete;

    /**
     * @brief Обработать все события после шага симуляции
     *
     * Извлекает события контактов и сенсоров из Box2D и отправляет их
     * через CollisionSignals. Должен вызываться после каждого b2World_Step().
     *
     * @note Безопасно вызывать даже если событий нет.
     */
    void processEvents();

    /**
     * @brief Получить доступ к сигналам событий
     *
     * @return Ссылка на CollisionSignals для подписки на события
     */
    CollisionSignals& getSignals() { return m_signals; }

    /**
     * @brief Получить доступ к сигналам событий (const)
     *
     * @return Константная ссылка на CollisionSignals
     */
    const CollisionSignals& getSignals() const { return m_signals; }

    /**
     * @brief Установить порог скорости для CollisionHitEvent
     *
     * @param threshold Минимальная скорость столкновения (м/с)
     */
    void setHitSpeedThreshold(float threshold) { m_hitSpeedThreshold = threshold; }

    /**
     * @brief Получить текущий порог скорости
     *
     * @return Порог скорости в м/с
     */
    float getHitSpeedThreshold() const { return m_hitSpeedThreshold; }

private:
    PhysicsWorld& m_world;          ///< Ссылка на физический мир
    entt::registry& m_registry;     ///< Ссылка на ECS registry
    CollisionSignals m_signals;     ///< Сигналы для отправки событий
    float m_hitSpeedThreshold;      ///< Порог скорости для HitEvent

    /**
     * @brief Обработать события контактов (touch begin/end)
     */
    void processContactEvents();

    /**
     * @brief Обработать события сенсоров (sensor overlap begin/end)
     */
    void processSensorEvents();

    /**
     * @brief Получить entt::entity из Box2D body
     *
     * Извлекает entity из userData, установленного при создании тела.
     *
     * @param bodyId ID Box2D тела
     * @return entt::entity или entt::null если не найден
     */
    entt::entity getEntityFromBody(b2BodyId bodyId) const;

    /**
     * @brief Получить entt::entity из Box2D shape
     *
     * @param shapeId ID Box2D shape
     * @return entt::entity или entt::null если не найден
     */
    entt::entity getEntityFromShape(b2ShapeId shapeId) const;

    /**
     * @brief Конвертировать Box2D точку в пиксельные координаты
     *
     * @param point Точка в метрах
     * @return Точка в пикселях
     */
    sf::Vector2f toPixelCoords(b2Vec2 point) const;
};

} // namespace simulation
