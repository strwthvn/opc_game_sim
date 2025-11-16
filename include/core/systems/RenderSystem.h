#pragma once

#include <entt/entt.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

namespace core {

// Forward declaration
class ResourceManager;

/**
 * @brief Система рендеринга ECS сущностей
 *
 * Отрисовывает все сущности с компонентами Transform и Sprite
 * Поддерживает слои отрисовки и сортировку
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

private:
    ResourceManager* m_resourceManager;  ///< Менеджер ресурсов для текстур
};

} // namespace core
