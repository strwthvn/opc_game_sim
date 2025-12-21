#pragma once

#include "core/systems/ISystem.h"
#include <entt/entt.hpp>

namespace core {

/**
 * @brief Система обновления анимаций с поддержкой JSON метаданных
 *
 * Обновляет AnimationComponentV2, переключая кадры по времени
 * и устанавливая соответствующий textureRect в SpriteComponent.
 * Поддерживает множественные анимации с разной длительностью кадров.
 */
class AnimationSystemV2 : public ISystem {
public:
    /**
     * @brief Конструктор
     */
    AnimationSystemV2();

    /**
     * @brief Обновление всех анимаций
     *
     * Обновляет время анимации, переключает кадры и синхронизирует
     * textureRect в SpriteComponent.
     *
     * @param registry EnTT registry с сущностями
     * @param dt Время с последнего обновления (секунды)
     */
    void update(entt::registry& registry, double dt) override;

    int getPriority() const override { return 300; }
    const char* getName() const override { return "AnimationSystemV2"; }

private:
    /**
     * @brief Обновляет состояние анимации
     *
     * Увеличивает elapsedTime, переключает кадры при необходимости,
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
     * текущего кадра из AnimationComponentV2.
     *
     * @param registry EnTT registry
     */
    void updateTextureRects(entt::registry& registry);
};

} // namespace core
