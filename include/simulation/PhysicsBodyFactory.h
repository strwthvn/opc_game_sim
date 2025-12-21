#pragma once

#include "PhysicsComponents.h"
#include "PhysicsWorld.h"
#include <core/Components.h>
#include <box2d/box2d.h>

/**
 * @file PhysicsBodyFactory.h
 * @brief Фабрика для создания Box2D физических тел
 *
 * Предоставляет удобные методы для создания и уничтожения Box2D тел
 * с автоматической конвертацией координат из пиксельной системы в метры.
 */

namespace simulation {

/**
 * @brief Фабрика для создания Box2D физических тел из ECS компонентов
 *
 * PhysicsBodyFactory упрощает создание Box2D тел, автоматически конвертируя
 * координаты из пикселей в метры и применяя параметры из компонентов.
 *
 * Все методы принимают координаты в пикселях и возвращают b2BodyId.
 *
 * @code
 * // Пример создания динамического прямоугольного тела
 * core::TransformComponent transform{100.0f, 200.0f, 0.0f};
 * simulation::RigidbodyComponent rigidbody = RigidbodyComponent::createDynamic(2.0f);
 * simulation::ColliderComponent collider(64.0f, 32.0f); // 64x32 пикселей
 *
 * PhysicsBodyFactory factory;
 * b2BodyId bodyId = factory.createBody(world, transform, rigidbody, collider);
 * @endcode
 */
class PhysicsBodyFactory {
public:
    /**
     * @brief Создать Box2D тело из ECS компонентов
     *
     * Создаёт Box2D тело на основе Transform, Rigidbody и Collider компонентов.
     * Автоматически конвертирует координаты из пикселей в метры.
     *
     * @param world Физический мир Box2D
     * @param transform Transform компонент с позицией и углом поворота
     * @param rigidbody Rigidbody компонент с физическими свойствами
     * @param collider Collider компонент с формой коллайдера
     * @return ID созданного Box2D тела
     *
     * @note Позиция берётся из TransformComponent (x, y) в пикселях
     * @note Угол поворота конвертируется из градусов в радианы
     * @note Размеры коллайдера конвертируются из пикселей в метры
     */
    static b2BodyId createBody(
        const PhysicsWorld& world,
        const core::TransformComponent& transform,
        const RigidbodyComponent& rigidbody,
        const ColliderComponent& collider
    );

    /**
     * @brief Создать прямоугольное тело (упрощённый метод)
     *
     * Создаёт прямоугольное Box2D тело с заданными размерами.
     * Удобно для быстрого прототипирования.
     *
     * @param world Физический мир Box2D
     * @param transform Transform компонент с позицией
     * @param rigidbody Rigidbody компонент с физическими свойствами
     * @param width Ширина прямоугольника в пикселях
     * @param height Высота прямоугольника в пикселях
     * @return ID созданного Box2D тела
     *
     * @code
     * // Создать статическую платформу 128x32 пикселей
     * core::TransformComponent transform{200.0f, 300.0f};
     * auto rigidbody = RigidbodyComponent::createStatic();
     * b2BodyId bodyId = PhysicsBodyFactory::createBox(world, transform, rigidbody, 128.0f, 32.0f);
     * @endcode
     */
    static b2BodyId createBox(
        const PhysicsWorld& world,
        const core::TransformComponent& transform,
        const RigidbodyComponent& rigidbody,
        float width,
        float height
    );

    /**
     * @brief Создать круглое тело
     *
     * Создаёт круглое Box2D тело с заданным радиусом.
     *
     * @param world Физический мир Box2D
     * @param transform Transform компонент с позицией
     * @param rigidbody Rigidbody компонент с физическими свойствами
     * @param radius Радиус круга в пикселях
     * @return ID созданного Box2D тела
     *
     * @code
     * // Создать динамический мяч радиусом 16 пикселей
     * core::TransformComponent transform{100.0f, 100.0f};
     * auto rigidbody = RigidbodyComponent::createDynamic(1.0f);
     * b2BodyId bodyId = PhysicsBodyFactory::createCircle(world, transform, rigidbody, 16.0f);
     * @endcode
     */
    static b2BodyId createCircle(
        const PhysicsWorld& world,
        const core::TransformComponent& transform,
        const RigidbodyComponent& rigidbody,
        float radius
    );

    /**
     * @brief Уничтожить Box2D тело
     *
     * Безопасно уничтожает Box2D тело и освобождает ресурсы.
     * После вызова b2BodyId становится невалидным.
     *
     * @param world Физический мир Box2D
     * @param bodyId ID тела для уничтожения
     *
     * @note Метод безопасен для вызова с b2_nullBodyId (ничего не делает)
     *
     * @code
     * // Удалить тело
     * if (B2_IS_NON_NULL(bodyId)) {
     *     PhysicsBodyFactory::destroyBody(world, bodyId);
     *     bodyId = b2_nullBodyId;
     * }
     * @endcode
     */
    static void destroyBody(const PhysicsWorld& world, b2BodyId bodyId);

private:
    /**
     * @brief Конвертировать тип тела из Rigidbody в Box2D формат
     *
     * @param type Тип тела из RigidbodyComponent
     * @return Тип тела Box2D
     */
    static b2BodyType toBox2DBodyType(RigidbodyComponent::BodyType type);

    /**
     * @brief Создать фикстуру (shape) для тела из ColliderComponent
     *
     * @param bodyId ID тела, к которому добавляется фикстура
     * @param collider Collider компонент с параметрами фикстуры
     * @return ID созданной фикстуры (shape)
     */
    static b2ShapeId createShape(b2BodyId bodyId, const ColliderComponent& collider);
};

} // namespace simulation
