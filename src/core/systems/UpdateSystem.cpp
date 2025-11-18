#include "core/systems/UpdateSystem.h"
#include "core/Components.h"
#include "core/Logger.h"
#include <cmath>

namespace core {

UpdateSystem::UpdateSystem() {
    LOG_DEBUG("UpdateSystem initialized");
}

void UpdateSystem::update(entt::registry& registry, double dt) {
    // Обновляем движение
    updateMovement(registry, dt);

    // TODO: Добавить другие системы обновления
    // - Система анимации
    // - Система жизненного цикла
    // - Система поведения (AI)
}

void UpdateSystem::updateMovement(entt::registry& registry, double dt) {
    // Получаем view всех сущностей с Transform и Velocity
    auto view = registry.view<TransformComponent, VelocityComponent>();

    for (auto entity : view) {
        auto& transform = view.get<TransformComponent>(entity);
        const auto& velocity = view.get<VelocityComponent>(entity);

        // Обновляем позицию на основе скорости
        transform.x += velocity.vx * static_cast<float>(dt);
        transform.y += velocity.vy * static_cast<float>(dt);

        // Обновляем вращение
        transform.rotation += velocity.angularVelocity * static_cast<float>(dt);

        // Нормализуем угол вращения (0-360) используя fmod для O(1) сложности
        transform.rotation = std::fmod(transform.rotation, 360.0f);
        if (transform.rotation < 0.0f) {
            transform.rotation += 360.0f;
        }
    }
}

} // namespace core
