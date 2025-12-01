#pragma once

#include "core/systems/ISystem.h"
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
 *
 * Использует двухэтапный рендеринг:
 * 1. update() - подготовка данных (culling, сортировка, кеширование)
 * 2. render() - отрисовка подготовленных спрайтов
 *
 * @note Требует вызова setViewBounds() перед update() для frustum culling
 */
class RenderSystem : public ISystem {
public:
    /**
     * @brief Конструктор
     * @param resourceManager Указатель на менеджер ресурсов
     */
    explicit RenderSystem(ResourceManager* resourceManager);

    /**
     * @brief Обновление системы (подготовка данных для рендеринга)
     *
     * Реализация интерфейса ISystem. Подготавливает данные для рендеринга:
     * - Собирает видимые сущности
     * - Выполняет frustum culling
     * - Сортирует по слоям
     * - Обновляет кеш спрайтов и их параметры
     *
     * @param registry EnTT registry с сущностями
     * @param dt Время с последнего обновления (не используется)
     * @warning Требует предварительного вызова setViewBounds()
     */
    void update(entt::registry& registry, double dt) override;

    /**
     * @brief Устанавливает границы видимой области для frustum culling
     * @param viewBounds Прямоугольник видимой области в world coordinates
     * @note Должен быть вызван перед update()
     */
    void setViewBounds(const sf::FloatRect& viewBounds);

    /**
     * @brief Отрисовка подготовленных сущностей
     * @param window Окно для отрисовки
     * @note Требует предварительного вызова update() для подготовки данных
     */
    void render(sf::RenderWindow& window);

    /**
     * @brief Очищает кеш спрайтов для конкретной сущности
     * @param entity Сущность для которой нужно очистить кеш
     */
    void invalidateCache(entt::entity entity);

    /**
     * @brief Очищает весь кеш спрайтов
     */
    void clearCache();

    /**
     * @brief Помечает слои как измененные (требуется пересортировка)
     */
    void markLayersDirty();

    int getPriority() const override { return 500; }
    const char* getName() const override { return "RenderSystem"; }

private:
    /**
     * @brief Данные для рендеринга одной сущности
     */
    struct RenderData {
        entt::entity entity;
        int layer;
    };

    ResourceManager* m_resourceManager;  ///< Менеджер ресурсов для текстур
    sf::FloatRect m_viewBounds;          ///< Границы видимой области для frustum culling
    bool m_layersDirty = true;           ///< Флаг необходимости пересортировки слоев

    /**
     * @brief Кеш sf::Sprite объектов для избежания создания каждый кадр
     * Ключ - entity ID, значение - кешированный спрайт
     */
    std::unordered_map<entt::entity, sf::Sprite> m_spriteCache;

    /**
     * @brief Очередь подготовленных сущностей для рендеринга
     * Заполняется в update(), используется в render()
     */
    std::vector<RenderData> m_renderQueue;
};

} // namespace core
