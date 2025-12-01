#pragma once

#include "core/systems/ISystem.h"
#include <SFML/Graphics/Rect.hpp>
#include <entt/entt.hpp>
#include <set>
#include <utility>

namespace core {

/**
 * @brief Система обработки коллизий
 *
 * Проверяет AABB коллизии между сущностями с CollisionComponent.
 * Поддерживает solid коллизии (блокирующие движение) и trigger коллизии (только детекция).
 * Вызывает коллбеки onCollisionEnter/Stay/Exit для событий коллизий.
 *
 * Приоритет: 100 (после UpdateSystem, до TilePositionSystem)
 */
class CollisionSystem : public ISystem {
public:
    /**
     * @brief Конструктор
     */
    CollisionSystem();

    /**
     * @brief Деструктор
     */
    ~CollisionSystem() override;

    /**
     * @brief Обновление системы коллизий
     *
     * Проверяет все пары сущностей с CollisionComponent на столкновения.
     * Вызывает соответствующие коллбеки для событий коллизий.
     *
     * @param registry EnTT registry с сущностями
     * @param dt Время с последнего обновления (секунды)
     */
    void update(entt::registry& registry, double dt) override;

    /**
     * @brief Получить приоритет системы
     * @return Приоритет (100 - после UpdateSystem, до других систем)
     */
    int getPriority() const override { return 100; }

    /**
     * @brief Получить имя системы
     * @return Имя системы
     */
    const char* getName() const override { return "CollisionSystem"; }

private:
    /**
     * @brief Пара сущностей для отслеживания коллизий
     *
     * Всегда хранит меньший entity первым для уникальности пар.
     */
    struct EntityPair {
        entt::entity first;
        entt::entity second;

        EntityPair(entt::entity a, entt::entity b)
            : first(a < b ? a : b)
            , second(a < b ? b : a) {
        }

        bool operator<(const EntityPair& other) const {
            if (first != other.first)
                return first < other.first;
            return second < other.second;
        }

        bool operator==(const EntityPair& other) const {
            return first == other.first && second == other.second;
        }
    };

    /**
     * @brief Активные коллизии с предыдущего кадра
     *
     * Используется для определения onCollisionEnter и onCollisionExit.
     */
    std::set<EntityPair> m_previousCollisions;

    /**
     * @brief Проверить AABB пересечение двух прямоугольников
     * @param a Первый прямоугольник
     * @param b Второй прямоугольник
     * @return true если прямоугольники пересекаются
     */
    bool checkAABB(const sf::FloatRect& a, const sf::FloatRect& b) const;

    /**
     * @brief Обработать коллизию между двумя сущностями
     *
     * Вызывает соответствующие коллбеки (onCollisionEnter или onCollisionStay).
     *
     * @param registry EnTT registry
     * @param entityA Первая сущность
     * @param entityB Вторая сущность
     * @param isNewCollision true если это новая коллизия (для onCollisionEnter)
     */
    void handleCollision(entt::registry& registry, entt::entity entityA, entt::entity entityB, bool isNewCollision);

    /**
     * @brief Обработать выход из коллизии
     *
     * Вызывает onCollisionExit для обеих сущностей.
     *
     * @param registry EnTT registry
     * @param pair Пара сущностей, которые перестали сталкиваться
     */
    void handleCollisionExit(entt::registry& registry, const EntityPair& pair);
};

} // namespace core
