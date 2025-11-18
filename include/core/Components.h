#pragma once

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <entt/entt.hpp>
#include <string>
#include <vector>
#include <algorithm>

namespace core {

/**
 * @brief Компонент трансформации
 *
 * Хранит позицию, вращение и масштаб сущности
 */
struct TransformComponent {
    float x = 0.0f;              ///< Позиция X в пикселях
    float y = 0.0f;              ///< Позиция Y в пикселях
    float rotation = 0.0f;       ///< Угол поворота в градусах
    float scaleX = 1.0f;         ///< Масштаб по X
    float scaleY = 1.0f;         ///< Масштаб по Y
};

/**
 * @brief Компонент спрайта
 *
 * Хранит информацию о текстуре и отрисовке
 */
struct SpriteComponent {
    std::string textureName;     ///< Имя текстуры в ResourceManager
    sf::IntRect textureRect;     ///< Прямоугольник текстуры (для спрайт-атласов)
    sf::Color color;             ///< Цвет модуляции (белый = без изменений)
    int layer = 0;               ///< Слой отрисовки (меньше = раньше)
    bool visible = true;         ///< Видимость спрайта

    /**
     * @brief Конструктор по умолчанию
     */
    SpriteComponent()
        : textureName("")
        , textureRect()  // Default constructor
        , color(sf::Color::White)
        , layer(0)
        , visible(true) {
    }

    /**
     * @brief Конструктор с текстурой
     * @param name Имя текстуры
     */
    explicit SpriteComponent(const std::string& name)
        : textureName(name)
        , textureRect()  // Default constructor
        , color(sf::Color::White)
        , layer(0)
        , visible(true) {
    }
};

/**
 * @brief Компонент скорости
 *
 * Хранит линейную и угловую скорость для простого движения
 */
struct VelocityComponent {
    float vx = 0.0f;             ///< Скорость по X (пикселей/секунду)
    float vy = 0.0f;             ///< Скорость по Y (пикселей/секунду)
    float angularVelocity = 0.0f; ///< Угловая скорость (градусов/секунду)
};

/**
 * @brief Тег компонент для обозначения камеры
 *
 * Сущность с этим компонентом будет использоваться как камера
 */
struct CameraComponent {
    float zoom = 1.0f;           ///< Масштаб камеры (1.0 = нормальный)
    bool active = true;          ///< Активна ли камера
};

/**
 * @brief Компонент имени сущности
 *
 * Используется для отладки, инспектора и поиска сущностей
 */
struct NameComponent {
    std::string name;            ///< Читаемое имя сущности

    NameComponent() : name("Unnamed Entity") {}
    explicit NameComponent(const std::string& n) : name(n) {}
};

/**
 * @brief Компонент тега для категоризации
 *
 * Позволяет группировать и фильтровать сущности
 */
struct TagComponent {
    std::string tag;             ///< Тег категории (например: "player", "enemy", "obstacle")

    TagComponent() : tag("") {}
    explicit TagComponent(const std::string& t) : tag(t) {}
};

/**
 * @brief Компонент времени жизни
 *
 * Автоматически уничтожает сущность после истечения времени
 * Полезно для временных эффектов, частиц
 */
struct LifetimeComponent {
    float lifetime = 1.0f;       ///< Оставшееся время жизни (секунды)
    bool autoDestroy = true;     ///< Автоматически уничтожать по истечению

    explicit LifetimeComponent(float time = 1.0f, bool destroy = true)
        : lifetime(time), autoDestroy(destroy) {}
};

/**
 * @brief Компонент анимации спрайта
 *
 * Управляет покадровой анимацией через смену textureRect
 */
struct AnimationComponent {
    std::string currentAnimation;     ///< Имя текущей анимации
    int currentFrame = 0;             ///< Текущий кадр
    float frameTime = 0.0f;           ///< Время до следующего кадра (секунды)
    float frameDelay = 0.1f;          ///< Задержка между кадрами (секунды)
    bool loop = true;                 ///< Зациклить анимацию
    bool playing = true;              ///< Воспроизводится ли анимация

    // Данные анимации будут храниться в отдельном AnimationSystem/AnimationManager
};

/**
 * @brief Компонент для создания иерархии (родитель)
 *
 * Позволяет создавать родительско-дочерние отношения между сущностями
 */
struct ParentComponent {
    entt::entity parent = entt::null;  ///< Родительская сущность

    ParentComponent() = default;
    explicit ParentComponent(entt::entity p) : parent(p) {}
};

/**
 * @brief Компонент для хранения дочерних сущностей
 *
 * Автоматически обновляется системами при добавлении ParentComponent
 */
struct ChildrenComponent {
    std::vector<entt::entity> children;  ///< Список дочерних сущностей

    void addChild(entt::entity child) {
        children.push_back(child);
    }

    void removeChild(entt::entity child) {
        children.erase(
            std::remove(children.begin(), children.end(), child),
            children.end()
        );
    }
};

} // namespace core
