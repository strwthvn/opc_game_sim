#include "core/systems/CollisionSystem.h"
#include "core/Components.h"
#include "core/Logger.h"

namespace core {

CollisionSystem::CollisionSystem() {
    LOG_DEBUG("CollisionSystem initialized");
}

CollisionSystem::~CollisionSystem() = default;

void CollisionSystem::update(entt::registry& registry, double dt) {
    // Получаем view всех сущностей с коллизиями
    auto view = registry.view<TransformComponent, CollisionComponent>();

    // Множество текущих активных коллизий
    std::set<EntityPair> currentCollisions;

    // Проверяем все пары сущностей на коллизии
    for (auto it1 = view.begin(); it1 != view.end(); ++it1) {
        entt::entity entityA = *it1;
        const auto& transformA = view.get<TransformComponent>(entityA);
        const auto& collisionA = view.get<CollisionComponent>(entityA);

        // Вычисляем мировые границы первой сущности
        sf::FloatRect boundsA = collisionA.getWorldBounds(transformA);

        // Проверяем коллизии с остальными сущностями
        auto it2 = it1;
        ++it2;
        for (; it2 != view.end(); ++it2) {
            entt::entity entityB = *it2;
            const auto& transformB = view.get<TransformComponent>(entityB);
            const auto& collisionB = view.get<CollisionComponent>(entityB);

            // Вычисляем мировые границы второй сущности
            sf::FloatRect boundsB = collisionB.getWorldBounds(transformB);

            // Проверяем AABB пересечение
            if (checkAABB(boundsA, boundsB)) {
                // Создаем пару сущностей
                EntityPair pair(entityA, entityB);
                currentCollisions.insert(pair);

                // Проверяем, была ли эта коллизия в предыдущем кадре
                bool isNewCollision = m_previousCollisions.find(pair) == m_previousCollisions.end();

                // Обрабатываем коллизию
                handleCollision(registry, entityA, entityB, isNewCollision);
            }
        }
    }

    // Обрабатываем коллизии, которые закончились (были в предыдущем кадре, но нет сейчас)
    for (const auto& pair : m_previousCollisions) {
        if (currentCollisions.find(pair) == currentCollisions.end()) {
            handleCollisionExit(registry, pair);
        }
    }

    // Обновляем список активных коллизий для следующего кадра
    m_previousCollisions = std::move(currentCollisions);
}

bool CollisionSystem::checkAABB(const sf::FloatRect& a, const sf::FloatRect& b) const {
    // AABB пересечение: проверяем, не разделены ли прямоугольники по какой-либо оси
    return !(a.position.x + a.size.x < b.position.x ||  // A правее B
             b.position.x + b.size.x < a.position.x ||  // B правее A
             a.position.y + a.size.y < b.position.y ||  // A выше B
             b.position.y + b.size.y < a.position.y);   // B выше A
}

void CollisionSystem::handleCollision(entt::registry& registry, entt::entity entityA, entt::entity entityB, bool isNewCollision) {
    // Получаем компоненты коллизий
    auto* collisionA = registry.try_get<CollisionComponent>(entityA);
    auto* collisionB = registry.try_get<CollisionComponent>(entityB);

    // Проверяем, что компоненты еще существуют
    if (!collisionA || !collisionB) {
        return;
    }

    // Если это новая коллизия, вызываем onCollisionEnter
    if (isNewCollision) {
        if (collisionA->onCollisionEnter) {
            collisionA->onCollisionEnter(entityB);
        }
        if (collisionB->onCollisionEnter) {
            collisionB->onCollisionEnter(entityA);
        }

        LOG_TRACE("Collision ENTER: entity {} <-> entity {}", static_cast<uint32_t>(entityA), static_cast<uint32_t>(entityB));
    }
    // Иначе вызываем onCollisionStay (продолжающаяся коллизия)
    else {
        if (collisionA->onCollisionStay) {
            collisionA->onCollisionStay(entityB);
        }
        if (collisionB->onCollisionStay) {
            collisionB->onCollisionStay(entityA);
        }
    }
}

void CollisionSystem::handleCollisionExit(entt::registry& registry, const EntityPair& pair) {
    // Получаем компоненты коллизий
    auto* collisionA = registry.try_get<CollisionComponent>(pair.first);
    auto* collisionB = registry.try_get<CollisionComponent>(pair.second);

    // Вызываем коллбеки выхода из коллизии
    if (collisionA && collisionA->onCollisionExit) {
        collisionA->onCollisionExit(pair.second);
    }
    if (collisionB && collisionB->onCollisionExit) {
        collisionB->onCollisionExit(pair.first);
    }

    LOG_TRACE("Collision EXIT: entity {} <-> entity {}", static_cast<uint32_t>(pair.first), static_cast<uint32_t>(pair.second));
}

} // namespace core
