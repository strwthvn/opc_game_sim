#include "PhysicsEventProcessor.h"
#include "core/Logger.h"

namespace simulation {

PhysicsEventProcessor::PhysicsEventProcessor(PhysicsWorld& world, entt::registry& registry)
    : m_world(world), m_registry(registry), m_hitSpeedThreshold(HIT_SPEED_THRESHOLD) {
    LOG_DEBUG("PhysicsEventProcessor initialized");
}

PhysicsEventProcessor::~PhysicsEventProcessor() {
    m_signals.disconnectAll();
    LOG_DEBUG("PhysicsEventProcessor destroyed");
}

void PhysicsEventProcessor::processEvents() {
    processContactEvents();
    processSensorEvents();
}

void PhysicsEventProcessor::processContactEvents() {
    b2WorldId worldId = m_world.getWorldId();

    // Получаем события контактов
    b2ContactEvents contactEvents = b2World_GetContactEvents(worldId);

    // Обработка BeginTouch событий (начало контакта)
    for (int i = 0; i < contactEvents.beginCount; ++i) {
        const b2ContactBeginTouchEvent& event = contactEvents.beginEvents[i];

        // Получаем entity для каждого shape
        entt::entity entityA = getEntityFromShape(event.shapeIdA);
        entt::entity entityB = getEntityFromShape(event.shapeIdB);

        if (entityA == entt::null || entityB == entt::null) {
            LOG_WARN("PhysicsEventProcessor: Could not resolve entities for contact begin event");
            continue;
        }

        // Формируем и отправляем событие
        CollisionBeginEvent collisionEvent;
        collisionEvent.entityA = entityA;
        collisionEvent.entityB = entityB;
        collisionEvent.shapeIdA = event.shapeIdA;
        collisionEvent.shapeIdB = event.shapeIdB;

        // Получаем информацию о контакте из manifold
        // Box2D 3.x: manifold содержит точки контакта
        collisionEvent.contactPoint.position = sf::Vector2f(0.0f, 0.0f);
        collisionEvent.contactPoint.normal = sf::Vector2f(0.0f, 0.0f);
        collisionEvent.contactPoint.separation = 0.0f;

        m_signals.onCollisionBegin(collisionEvent);

        LOG_TRACE("Collision begin: entity {} <-> entity {}",
                  static_cast<uint32_t>(entityA), static_cast<uint32_t>(entityB));
    }

    // Обработка EndTouch событий (конец контакта)
    for (int i = 0; i < contactEvents.endCount; ++i) {
        const b2ContactEndTouchEvent& event = contactEvents.endEvents[i];

        // Проверяем валидность shapes (могут быть уже удалены)
        if (!b2Shape_IsValid(event.shapeIdA) || !b2Shape_IsValid(event.shapeIdB)) {
            continue;
        }

        entt::entity entityA = getEntityFromShape(event.shapeIdA);
        entt::entity entityB = getEntityFromShape(event.shapeIdB);

        if (entityA == entt::null || entityB == entt::null) {
            continue;
        }

        CollisionEndEvent collisionEvent;
        collisionEvent.entityA = entityA;
        collisionEvent.entityB = entityB;
        collisionEvent.shapeIdA = event.shapeIdA;
        collisionEvent.shapeIdB = event.shapeIdB;

        m_signals.onCollisionEnd(collisionEvent);

        LOG_TRACE("Collision end: entity {} <-> entity {}",
                  static_cast<uint32_t>(entityA), static_cast<uint32_t>(entityB));
    }

    // Обработка Hit событий (высокоскоростные столкновения)
    for (int i = 0; i < contactEvents.hitCount; ++i) {
        const b2ContactHitEvent& event = contactEvents.hitEvents[i];

        // Проверяем порог скорости (event.approachSpeed в м/с)
        if (event.approachSpeed < m_hitSpeedThreshold) {
            continue;
        }

        entt::entity entityA = getEntityFromShape(event.shapeIdA);
        entt::entity entityB = getEntityFromShape(event.shapeIdB);

        if (entityA == entt::null || entityB == entt::null) {
            continue;
        }

        CollisionHitEvent hitEvent;
        hitEvent.entityA = entityA;
        hitEvent.entityB = entityB;
        hitEvent.shapeIdA = event.shapeIdA;
        hitEvent.shapeIdB = event.shapeIdB;
        hitEvent.point = toPixelCoords(event.point);
        hitEvent.normal = sf::Vector2f(event.normal.x, event.normal.y);
        hitEvent.approachSpeed = PhysicsWorld::metersToPixels(event.approachSpeed);

        m_signals.onCollisionHit(hitEvent);

        LOG_TRACE("Collision hit: entity {} <-> entity {}, speed: {:.2f} px/s",
                  static_cast<uint32_t>(entityA), static_cast<uint32_t>(entityB),
                  hitEvent.approachSpeed);
    }
}

void PhysicsEventProcessor::processSensorEvents() {
    b2WorldId worldId = m_world.getWorldId();

    // Получаем события сенсоров
    b2SensorEvents sensorEvents = b2World_GetSensorEvents(worldId);

    // Обработка BeginTouch событий сенсоров
    for (int i = 0; i < sensorEvents.beginCount; ++i) {
        const b2SensorBeginTouchEvent& event = sensorEvents.beginEvents[i];

        entt::entity sensorEntity = getEntityFromShape(event.sensorShapeId);
        entt::entity visitorEntity = getEntityFromShape(event.visitorShapeId);

        if (sensorEntity == entt::null || visitorEntity == entt::null) {
            LOG_WARN("PhysicsEventProcessor: Could not resolve entities for sensor begin event");
            continue;
        }

        TriggerEnterEvent triggerEvent;
        triggerEvent.triggerEntity = sensorEntity;
        triggerEvent.otherEntity = visitorEntity;
        triggerEvent.triggerShapeId = event.sensorShapeId;
        triggerEvent.otherShapeId = event.visitorShapeId;

        m_signals.onTriggerEnter(triggerEvent);

        LOG_TRACE("Trigger enter: entity {} entered trigger {}",
                  static_cast<uint32_t>(visitorEntity), static_cast<uint32_t>(sensorEntity));
    }

    // Обработка EndTouch событий сенсоров
    for (int i = 0; i < sensorEvents.endCount; ++i) {
        const b2SensorEndTouchEvent& event = sensorEvents.endEvents[i];

        // Проверяем валидность shapes
        if (!b2Shape_IsValid(event.sensorShapeId) || !b2Shape_IsValid(event.visitorShapeId)) {
            continue;
        }

        entt::entity sensorEntity = getEntityFromShape(event.sensorShapeId);
        entt::entity visitorEntity = getEntityFromShape(event.visitorShapeId);

        if (sensorEntity == entt::null || visitorEntity == entt::null) {
            continue;
        }

        TriggerExitEvent triggerEvent;
        triggerEvent.triggerEntity = sensorEntity;
        triggerEvent.otherEntity = visitorEntity;
        triggerEvent.triggerShapeId = event.sensorShapeId;
        triggerEvent.otherShapeId = event.visitorShapeId;

        m_signals.onTriggerExit(triggerEvent);

        LOG_TRACE("Trigger exit: entity {} exited trigger {}",
                  static_cast<uint32_t>(visitorEntity), static_cast<uint32_t>(sensorEntity));
    }
}

entt::entity PhysicsEventProcessor::getEntityFromBody(b2BodyId bodyId) const {
    if (!b2Body_IsValid(bodyId)) {
        return entt::null;
    }

    // Получаем userData из body
    void* userData = b2Body_GetUserData(bodyId);
    if (userData == nullptr) {
        return entt::null;
    }

    // userData содержит entity + 1 (чтобы избежать nullptr для entity = 0)
    // Вычитаем 1 для получения реального значения entity
    uintptr_t value = reinterpret_cast<uintptr_t>(userData);
    return static_cast<entt::entity>(value - 1);
}

entt::entity PhysicsEventProcessor::getEntityFromShape(b2ShapeId shapeId) const {
    if (!b2Shape_IsValid(shapeId)) {
        return entt::null;
    }

    // Получаем body из shape, затем entity из body
    b2BodyId bodyId = b2Shape_GetBody(shapeId);
    return getEntityFromBody(bodyId);
}

sf::Vector2f PhysicsEventProcessor::toPixelCoords(b2Vec2 point) const {
    b2Vec2 pixels = PhysicsWorld::metersToPixels(point);
    return sf::Vector2f(pixels.x, pixels.y);
}

} // namespace simulation
