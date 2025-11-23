#pragma once

#include "core/systems/ISystem.h"
#include <entt/entt.hpp>

namespace core {

/**
 * @brief Система обновления покадровой анимации
 *
 * Обновляет AnimationComponent, переключая кадры по времени
 * и устанавливая соответствующий textureRect в SpriteComponent.
 * Поддерживает горизонтальные спрайт-листы.
 */
class AnimationSystem : public ISystem {
public:
    /**
     * @brief Конструктор
     */
    AnimationSystem();

    /**
     * @brief Обновление всех анимаций
     *
     * Обновляет frameTime, переключает кадры и синхронизирует
     * textureRect в SpriteComponent.
     *
     * @param registry EnTT registry с сущностями
     * @param dt Время с последнего обновления (секунды)
     */
    void update(entt::registry& registry, double dt) override;

    int getPriority() const override { return 300; }
    const char* getName() const override { return "AnimationSystem"; }

private:
    /**
     * @brief Обновляет состояние анимации
     *
     * Увеличивает frameTime, переключает кадры при необходимости,
     * обрабатывает зацикливание.
     *
     * @param registry EnTT registry
     * @param dt Delta time
     */
    void updateAnimationState(entt::registry& registry, double dt);

    /**
     * @brief Синхронизирует textureRect с текущим кадром
     *
     * Устанавливает textureRect в SpriteComponent на основе
     * currentFrame из AnimationComponent.
     *
     * @param registry EnTT registry
     */
    void updateTextureRects(entt::registry& registry);
};

} // namespace core
