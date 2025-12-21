#include "core/systems/AnimationSystemV2.h"
#include "core/Components.h"
#include "core/AnimationData.h"
#include "core/Logger.h"

namespace core {

AnimationSystemV2::AnimationSystemV2() {
    LOG_DEBUG("AnimationSystemV2 initialized");
}

void AnimationSystemV2::update(entt::registry& registry, double dt) {
    // Обновляем состояние анимаций
    updateAnimationState(registry, dt);

    // Синхронизируем textureRect с текущим кадром
    updateTextureRects(registry);
}

void AnimationSystemV2::updateAnimationState(entt::registry& registry, double dt) {
    // Получаем view всех сущностей с AnimationComponentV2
    auto view = registry.view<AnimationComponentV2>();

    for (auto entity : view) {
        auto& anim = view.get<AnimationComponentV2>(entity);

        // Пропускаем если анимация не воспроизводится или нет данных
        if (!anim.playing || !anim.hasAnimationData()) {
            continue;
        }

        // Получаем текущее определение анимации
        const auto* animDef = anim.getCurrentAnimationDef();
        if (!animDef || animDef->frames.empty()) {
            LOG_WARN("AnimationSystemV2: entity {} has invalid animation definition",
                     static_cast<uint32_t>(entity));
            continue;
        }

        // Проверяем корректность текущего кадра
        if (anim.currentFrame < 0 || anim.currentFrame >= static_cast<int>(animDef->frames.size())) {
            LOG_WARN("AnimationSystemV2: entity {} has invalid frame index {}/{}",
                     static_cast<uint32_t>(entity), anim.currentFrame, animDef->frames.size());
            anim.currentFrame = 0;
        }

        // Получаем текущий кадр
        const auto& currentFrameData = animDef->frames[anim.currentFrame];

        // Накапливаем время
        anim.elapsedTime += sf::seconds(static_cast<float>(dt));

        // Проверяем, нужно ли переключить кадр
        // Для статичных кадров (duration = 0) не переключаем
        if (currentFrameData.duration > sf::Time::Zero &&
            anim.elapsedTime >= currentFrameData.duration) {

            // Сбрасываем накопленное время
            anim.elapsedTime -= currentFrameData.duration;

            // Переключаем на следующий кадр
            anim.currentFrame++;

            // Проверяем выход за границы
            if (anim.currentFrame >= static_cast<int>(animDef->frames.size())) {
                if (animDef->loop) {
                    // Зацикливаем анимацию
                    anim.currentFrame = 0;
                    LOG_TRACE("AnimationSystemV2: entity {} looped animation '{}'",
                             static_cast<uint32_t>(entity), animDef->name);
                } else {
                    // Останавливаем на последнем кадре
                    anim.currentFrame = animDef->frames.size() - 1;
                    anim.playing = false;
                    LOG_DEBUG("AnimationSystemV2: entity {} finished animation '{}'",
                             static_cast<uint32_t>(entity), animDef->name);
                }
            }
        }
    }
}

void AnimationSystemV2::updateTextureRects(entt::registry& registry) {
    // Получаем view сущностей с AnimationV2 и Sprite
    auto view = registry.view<AnimationComponentV2, SpriteComponent>();

    for (auto entity : view) {
        auto& anim = view.get<AnimationComponentV2>(entity);
        auto& sprite = view.get<SpriteComponent>(entity);

        // Пропускаем если нет данных
        if (!anim.hasAnimationData()) {
            continue;
        }

        // Получаем текущий кадр
        const auto* currentFrame = anim.getCurrentFrame();
        if (!currentFrame) {
            continue;
        }

        // Обновляем textureRect только если он изменился
        if (sprite.textureRect.position != currentFrame->textureRect.position ||
            sprite.textureRect.size != currentFrame->textureRect.size) {

            sprite.textureRect = currentFrame->textureRect;

            LOG_TRACE("AnimationSystemV2: entity {} updated textureRect to ({}, {}) size ({}, {})",
                     static_cast<uint32_t>(entity),
                     currentFrame->textureRect.position.x,
                     currentFrame->textureRect.position.y,
                     currentFrame->textureRect.size.x,
                     currentFrame->textureRect.size.y);
        }
    }
}

} // namespace core
