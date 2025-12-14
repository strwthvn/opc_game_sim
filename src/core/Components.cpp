#include "core/Components.h"
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

} // namespace core
