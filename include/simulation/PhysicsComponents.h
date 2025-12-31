#pragma once

#include <box2d/box2d.h>
#include <SFML/System/Vector2.hpp>
#include <vector>
#include <cstddef>

/**
 * @file PhysicsComponents.h
 * @brief Компоненты для интеграции с физическим движком Box2D 3.x
 *
 * Эти компоненты определяют физические свойства объектов и формы коллайдеров
 * для интеграции с Box2D Physics Engine (версия 3.x).
 *
 * @note Box2D 3.x использует ID-based API вместо указателей.
 */

namespace simulation {

// ============== КОМПОНЕНТЫ ДЛЯ Box2D ФИЗИКИ ==============

/**
 * @brief Компонент коллайдера для Box2D физики
 *
 * Определяет форму и свойства коллайдера объекта.
 * Поддерживает различные типы форм: прямоугольник, круг, полигон.
 *
 * ВАЖНО: Этот компонент предназначен для интеграции с Box2D.
 * Для простых тайловых коллизий используйте core::CollisionComponent.
 */
struct ColliderComponent {
    /**
     * @brief Типы форм коллайдера
     */
    enum class Shape {
        Box,        ///< Прямоугольный коллайдер (AABB или OBB)
        Circle,     ///< Круглый коллайдер
        Polygon     ///< Произвольный выпуклый полигон (до 8 вершин в Box2D)
    };

    Shape shape = Shape::Box;           ///< Тип формы коллайдера

    // === Параметры для Box ===
    sf::Vector2f size{32.0f, 32.0f};    ///< Размер прямоугольника (ширина, высота) в пикселях

    // === Параметры для Circle ===
    float radius = 16.0f;               ///< Радиус круга в пикселях

    // === Параметры для Polygon ===
    std::vector<sf::Vector2f> vertices; ///< Вершины полигона (относительно центра, против часовой стрелки)

    // === Общие параметры ===
    sf::Vector2f offset{0.0f, 0.0f};    ///< Смещение коллайдера относительно Transform сущности

    float density = 1.0f;               ///< Плотность материала (кг/м²)
    float friction = 0.3f;              ///< Коэффициент трения (0-1)
    float restitution = 0.0f;           ///< Упругость/отскок (0 = не отскакивает, 1 = полностью упругий)

    bool isSensor = false;              ///< Триггер без физического взаимодействия (проходимый)

    // === Фильтрация коллизий ===
    /**
     * @brief Категория этого коллайдера (битовая маска)
     *
     * Используется для фильтрации коллизий. Коллизия происходит, если:
     * (colliderA.categoryBits & colliderB.maskBits) != 0 И
     * (colliderB.categoryBits & colliderA.maskBits) != 0
     *
     * По умолчанию: 0x0001 (все объекты в одной категории)
     */
    uint16_t categoryBits = 0x0001;

    /**
     * @brief Маска категорий для фильтрации коллизий
     *
     * Определяет, с какими категориями объектов может сталкиваться данный коллайдер.
     * По умолчанию: 0xFFFF (столкновения со всеми категориями)
     *
     * @code
     * // Пример: Игрок сталкивается только со стенами и врагами
     * collider.categoryBits = 0x0001; // Категория: Player
     * collider.maskBits = 0x0006;     // Столкновения с: Walls (0x0002) | Enemies (0x0004)
     * @endcode
     */
    uint16_t maskBits = 0xFFFF;

    /**
     * @brief Группа коллизий
     *
     * Если groupIndex != 0:
     * - Положительное значение: объекты с одинаковым groupIndex ВСЕГДА сталкиваются
     * - Отрицательное значение: объекты с одинаковым groupIndex НИКОГДА не сталкиваются
     * - 0: используется стандартная фильтрация по categoryBits/maskBits
     */
    int16_t groupIndex = 0;

    /**
     * @brief Конструктор по умолчанию (прямоугольный коллайдер 32x32)
     */
    ColliderComponent() = default;

    /**
     * @brief Конструктор для прямоугольного коллайдера
     * @param width Ширина в пикселях
     * @param height Высота в пикселях
     */
    ColliderComponent(float width, float height)
        : shape(Shape::Box), size(width, height) {}

    /**
     * @brief Конструктор для круглого коллайдера
     * @param r Радиус в пикселях
     */
    explicit ColliderComponent(float r)
        : shape(Shape::Circle), radius(r) {}

    /**
     * @brief Конструктор для полигонального коллайдера
     * @param verts Вершины полигона (относительные координаты)
     */
    explicit ColliderComponent(const std::vector<sf::Vector2f>& verts)
        : shape(Shape::Polygon), vertices(verts) {}

    /**
     * @brief Установить размер из тайловых координат
     * @param widthTiles Ширина в тайлах
     * @param heightTiles Высота в тайлах
     * @param tileSize Размер одного тайла в пикселях (по умолчанию 32)
     */
    void setFromTileSize(int widthTiles, int heightTiles, int tileSize = 32) {
        shape = Shape::Box;
        size = sf::Vector2f(
            static_cast<float>(widthTiles * tileSize),
            static_cast<float>(heightTiles * tileSize)
        );
    }

    /**
     * @brief Создать прямоугольный коллайдер из тайловых размеров
     * @param widthTiles Ширина в тайлах
     * @param heightTiles Высота в тайлах
     * @param tileSize Размер тайла (по умолчанию 32)
     * @return Настроенный ColliderComponent
     */
    static ColliderComponent createBox(int widthTiles, int heightTiles, int tileSize = 32) {
        ColliderComponent collider;
        collider.setFromTileSize(widthTiles, heightTiles, tileSize);
        return collider;
    }

    /**
     * @brief Проверить корректность полигона (минимум 3 вершины, максимум 8)
     * @return true если полигон валиден
     */
    bool isPolygonValid() const {
        return shape == Shape::Polygon && vertices.size() >= 3 && vertices.size() <= 8;
    }
};

/**
 * @brief Компонент твёрдого тела для Box2D физики
 *
 * Определяет физические свойства и поведение объекта в симуляции.
 * Хранит ссылку на b2Body после интеграции с Box2D.
 */
struct RigidbodyComponent {
    /**
     * @brief Типы твёрдых тел
     */
    enum class BodyType {
        Static,     ///< Неподвижное тело (стены, пол)
        Kinematic,  ///< Управляется скриптом, не реагирует на силы (платформы)
        Dynamic     ///< Полноценное физическое тело (падающие объекты)
    };

    BodyType bodyType = BodyType::Dynamic;  ///< Тип тела

    // === Физические свойства ===
    float mass = 1.0f;                      ///< Масса (кг), игнорируется для Static/Kinematic
    float linearDamping = 0.0f;             ///< Затухание линейной скорости (0 = без затухания)
    float angularDamping = 0.0f;            ///< Затухание угловой скорости (0 = без затухания)
    float gravityScale = 1.0f;              ///< Множитель гравитации (1.0 = нормальная, 0.0 = невесомость)

    // === Ограничения ===
    bool fixedRotation = false;             ///< Запретить вращение (полезно для персонажей)
    bool allowSleep = true;                 ///< Разрешить "засыпание" неактивных тел (оптимизация)
    bool isBullet = false;                  ///< Непрерывная детекция коллизий для быстрых объектов

    // === Начальная скорость ===
    sf::Vector2f linearVelocity{0.0f, 0.0f};  ///< Начальная линейная скорость (пиксели/сек)
    float angularVelocity = 0.0f;             ///< Начальная угловая скорость (радианы/сек)

    // === Интеграция с Box2D 3.x ===
    b2BodyId box2dBodyId = b2_nullBodyId;   ///< ID Box2D тела (b2_nullBodyId до создания)

    /**
     * @brief Конструктор по умолчанию (динамическое тело с массой 1 кг)
     */
    RigidbodyComponent() = default;

    /**
     * @brief Конструктор с типом тела
     * @param type Тип твёрдого тела
     * @param m Масса (используется только для Dynamic)
     */
    explicit RigidbodyComponent(BodyType type, float m = 1.0f)
        : bodyType(type), mass(m) {}

    /**
     * @brief Проверить, является ли тело статичным
     * @return true если тело Static
     */
    bool isStatic() const {
        return bodyType == BodyType::Static;
    }

    /**
     * @brief Проверить, является ли тело кинематическим
     * @return true если тело Kinematic
     */
    bool isKinematic() const {
        return bodyType == BodyType::Kinematic;
    }

    /**
     * @brief Проверить, является ли тело динамическим
     * @return true если тело Dynamic
     */
    bool isDynamic() const {
        return bodyType == BodyType::Dynamic;
    }

    /**
     * @brief Проверить, связан ли компонент с Box2D телом
     * @return true если тело создано в Box2D
     */
    bool hasBox2DBody() const {
        return B2_IS_NON_NULL(box2dBodyId);
    }

    /**
     * @brief Создать статическое тело (для стен, пола)
     * @return Настроенный RigidbodyComponent
     */
    static RigidbodyComponent createStatic() {
        return RigidbodyComponent(BodyType::Static, 0.0f);
    }

    /**
     * @brief Создать кинематическое тело (для платформ, дверей)
     * @return Настроенный RigidbodyComponent
     */
    static RigidbodyComponent createKinematic() {
        return RigidbodyComponent(BodyType::Kinematic, 0.0f);
    }

    /**
     * @brief Создать динамическое тело (для падающих объектов)
     * @param m Масса в килограммах
     * @return Настроенный RigidbodyComponent
     */
    static RigidbodyComponent createDynamic(float m = 1.0f) {
        return RigidbodyComponent(BodyType::Dynamic, m);
    }
};

} // namespace simulation
