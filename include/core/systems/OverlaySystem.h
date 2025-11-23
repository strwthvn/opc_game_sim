#pragma once

#include "core/systems/ISystem.h"
#include <entt/entt.hpp>

namespace core {

/**
 * @brief Система синхронизации оверлеев с родительскими объектами
 *
 * Обновляет позиции оверлеев (индикаторов, кнопок) на основе
 * позиции родительской сущности через ParentComponent.
 * Применяет localOffset из OverlayComponent.
 */
class OverlaySystem : public ISystem {
public:
    /**
     * @brief Конструктор
     */
    OverlaySystem();

    /**
     * @brief Обновление позиций всех оверлеев
     *
     * Синхронизирует позицию оверлеев с родительскими объектами,
     * применяя локальное смещение.
     *
     * @param registry EnTT registry с сущностями
     * @param dt Время с последнего обновления (не используется)
     */
    void update(entt::registry& registry, double dt) override;

    /**
     * @brief Обновление позиций всех оверлеев (старый API)
     * @param registry EnTT registry с сущностями
     */
    void update(entt::registry& registry);

    int getPriority() const override { return 400; }
    const char* getName() const override { return "OverlaySystem"; }

private:
    /**
     * @brief Синхронизирует позицию оверлея с родителем
     *
     * Копирует позицию родителя в TransformComponent оверлея
     * и добавляет localOffset.
     *
     * @param registry EnTT registry
     */
    void syncOverlayPositions(entt::registry& registry);
};

} // namespace core
