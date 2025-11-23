#pragma once

#include <entt/entt.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <unordered_map>

namespace core {

// Forward declaration
class ResourceManager;

/**
 * @brief Система рендеринга ECS сущностей
 *
 * Отрисовывает все сущности с компонентами Transform и Sprite.
 * Поддерживает слои отрисовки и сортировку.
 * Кеширует sf::Sprite объекты для оптимизации производительности.
 */
class RenderSystem {
public:
    /**
     * @brief Конструктор
     * @param resourceManager Указатель на менеджер ресурсов
     */
    explicit RenderSystem(ResourceManager* resourceManager);

    /**
     * @brief Отрисовка всех сущностей
     * @param registry EnTT registry с сущностями
     * @param window Окно для отрисовки
     */
    void render(entt::registry& registry, sf::RenderWindow& window);

    /**
     * @brief Очищает кеш спрайтов для конкретной сущности
     * @param entity Сущность для которой нужно очистить кеш
     */
    void invalidateCache(entt::entity entity);

    /**
     * @brief Очищает весь кеш спрайтов
     */
    void clearCache();

private:
    ResourceManager* m_resourceManager;  ///< Менеджер ресурсов для текстур
    bool m_layersDirty = true;           ///< Флаг необходимости пересортировки слоев

    /**
     * @brief Кеш sf::Sprite объектов для избежания создания каждый кадр
     * Ключ - entity ID, значение - кешированный спрайт
     */
    std::unordered_map<entt::entity, sf::Sprite> m_spriteCache;
};

} // namespace core
