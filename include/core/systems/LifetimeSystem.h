#pragma once

#include "core/systems/ISystem.h"
#include <entt/entt.hpp>

namespace core {

/**
 * @brief Система обработки времени жизни сущностей
 *
 * Уменьшает lifetime компонента и уничтожает сущности при достижении нуля
 */
class LifetimeSystem : public ISystem {
public:
    /**
     * @brief Обновляет время жизни всех сущностей
     * @param registry EnTT registry
     * @param dt Время кадра в секундах
     */
    void update(entt::registry& registry, double dt) override;

    int getPriority() const override { return 50; }
    const char* getName() const override { return "LifetimeSystem"; }

private:
    /**
     * @brief Обрабатывает одну сущность с LifetimeComponent
     * @param registry EnTT registry
     * @param entity Сущность для обработки
     * @param dt Время кадра в секундах
     */
    void processEntity(entt::registry& registry, entt::entity entity, double dt);
};

} // namespace core
