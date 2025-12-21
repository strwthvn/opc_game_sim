#include "core/Components.h"
#include "core/AnimationData.h"
#include "core/Logger.h"
#include "core/EventBus.h"

namespace core {

void EntityStateComponent::setState(const std::string& newState, entt::entity entity) {
    // Игнорируем, если пытаемся установить то же состояние
    if (currentState == newState) {
        return;
    }

    // Вызываем onExit для текущего состояния
    if (!currentState.empty()) {
        auto exitIt = onExitCallbacks.find(currentState);
        if (exitIt != onExitCallbacks.end() && exitIt->second) {
            exitIt->second();
        }
    }

    // Сохраняем предыдущее состояние
    previousState = currentState;

    // Обновляем текущее состояние
    currentState = newState;

    // Сбрасываем время в состоянии
    timeInState = 0.0f;

    // Вызываем onEnter для нового состояния
    if (!currentState.empty()) {
        auto enterIt = onEnterCallbacks.find(currentState);
        if (enterIt != onEnterCallbacks.end() && enterIt->second) {
            enterIt->second();
        }
    }

    LOG_TRACE("EntityStateComponent: {} -> {}", previousState.empty() ? "none" : previousState, currentState);

    // Публикуем событие изменения состояния (если передана сущность)
    if (entity != entt::null) {
        StateChangedEvent event(entity, previousState, currentState);
        EventBus::getInstance().publish(event);
    }
}

// ========== AnimationComponentV2 Implementation ==========

bool AnimationComponentV2::setAnimation(const std::string& name, bool restart) {
    if (!animationData) {
        LOG_WARN("AnimationComponentV2::setAnimation: no animation data");
        return false;
    }

    if (!animationData->hasAnimation(name)) {
        LOG_WARN("AnimationComponentV2::setAnimation: animation '{}' not found", name);
        return false;
    }

    // Если это та же анимация и не нужно перезапускать
    if (currentAnimation == name && !restart) {
        return true;
    }

    // Переключаем анимацию
    currentAnimation = name;

    if (restart) {
        currentFrame = 0;
        elapsedTime = sf::Time::Zero;
        playing = true;
    }

    LOG_DEBUG("AnimationComponentV2: switched to animation '{}'", name);
    return true;
}

const AnimationDefinition* AnimationComponentV2::getCurrentAnimationDef() const {
    if (!animationData || currentAnimation.empty()) {
        return nullptr;
    }

    return animationData->getAnimation(currentAnimation);
}

const AnimationFrameData* AnimationComponentV2::getCurrentFrame() const {
    const auto* animDef = getCurrentAnimationDef();
    if (!animDef || currentFrame < 0 || currentFrame >= static_cast<int>(animDef->frames.size())) {
        return nullptr;
    }

    return &animDef->frames[currentFrame];
}

} // namespace core
