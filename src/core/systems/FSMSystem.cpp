#include "core/systems/FSMSystem.h"
#include "core/Components.h"
#include "core/Logger.h"

namespace core {

FSMSystem::FSMSystem() {
    LOG_DEBUG("FSMSystem initialized");
}

FSMSystem::~FSMSystem() = default;

void FSMSystem::update(entt::registry& registry, double dt) {
    // Получаем view всех сущностей с EntityStateComponent
    auto view = registry.view<EntityStateComponent>();

    // Обновляем время в состоянии для всех сущностей
    for (auto entity : view) {
        auto& stateComponent = view.get<EntityStateComponent>(entity);

        // Увеличиваем время в текущем состоянии
        stateComponent.timeInState += static_cast<float>(dt);
    }
}

} // namespace core
