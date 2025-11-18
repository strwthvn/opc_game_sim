#include "core/systems/LifetimeSystem.h"
#include "core/Components.h"
#include "core/Logger.h"
#include <algorithm>
#include <cstdint>

namespace core {

void LifetimeSystem::update(entt::registry& registry, double dt) {
    // Получаем все сущности с LifetimeComponent
    auto view = registry.view<LifetimeComponent>();

    // Собираем сущности для удаления (нельзя удалять во время итерации)
    std::vector<entt::entity> entitiesToDestroy;

    for (auto entity : view) {
        // Обрабатываем сущность (fade-out эффекты и т.д.)
        processEntity(registry, entity, dt);

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
    auto& lifetime = registry.get<LifetimeComponent>(entity);

    // Применяем fade-out эффект если включен и есть SpriteComponent
    if (lifetime.fadeOut && registry.all_of<SpriteComponent>(entity)) {
        auto& sprite = registry.get<SpriteComponent>(entity);

        // Вычисляем порог начала затухания
        float fadeThreshold = lifetime.initialLifetime * lifetime.fadeStartRatio;

        if (lifetime.lifetime <= fadeThreshold && fadeThreshold > 0.0f) {
            // Вычисляем прогресс затухания (1.0 -> 0.0)
            float fadeProgress = lifetime.lifetime / fadeThreshold;
            fadeProgress = std::clamp(fadeProgress, 0.0f, 1.0f);

            // Применяем к альфа-каналу цвета
            auto alpha = static_cast<std::uint8_t>(255.0f * fadeProgress);
            sprite.color = sf::Color(
                sprite.color.r,
                sprite.color.g,
                sprite.color.b,
                alpha
            );
        }
    }

    // Здесь можно добавить дополнительную логику:
    // - События перед уничтожением (onAboutToDestroy callback)
    // - Звуковые эффекты при уничтожении
    // - Spawn эффектов частиц
}

} // namespace core
