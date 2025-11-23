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

// ============== КОНСТАНТЫ ТАЙЛОВОЙ СИСТЕМЫ ==============

/**
 * @brief Размер одного тайла в пикселях (фиксировано)
 */
constexpr int TILE_SIZE = 32;

/**
 * @brief Слои отрисовки для корректного z-ordering
 *
 * Использует enum class для строгой типизации и возможности
 * дальнейшего расширения через конфигурационные файлы.
 */
enum class RenderLayer : int {
    Background = 0,    ///< Фон (под картой)
    Ground = 100,      ///< Базовая карта (пол, земля)
    Objects = 200,     ///< Промышленные объекты (+ tileY для Y-sorting)
    Overlays = 300,    ///< Индикаторы, кнопки состояния (+ tileY для синхронизации)
    UIOverlay = 400    ///< HUD, курсор, UI элементы в игровом мире
};

/**
 * @brief Преобразование RenderLayer в int для арифметических операций
 * @param layer Слой отрисовки
 * @return Целочисленное значение слоя
 */
inline constexpr int toInt(RenderLayer layer) {
    return static_cast<int>(layer);
}

// ============== БАЗОВЫЕ КОМПОНЕНТЫ ==============

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
 * Компонент содержит только чистые данные (без кеша системы).
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
        , textureRect()
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
        , textureRect()
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
 * Управляет покадровой анимацией через смену textureRect.
 * Поддерживает спрайт-листы (горизонтальные полосы кадров).
 */
struct AnimationComponent {
    std::string currentAnimation;     ///< Имя текущей анимации
    int currentFrame = 0;             ///< Текущий кадр
    float frameTime = 0.0f;           ///< Накопитель времени до следующего кадра (секунды)
    float frameDelay = 0.1f;          ///< Задержка между кадрами (секунды)
    bool loop = true;                 ///< Зациклить анимацию
    bool playing = true;              ///< Воспроизводится ли анимация

    // === НОВЫЕ ПОЛЯ ДЛЯ ТАЙЛОВОЙ СИСТЕМЫ ===
    int frameCount = 1;               ///< Общее количество кадров в анимации
    int frameWidth = TILE_SIZE;       ///< Ширина одного кадра в пикселях
    int frameHeight = TILE_SIZE;      ///< Высота одного кадра в пикселях

    /**
     * @brief Получить прямоугольник текущего кадра для textureRect
     * @return Прямоугольник текущего кадра (для горизонтального спрайт-листа)
     */
    sf::IntRect getCurrentFrameRect() const {
        return sf::IntRect(
            sf::Vector2i(currentFrame * frameWidth, 0),
            sf::Vector2i(frameWidth, frameHeight)
        );
    }

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

// ============== КОМПОНЕНТЫ ТАЙЛОВОЙ СИСТЕМЫ ==============

/**
 * @brief Позиция объекта в тайловых координатах
 *
 * ИСТОЧНИК ПРАВДЫ для позиционирования объектов на тайловой сетке.
 * TransformComponent автоматически синхронизируется с TilePositionComponent,
 * когда autoSync = true (по умолчанию).
 *
 * Тайловые координаты первичны для статичных объектов (здания, оборудование).
 * Для динамических объектов (движущиеся сущности) можно отключить autoSync
 * и управлять TransformComponent напрямую.
 */
struct TilePositionComponent {
    int tileX = 0;           ///< X координата в тайлах
    int tileY = 0;           ///< Y координата в тайлах
    int widthTiles = 1;      ///< Ширина объекта в тайлах
    int heightTiles = 1;     ///< Высота объекта в тайлах
    bool autoSync = true;    ///< Автоматическая синхронизация с TransformComponent

    /**
     * @brief Получить пиксельную позицию (левый НИЖНИЙ угол)
     * @return Позиция в пикселях (левый нижний угол объекта)
     */
    sf::Vector2f getPixelPosition() const {
        return {
            static_cast<float>(tileX * TILE_SIZE),
            static_cast<float>((tileY + heightTiles) * TILE_SIZE)
        };
    }

    /**
     * @brief Получить центр объекта в пикселях
     * @return Центральная позиция в пикселях
     */
    sf::Vector2f getCenterPixel() const {
        return {
            (tileX + widthTiles * 0.5f) * TILE_SIZE,
            (tileY + heightTiles * 0.5f) * TILE_SIZE
        };
    }

    /**
     * @brief Преобразовать пиксельные координаты в тайловые
     * @param pixelPos Позиция в пикселях
     */
    void setFromPixelPosition(const sf::Vector2f& pixelPos) {
        tileX = static_cast<int>(pixelPos.x / TILE_SIZE);
        tileY = static_cast<int>((pixelPos.y / TILE_SIZE) - heightTiles);
    }

    /**
     * @brief Проверить, содержит ли объект указанный тайл
     * @param x Тайловая координата X
     * @param y Тайловая координата Y
     * @return true если тайл входит в область объекта
     */
    bool containsTile(int x, int y) const {
        return x >= tileX && x < tileX + widthTiles &&
               y >= tileY && y < tileY + heightTiles;
    }
};

/**
 * @brief Оверлей состояния объекта (кнопка, индикатор)
 *
 * Создается как отдельная entity с ParentComponent,
 * которая ссылается на родительский объект.
 * OverlaySystem автоматически синхронизирует позицию с родителем.
 */
struct OverlayComponent {
    sf::Vector2f localOffset;  ///< Смещение относительно позиции родителя
    bool syncWithParent = true; ///< Автоматическая синхронизация позиции с родителем

    OverlayComponent() : localOffset(0.0f, 0.0f), syncWithParent(true) {}

    explicit OverlayComponent(float offsetX, float offsetY, bool sync = true)
        : localOffset(offsetX, offsetY), syncWithParent(sync) {}

    explicit OverlayComponent(sf::Vector2f offset, bool sync = true)
        : localOffset(offset), syncWithParent(sync) {}
};

} // namespace core
