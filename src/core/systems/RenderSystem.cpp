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

void RenderSystem::setViewBounds(const sf::FloatRect& viewBounds) {
    m_viewBounds = viewBounds;
}

void RenderSystem::update(entt::registry& registry, double dt) {
    // Очищаем очередь рендеринга от предыдущего кадра
    m_renderQueue.clear();

    // Получаем view всех сущностей с Transform и Sprite
    auto view = registry.view<TransformComponent, SpriteComponent>();

    // Резервируем место для оптимизации
    m_renderQueue.reserve(view.size_hint());

    // Собираем все видимые спрайты и выполняем frustum culling
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
            if (!m_viewBounds.findIntersection(spriteBounds).has_value()) {
                continue;
            }
        } catch (const std::exception& e) {
            // Если текстура не найдена, пропускаем culling для этого спрайта
            LOG_WARN("Frustum culling skipped for entity - texture not found: {}", sprite.textureName);
        }

        // Добавляем в очередь рендеринга
        m_renderQueue.push_back({entity, sprite.layer});

        // Подготавливаем или обновляем спрайт в кеше
        try {
            const sf::Texture& texture = m_resourceManager->getTexture(sprite.textureName);

            // Получаем или создаем спрайт из кеша
            auto it = m_spriteCache.find(entity);
            bool isNewSprite = (it == m_spriteCache.end());

            if (isNewSprite) {
                // Создаем новый спрайт и добавляем в кеш
                auto [inserted_it, success] = m_spriteCache.emplace(entity, sf::Sprite(texture));
                it = inserted_it;
            }

            sf::Sprite& cachedSprite = it->second;

            // Обновляем текстуру (на случай если изменилась)
            cachedSprite.setTexture(texture);

            // Устанавливаем прямоугольник текстуры
            if (sprite.textureRect.size.x > 0 && sprite.textureRect.size.y > 0) {
                cachedSprite.setTextureRect(sprite.textureRect);
            } else {
                cachedSprite.setTextureRect(sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(texture.getSize())));
            }

            // Устанавливаем цвет модуляции
            cachedSprite.setColor(sprite.color);

            // Устанавливаем origin ТОЛЬКО при создании нового спрайта
            if (isNewSprite) {
                sf::FloatRect bounds = cachedSprite.getLocalBounds();
                // Для вращения используем центр, иначе левый НИЖНИЙ угол
                if (transform.rotation != 0.0f) {
                    // Origin в центре для корректного вращения
                    cachedSprite.setOrigin(bounds.size / 2.0f);
                } else {
                    // Origin в левом НИЖНЕМ углу для интуитивного позиционирования
                    cachedSprite.setOrigin(sf::Vector2f(0.0f, bounds.size.y));
                }
            }

            // Применяем трансформацию (SFML 3 uses Vector2f and sf::Angle)
            cachedSprite.setPosition(sf::Vector2f(transform.x, transform.y));
            cachedSprite.setRotation(sf::degrees(transform.rotation));
            cachedSprite.setScale(sf::Vector2f(transform.scaleX, transform.scaleY));

        } catch (const std::exception& e) {
            LOG_ERROR("Failed to prepare sprite for entity: {}", e.what());
        }
    }

    // Сортируем по слоям только если они изменились
    if (m_layersDirty) {
        std::stable_sort(m_renderQueue.begin(), m_renderQueue.end(),
                         [](const RenderData& a, const RenderData& b) {
                             return a.layer < b.layer;
                         });
        m_layersDirty = false;
    }
}

void RenderSystem::render(sf::RenderWindow& window) {
    // Отрисовываем все подготовленные спрайты из очереди
    for (const auto& data : m_renderQueue) {
        // Получаем спрайт из кеша
        auto it = m_spriteCache.find(data.entity);
        if (it == m_spriteCache.end()) {
            // Спрайт должен был быть подготовлен в update()
            LOG_WARN("Sprite not found in cache for entity during render - was update() called?");
            continue;
        }

        // Отрисовываем спрайт (все параметры уже установлены в update())
        window.draw(it->second);
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
