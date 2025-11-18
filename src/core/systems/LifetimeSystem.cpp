#include "core/systems/LifetimeSystem.h"
#include "core/Components.h"
#include "core/Logger.h"

namespace core {

void LifetimeSystem::update(entt::registry& registry, double dt) {
    // Получаем все сущности с LifetimeComponent
    auto view = registry.view<LifetimeComponent>();

    // Собираем сущности для удаления (нельзя удалять во время итерации)
    std::vector<entt::entity> entitiesToDestroy;

    for (auto entity : view) {
        auto& lifetime = view.get<LifetimeComponent>(entity);

        // Уменьшаем время жизни
        lifetime.lifetime -= static_cast<float>(dt);

        // Проверяем истекло ли время
        if (lifetime.lifetime <= 0.0f && lifetime.autoDestroy) {
            entitiesToDestroy.push_back(entity);
        }
    }

    // Уничтожаем сущности
    for (auto entity : entitiesToDestroy) {
        // Логируем для отладки
        if (registry.all_of<NameComponent>(entity)) {
            const auto& name = registry.get<NameComponent>(entity);
            LOG_DEBUG("Destroying entity '{}' (lifetime expired)", name.name);
        } else {
            LOG_DEBUG("Destroying entity {} (lifetime expired)", static_cast<int>(entity));
        }

        registry.destroy(entity);
    }
}

void LifetimeSystem::processEntity(entt::registry& registry, entt::entity entity, double dt) {
    // Эта функция может быть расширена для более сложной логики
    // Например, события перед уничтожением, fade-out эффекты и т.д.
}

} // namespace core
