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
    : m_resourceManager(resourceManager), m_renderTarget(nullptr) {
    if (!m_resourceManager) {
        LOG_ERROR("RenderSystem created with null ResourceManager");
        throw std::runtime_error("RenderSystem requires valid ResourceManager");
    }
    LOG_DEBUG("RenderSystem initialized");
}

void RenderSystem::update(entt::registry& registry, double dt) {
    if (!m_renderTarget) {
        LOG_ERROR("RenderSystem::update() called without render target. Call setRenderTarget() first.");
        return;
    }
    render(registry, *m_renderTarget);
}

void RenderSystem::setRenderTarget(sf::RenderWindow* window) {
    m_renderTarget = window;
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

    // Получаем bounds камеры для frustum culling
    sf::FloatRect viewBounds = sf::FloatRect(window.getView().getCenter() - window.getView().getSize() / 2.0f,
                                              window.getView().getSize());

    // Собираем все видимые спрайты
    for (auto entity : view) {
        const auto& transform = view.get<TransformComponent>(entity);
        const auto& sprite = view.get<SpriteComponent>(entity);

        // Пропускаем невидимые спрайты
        if (!sprite.visible) {
            continue;
        }

        // Пропускаем спрайты без текстуры
        if (sprite.textureName.empty()) {
            continue;
        }

        // Frustum culling: вычисляем bounds с учетом bottom-left origin
        try {
            const sf::Texture& texture = m_resourceManager->getTexture(sprite.textureName);

            // Получаем размер спрайта (либо из textureRect, либо из размера текстуры)
            sf::Vector2f spriteSize;
            if (sprite.textureRect.size.x > 0 && sprite.textureRect.size.y > 0) {
                sf::Vector2f baseSize = sf::Vector2f(sprite.textureRect.size);
                spriteSize = sf::Vector2f(baseSize.x * transform.scaleX, baseSize.y * transform.scaleY);
            } else {
                sf::Vector2f baseSize = sf::Vector2f(texture.getSize());
                spriteSize = sf::Vector2f(baseSize.x * transform.scaleX, baseSize.y * transform.scaleY);
            }

            // Вычисляем bounds с учетом bottom-left origin
            // Transform.x, Transform.y - это позиция НИЖНЕГО ЛЕВОГО угла спрайта
            sf::FloatRect spriteBounds(
                sf::Vector2f(transform.x, transform.y - spriteSize.y),  // top-left
                spriteSize
            );

            // Пропускаем объекты вне камеры
            if (!viewBounds.findIntersection(spriteBounds).has_value()) {
                continue;
            }
        } catch (const std::exception& e) {
            // Если текстура не найдена, пропускаем culling для этого спрайта
            LOG_WARN("Frustum culling skipped for entity - texture not found: {}", sprite.textureName);
        }

        renderQueue.push_back({entity, &transform, &sprite, sprite.layer});
    }

    // Сортируем по слоям только если они изменились
    if (m_layersDirty) {
        std::stable_sort(renderQueue.begin(), renderQueue.end(),
                         [](const RenderData& a, const RenderData& b) {
                             return a.layer < b.layer;
                         });
        m_layersDirty = false;
    }

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

            // Получаем или создаем спрайт из кеша системы
            auto it = m_spriteCache.find(data.entity);
            bool isNewSprite = (it == m_spriteCache.end());

            if (isNewSprite) {
                // Создаем новый спрайт и добавляем в кеш
                auto [inserted_it, success] = m_spriteCache.emplace(data.entity, sf::Sprite(texture));
                it = inserted_it;
            }

            sf::Sprite& sprite = it->second;

            // Обновляем текстуру (на случай если изменилась)
            sprite.setTexture(texture);

            // Устанавливаем прямоугольник текстуры
            // Если не указан явно, используем размер всей текстуры
            if (spriteComp.textureRect.size.x > 0 && spriteComp.textureRect.size.y > 0) {
                sprite.setTextureRect(spriteComp.textureRect);
            } else {
                // Для спрайтов без явного textureRect используем всю текстуру
                sprite.setTextureRect(sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(texture.getSize())));
            }

            // Устанавливаем цвет модуляции
            sprite.setColor(spriteComp.color);

            // Устанавливаем origin ТОЛЬКО при создании нового спрайта
            if (isNewSprite) {
                sf::FloatRect bounds = sprite.getLocalBounds();
                // Для вращения используем центр, иначе левый НИЖНИЙ угол
                if (transform.rotation != 0.0f) {
                    // Origin в центре для корректного вращения
                    sprite.setOrigin(bounds.size / 2.0f);
                } else {
                    // Origin в левом НИЖНЕМ углу для интуитивного позиционирования
                    // (объекты "стоят" на своей Y-координате)
                    sprite.setOrigin(sf::Vector2f(0.0f, bounds.size.y));
                }
            }

            // Применяем трансформацию (SFML 3 uses Vector2f and sf::Angle)
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

void RenderSystem::invalidateCache(entt::entity entity) {
    m_spriteCache.erase(entity);
}

void RenderSystem::clearCache() {
    m_spriteCache.clear();
}

void RenderSystem::markLayersDirty() {
    m_layersDirty = true;
}

} // namespace core
