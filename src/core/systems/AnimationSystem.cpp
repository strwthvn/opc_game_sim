#include "core/systems/AnimationSystem.h"
#include "core/Components.h"
#include "core/Logger.h"

namespace core {

AnimationSystem::AnimationSystem() {
    LOG_DEBUG("AnimationSystem initialized");
}

void AnimationSystem::update(entt::registry& registry, double dt) {
    // Обновляем состояние анимаций
    updateAnimationState(registry, dt);

    // Синхронизируем textureRect с текущим кадром
    updateTextureRects(registry);
}

void AnimationSystem::updateAnimationState(entt::registry& registry, double dt) {
    // Получаем view всех сущностей с AnimationComponent
    auto view = registry.view<AnimationComponent>();

    for (auto entity : view) {
        auto& anim = view.get<AnimationComponent>(entity);

        // Пропускаем если анимация не воспроизводится
        if (!anim.playing) {
            continue;
        }

        // Накапливаем время
        anim.frameTime += static_cast<float>(dt);

        // Проверяем, нужно ли переключить кадр
        if (anim.frameTime >= anim.frameDelay) {
            anim.frameTime -= anim.frameDelay;

            // Переключаем на следующий кадр
            anim.currentFrame++;

            // Проверяем выход за границы
            if (anim.currentFrame >= anim.frameCount) {
                if (anim.loop) {
                    // Зацикливаем анимацию
                    anim.currentFrame = 0;
                } else {
                    // Останавливаем на последнем кадре
                    anim.currentFrame = anim.frameCount - 1;
                    anim.playing = false;
                }
            }
        }
    }
}

void AnimationSystem::updateTextureRects(entt::registry& registry) {
    // Получаем view сущностей с Animation и Sprite
    auto view = registry.view<AnimationComponent, SpriteComponent>();

    for (auto entity : view) {
        auto& anim = view.get<AnimationComponent>(entity);
        auto& sprite = view.get<SpriteComponent>(entity);

        // Получаем прямоугольник текущего кадра
        sf::IntRect frameRect = anim.getCurrentFrameRect();

        // Обновляем textureRect только если он изменился
        if (sprite.textureRect.position != frameRect.position ||
            sprite.textureRect.size != frameRect.size) {
            sprite.textureRect = frameRect;

            // Помечаем кеш спрайта как требующий обновления
            sprite.markDirty();
        }
    }
}

} // namespace core
