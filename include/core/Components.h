#pragma once

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <entt/entt.hpp>
#include <string>
#include <vector>
#include <unordered_set>
#include <algorithm>
#include <optional>

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
 * Хранит информацию о текстуре и отрисовке.
 * Включает кеширование sf::Sprite для оптимизации hot path.
 */
struct SpriteComponent {
    std::string textureName;     ///< Имя текстуры в ResourceManager
    sf::IntRect textureRect;     ///< Прямоугольник текстуры (для спрайт-атласов)
    sf::Color color;             ///< Цвет модуляции (белый = без изменений)
    int layer = 0;               ///< Слой отрисовки (меньше = раньше)
    bool visible = true;         ///< Видимость спрайта

    // Кеш для оптимизации рендеринга (избегаем создания sf::Sprite каждый кадр)
    mutable std::optional<sf::Sprite> cachedSprite;  ///< Кешированный SFML спрайт
    mutable bool dirty = true;   ///< Флаг необходимости обновления кеша

    /**
     * @brief Помечает кеш как требующий обновления
     */
    void markDirty() const {
        dirty = true;
    }

    /**
     * @brief Конструктор по умолчанию
     */
    SpriteComponent()
        : textureName("")
        , textureRect()  // Default constructor
        , color(sf::Color::White)
        , layer(0)
        , visible(true)
        , cachedSprite(std::nullopt)
        , dirty(true) {
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
        , visible(true)
        , cachedSprite(std::nullopt)
        , dirty(true) {
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
    float initialLifetime = 1.0f; ///< Начальное время жизни (для расчёта прогресса)
    bool autoDestroy = true;     ///< Автоматически уничтожать по истечению
    bool fadeOut = false;        ///< Плавное затухание перед уничтожением
    float fadeStartRatio = 0.3f; ///< Когда начинать fade (0.3 = последние 30% времени)

    explicit LifetimeComponent(float time = 1.0f, bool destroy = true, bool fade = false)
        : lifetime(time)
        , initialLifetime(time)
        , autoDestroy(destroy)
        , fadeOut(fade)
        , fadeStartRatio(0.3f) {}
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
 * Автоматически обновляется системами при добавлении ParentComponent.
 * Использует unordered_set для O(1) операций добавления/удаления/проверки.
 */
struct ChildrenComponent {
    std::unordered_set<entt::entity> children;  ///< Множество дочерних сущностей

    void addChild(entt::entity child) {
        children.insert(child);
    }

    void removeChild(entt::entity child) {
        children.erase(child);
    }

    bool hasChild(entt::entity child) const {
        return children.contains(child);
    }

    size_t childCount() const {
        return children.size();
    }
};

} // namespace core
