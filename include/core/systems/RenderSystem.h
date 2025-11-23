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
 * @note RenderSystem требует вызова setRenderTarget() перед использованием
 */
class RenderSystem : public ISystem {
public:
    /**
     * @brief Конструктор
     * @param resourceManager Указатель на менеджер ресурсов
     */
    explicit RenderSystem(ResourceManager* resourceManager);

    /**
     * @brief Обновление системы (вызывает render)
     *
     * Реализация интерфейса ISystem. Вызывает render() с текущим render target.
     *
     * @param registry EnTT registry с сущностями
     * @param dt Время с последнего обновления (не используется)
     * @warning Требует предварительного вызова setRenderTarget()
     */
    void update(entt::registry& registry, double dt) override;

    /**
     * @brief Устанавливает целевое окно для рендеринга
     * @param window Окно для отрисовки
     * @note Должен быть вызван перед update()
     */
    void setRenderTarget(sf::RenderWindow* window);

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

    /**
     * @brief Помечает слои как измененные (требуется пересортировка)
     */
    void markLayersDirty();

    int getPriority() const override { return 500; }
    const char* getName() const override { return "RenderSystem"; }

private:
    ResourceManager* m_resourceManager;  ///< Менеджер ресурсов для текстур
    sf::RenderWindow* m_renderTarget = nullptr;  ///< Целевое окно для рендеринга
    bool m_layersDirty = true;           ///< Флаг необходимости пересортировки слоев

    /**
     * @brief Кеш sf::Sprite объектов для избежания создания каждый кадр
     * Ключ - entity ID, значение - кешированный спрайт
     */
    std::unordered_map<entt::entity, sf::Sprite> m_spriteCache;
};

} // namespace core
