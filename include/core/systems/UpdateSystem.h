#pragma once

#include <entt/entt.hpp>

namespace core {

/**
 * @brief Система обновления ECS сущностей
 *
 * Обновляет позиции сущностей на основе их скорости
 * Применяет простую физику (без коллизий)
 */
class UpdateSystem {
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
    void update(entt::registry& registry, double dt);

private:
    /**
     * @brief Обновляет позиции на основе скорости
     * @param registry EnTT registry
     * @param dt Delta time
     */
    void updateMovement(entt::registry& registry, double dt);
};

} // namespace core
