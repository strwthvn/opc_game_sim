#pragma once

#include <entt/entt.hpp>

namespace core {

/**
 * @brief Система синхронизации тайловых и пиксельных координат
 *
 * Синхронизирует TilePositionComponent с TransformComponent,
 * обеспечивая корректное позиционирование объектов на тайловой сетке.
 * Также вычисляет layer для Y-sorting (перспектива 3/4).
 */
class TilePositionSystem {
public:
    /**
     * @brief Конструктор
     */
    TilePositionSystem();

    /**
     * @brief Обновление позиций всех тайловых объектов
     *
     * Синхронизирует тайловые координаты с пиксельными координатами
     * в TransformComponent и вычисляет layer для корректного Z-ordering.
     *
     * @param registry EnTT registry с сущностями
     */
    void update(entt::registry& registry);

private:
    /**
     * @brief Синхронизирует тайловую позицию с трансформом
     *
     * Конвертирует тайловые координаты в пиксельные и обновляет
     * TransformComponent.
     *
     * @param registry EnTT registry
     */
    void syncPositions(entt::registry& registry);

    /**
     * @brief Вычисляет layer для Y-sorting
     *
     * Объекты на слое Objects сортируются по Y-координате
     * для создания эффекта перспективы 3/4.
     *
     * @param registry EnTT registry
     */
    void updateLayers(entt::registry& registry);
};

} // namespace core
