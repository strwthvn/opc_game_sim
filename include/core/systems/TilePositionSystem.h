#pragma once

#include "core/systems/ISystem.h"
#include <entt/entt.hpp>

namespace core {

/**
 * @brief Система синхронизации тайловых и пиксельных координат
 *
 * Синхронизирует TilePositionComponent с TransformComponent,
 * обеспечивая корректное позиционирование объектов на тайловой сетке.
 * Также вычисляет layer для Y-sorting (перспектива 3/4).
 *
 * Использует EnTT observers для реактивного обновления только измененных
 * объектов, что значительно повышает производительность при большом
 * количестве статичных объектов.
 */
class TilePositionSystem : public ISystem {
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
     * Обрабатывает только измененные объекты благодаря observers.
     *
     * @param registry EnTT registry с сущностями
     * @param dt Время с последнего обновления (не используется)
     */
    void update(entt::registry& registry, double dt) override;

    /**
     * @brief Обновление позиций всех тайловых объектов (старый API)
     * @param registry EnTT registry с сущностями
     */
    void update(entt::registry& registry);

    int getPriority() const override { return 200; }
    const char* getName() const override { return "TilePositionSystem"; }

private:
    /**
     * @brief Синхронизирует тайловую позицию с трансформом
     *
     * Конвертирует тайловые координаты в пиксельные и обновляет
     * TransformComponent. Обрабатывает только измененные сущности
     * из observer.
     *
     * @param registry EnTT registry
     */
    void syncPositions(entt::registry& registry);

    /**
     * @brief Вычисляет layer для Y-sorting
     *
     * Объекты на слое Objects сортируются по Y-координате
     * для создания эффекта перспективы 3/4.
     * Обрабатывает только измененные сущности из observer.
     *
     * @param registry EnTT registry
     */
    void updateLayers(entt::registry& registry);
};

} // namespace core
