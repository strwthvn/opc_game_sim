#pragma once

#include "core/systems/ISystem.h"
#include <entt/entt.hpp>

namespace core {

/**
 * @brief Система обновления ECS сущностей
 *
 * Обновляет позиции сущностей на основе их скорости
 * Применяет простую физику (без коллизий)
 */
class UpdateSystem : public ISystem {
public:
    /**
     * @brief Конструктор
     */
    UpdateSystem();

    /**
     * @brief Обновление всех сущностей
     * @param registry EnTT registry с сущностями
     * @param dt Время с последнего обновления (секунды)
     */
    void update(entt::registry& registry, double dt) override;

    /**
     * @brief Получить приоритет системы
     * @return Приоритет (0 - высокий приоритет, выполняется первой)
     */
    int getPriority() const override { return 0; }

    /**
     * @brief Получить имя системы
     * @return Имя системы
     */
    const char* getName() const override { return "UpdateSystem"; }

private:
    /**
     * @brief Обновляет позиции на основе скорости
     * @param registry EnTT registry
     * @param dt Delta time
     */
    void updateMovement(entt::registry& registry, double dt);
};

} // namespace core
