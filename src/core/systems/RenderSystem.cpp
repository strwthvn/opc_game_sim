#include "core/systems/RenderSystem.h"
#include "core/Components.h"
#include "core/ResourceManager.h"
#include "core/Logger.h"
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Transformable.hpp>
#include <algorithm>
#include <vector>

namespace core {

RenderSystem::RenderSystem(ResourceManager* resourceManager)
    : m_resourceManager(resourceManager) {
    if (!m_resourceManager) {
        LOG_ERROR("RenderSystem created with null ResourceManager");
        throw std::runtime_error("RenderSystem requires valid ResourceManager");
    }
    LOG_DEBUG("RenderSystem initialized");
}

void RenderSystem::render(entt::registry& registry, sf::RenderWindow& window) {
    // Получаем view всех сущностей с Transform и Sprite
    auto view = registry.view<TransformComponent, SpriteComponent>();

    // Создаем вектор для сортировки по слоям
    struct RenderData {
        entt::entity entity;
        const TransformComponent* transform;
        const SpriteComponent* sprite;
        int layer;
    };

    std::vector<RenderData> renderQueue;
    renderQueue.reserve(view.size_hint());

    // Собираем все видимые спрайты
    for (auto entity : view) {
        const auto& transform = view.get<TransformComponent>(entity);
        const auto& sprite = view.get<SpriteComponent>(entity);

        // Пропускаем невидимые спрайты
        if (!sprite.visible) {
            continue;
        }

        renderQueue.push_back({entity, &transform, &sprite, sprite.layer});
    }

    // Сортируем по слоям (меньше = раньше)
    std::sort(renderQueue.begin(), renderQueue.end(),
              [](const RenderData& a, const RenderData& b) {
                  return a.layer < b.layer;
              });

    // Отрисовываем все спрайты
    for (const auto& data : renderQueue) {
        const auto& transform = *data.transform;
        const auto& spriteComp = *data.sprite;

        // Пропускаем спрайты без текстуры
        if (spriteComp.textureName.empty()) {
            continue;
        }

        try {
            // Получаем текстуру из ResourceManager
            const sf::Texture& texture = m_resourceManager->getTexture(spriteComp.textureName);

            // Используем кешированный спрайт или создаем новый
            if (spriteComp.dirty || !spriteComp.cachedSprite.has_value()) {
                // Создаем или пересоздаем спрайт
                spriteComp.cachedSprite.emplace(texture);
                sf::Sprite& sprite = *spriteComp.cachedSprite;

                // Устанавливаем прямоугольник текстуры если указан
                if (spriteComp.textureRect.size.x > 0 && spriteComp.textureRect.size.y > 0) {
                    sprite.setTextureRect(spriteComp.textureRect);
                }

                // Устанавливаем цвет модуляции
                sprite.setColor(spriteComp.color);

                // Устанавливаем origin в центр спрайта для корректного вращения
                sf::FloatRect bounds = sprite.getLocalBounds();
                sprite.setOrigin(bounds.size / 2.0f);

                spriteComp.dirty = false;
            }

            sf::Sprite& sprite = *spriteComp.cachedSprite;

            // Применяем трансформацию каждый кадр (SFML 3 uses Vector2f and sf::Angle)
            sprite.setPosition(sf::Vector2f(transform.x, transform.y));
            sprite.setRotation(sf::degrees(transform.rotation));
            sprite.setScale(sf::Vector2f(transform.scaleX, transform.scaleY));

            // Отрисовываем
            window.draw(sprite);

        } catch (const std::exception& e) {
            // Логируем ошибку, но продолжаем рендеринг других сущностей
            LOG_ERROR("Failed to render entity: {}", e.what());
        }
    }
}

} // namespace core
