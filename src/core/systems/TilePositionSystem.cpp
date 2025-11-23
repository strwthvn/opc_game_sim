#include "core/systems/TilePositionSystem.h"
#include "core/Components.h"
#include "core/Logger.h"
#include <algorithm>  // для std::clamp

namespace core {

TilePositionSystem::TilePositionSystem() {
    LOG_DEBUG("TilePositionSystem initialized");
}

void TilePositionSystem::update(entt::registry& registry) {
    // Синхронизируем тайловые координаты с пиксельными
    syncPositions(registry);

    // Вычисляем layer для Y-sorting
    updateLayers(registry);
}

void TilePositionSystem::syncPositions(entt::registry& registry) {
    // Получаем view всех сущностей с TilePosition и Transform
    auto view = registry.view<TilePositionComponent, TransformComponent>();

    for (auto entity : view) {
        auto& tilePos = view.get<TilePositionComponent>(entity);
        auto& transform = view.get<TransformComponent>(entity);

        // Конвертируем тайловые координаты в пиксельные
        // Используем левый НИЖНИЙ угол объекта как anchor point
        // (объекты "стоят" на нижней границе своего тайла)
        sf::Vector2f pixelPos = tilePos.getPixelPosition();
        transform.x = pixelPos.x;
        transform.y = pixelPos.y;
    }
}

void TilePositionSystem::updateLayers(entt::registry& registry) {
    // Получаем view сущностей с TilePosition и Sprite
    auto view = registry.view<TilePositionComponent, SpriteComponent>();

    for (auto entity : view) {
        auto& tilePos = view.get<TilePositionComponent>(entity);
        auto& sprite = view.get<SpriteComponent>(entity);

        // Если объект на слое Objects, применяем Y-sorting
        // Объекты с большим Y рисуются позже (находятся "ближе" к камере)
        if (sprite.layer >= RenderLayer::Objects && sprite.layer < RenderLayer::Overlays) {
            // Базовый слой Objects (200) + позиция по Y для Y-sorting
            // Ограничиваем yOffset в диапазоне [0, 99] чтобы не переполнить слой Overlays (300)
            int yOffset = std::clamp(tilePos.tileY, 0, 99);
            sprite.layer = RenderLayer::Objects + yOffset;

            // Помечаем кеш спрайта как требующий обновления
            sprite.markDirty();
        }
        // Для Overlays синхронизируем с тем же Y, что у родителя
        else if (sprite.layer >= RenderLayer::Overlays && sprite.layer < RenderLayer::UIOverlay) {
            // Ограничиваем yOffset в диапазоне [0, 99] чтобы не переполнить слой UIOverlay (400)
            int yOffset = std::clamp(tilePos.tileY, 0, 99);
            sprite.layer = RenderLayer::Overlays + yOffset;
            sprite.markDirty();
        }
    }
}

} // namespace core
