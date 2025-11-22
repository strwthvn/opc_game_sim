#pragma once

#include <entt/entt.hpp>

namespace core {

/**
 * @brief Система обновления покадровой анимации
 *
 * Обновляет AnimationComponent, переключая кадры по времени
 * и устанавливая соответствующий textureRect в SpriteComponent.
 * Поддерживает горизонтальные спрайт-листы.
 */
class AnimationSystem {
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
    void update(entt::registry& registry, double dt);

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
