#include "core/systems/OverlaySystem.h"
#include "core/Components.h"
#include "core/Logger.h"

namespace core {

OverlaySystem::OverlaySystem() {
    LOG_DEBUG("OverlaySystem initialized");
}

void OverlaySystem::update(entt::registry& registry) {
    // Синхронизируем позиции оверлеев с родителями
    syncOverlayPositions(registry);
}

void OverlaySystem::syncOverlayPositions(entt::registry& registry) {
    // Получаем view всех оверлеев (сущности с OverlayComponent, ParentComponent и TransformComponent)
    auto view = registry.view<OverlayComponent, ParentComponent, TransformComponent>();

    for (auto entity : view) {
        auto& overlay = view.get<OverlayComponent>(entity);
        auto& parent = view.get<ParentComponent>(entity);
        auto& transform = view.get<TransformComponent>(entity);

        // Пропускаем если синхронизация отключена
        if (!overlay.syncWithParent) {
            continue;
        }

        // Проверяем, что родитель существует и валиден
        if (parent.parent == entt::null || !registry.valid(parent.parent)) {
            LOG_WARN("Overlay entity has invalid parent reference");
            continue;
        }

        // Получаем TransformComponent родителя
        auto* parentTransform = registry.try_get<TransformComponent>(parent.parent);
        if (!parentTransform) {
            LOG_WARN("Parent entity does not have TransformComponent");
            continue;
        }

        // Копируем позицию родителя и добавляем локальное смещение
        transform.x = parentTransform->x + overlay.localOffset.x;
        transform.y = parentTransform->y + overlay.localOffset.y;

        // Отладочное логирование (только для первых 5 обновлений)
        static int logCount = 0;
        if (logCount < 5) {
            LOG_DEBUG("Overlay sync: parent=({}, {}), offset=({}, {}), result=({}, {})",
                     parentTransform->x, parentTransform->y,
                     overlay.localOffset.x, overlay.localOffset.y,
                     transform.x, transform.y);
            logCount++;
        }

        // Наследуем вращение родителя (опционально)
        // transform.rotation = parentTransform->rotation;

        // Если у оверлея есть спрайт, помечаем его как требующий обновления
        if (auto* sprite = registry.try_get<SpriteComponent>(entity)) {
            sprite->markDirty();
        }
    }
}

} // namespace core
