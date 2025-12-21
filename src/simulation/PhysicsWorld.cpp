#include "PhysicsWorld.h"

namespace simulation {

PhysicsWorld::PhysicsWorld(const b2Vec2& gravity) {
    // Создать определение мира с заданной гравитацией
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = gravity;

    // Создать Box2D мир (Box2D 3.x использует ID-based API)
    m_worldId = b2CreateWorld(&worldDef);
}

PhysicsWorld::~PhysicsWorld() {
    // Уничтожить Box2D мир если он валиден
    if (b2World_IsValid(m_worldId)) {
        b2DestroyWorld(m_worldId);
    }
}

void PhysicsWorld::step(float deltaTime) {
    // Выполнить шаг симуляции с sub-stepping для точности
    // Box2D 3.x использует sub-steps вместо velocity/position iterations
    b2World_Step(m_worldId, deltaTime, SUB_STEP_COUNT);
}

void PhysicsWorld::setGravity(const b2Vec2& gravity) {
    b2World_SetGravity(m_worldId, gravity);
}

b2Vec2 PhysicsWorld::getGravity() const {
    return b2World_GetGravity(m_worldId);
}

b2WorldId PhysicsWorld::getWorldId() const {
    return m_worldId;
}

bool PhysicsWorld::isValid() const {
    return b2World_IsValid(m_worldId);
}

} // namespace simulation
