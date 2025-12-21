#pragma once

#include <core/systems/ISystem.h>
#include <simulation/PhysicsWorld.h>
#include <entt/entt.hpp>

/**
 * @file PhysicsSystem.h
 * @brief Система управления Box2D физической симуляцией
 *
 * PhysicsSystem интегрирует ECS сущности с Box2D физикой:
 * - Создаёт b2Body для сущностей с RigidbodyComponent + ColliderComponent
 * - Выполняет физическую симуляцию с фиксированным timestep (60 Hz)
 * - Синхронизирует TransformComponent с позициями Box2D тел
 */

namespace simulation {

/**
 * @brief Система интеграции ECS с Box2D физическим движком
 *
 * Управляет созданием Box2D тел, выполнением симуляции и синхронизацией
 * позиций между Box2D и TransformComponent.
 *
 * **Архитектура:**
 * - Сущности с RigidbodyComponent + ColliderComponent → создаются b2Body
 * - После step(): позиции b2Body → TransformComponent (только для Dynamic/Kinematic)
 * - Статические тела создаются один раз и не обновляются
 *
 * **Координаты:**
 * - TransformComponent: пиксели (origin = bottom-left для совместимости с рендерингом)
 * - Box2D: метры (конвертация через PhysicsWorld::PIXELS_PER_METER = 32)
 * - Y-ось: направлена вниз (как в SFML)
 *
 * **Приоритет:**
 * - 150 (после игровой логики, до синхронизации позиций)
 *
 * @note Использует фиксированный timestep (1/60 сек) для стабильности симуляции.
 * @note Поддерживает accumulator pattern для переменного frame time.
 */
class PhysicsSystem : public core::ISystem {
public:
    /**
     * @brief Фиксированный timestep для физики (60 Hz)
     */
    static constexpr float FIXED_TIMESTEP = 1.0f / 60.0f;

    /**
     * @brief Максимальный накопитель времени (предотвращает spiral of death)
     *
     * Если кадр занимает слишком много времени (> 0.25 сек),
     * ограничиваем количество физических шагов.
     */
    static constexpr float MAX_ACCUMULATOR = 0.25f;

    /**
     * @brief Конструктор
     * @param world Ссылка на PhysicsWorld (должен существовать на протяжении жизни системы)
     */
    explicit PhysicsSystem(PhysicsWorld& world);

    /**
     * @brief Деструктор
     *
     * Удаляет все созданные Box2D тела.
     */
    ~PhysicsSystem() override = default;

    /**
     * @brief Инициализация системы
     *
     * Создаёт Box2D тела для всех существующих сущностей с RigidbodyComponent + ColliderComponent.
     * Вызывается один раз при старте игры.
     *
     * @param registry EnTT registry
     *
     * @note Для каждой сущности:
     *       1. Создаётся b2BodyDef с типом из RigidbodyComponent
     *       2. Позиция из TransformComponent конвертируется в метры
     *       3. Создаётся b2Body через b2CreateBody()
     *       4. Добавляется b2ShapeDef из ColliderComponent
     *       5. box2dBodyId сохраняется в RigidbodyComponent
     */
    void init(entt::registry& registry);

    /**
     * @brief Обновление физики с фиксированным timestep
     *
     * Реализует accumulator pattern:
     * 1. Накапливает время: m_accumulator += dt
     * 2. Пока m_accumulator >= FIXED_TIMESTEP:
     *    - Выполняет шаг физики: m_physicsWorld.step(FIXED_TIMESTEP)
     *    - Уменьшает m_accumulator -= FIXED_TIMESTEP
     * 3. Синхронизирует TransformComponent с b2Body после всех шагов
     *
     * @param registry EnTT registry
     * @param dt Время с последнего кадра (секунды)
     *
     * @note Использует фиксированный timestep (60 Hz) для детерминизма.
     * @note Ограничивает m_accumulator через MAX_ACCUMULATOR (предотвращает spiral of death).
     */
    void update(entt::registry& registry, double dt) override;

    /**
     * @brief Синхронизация TransformComponent с Box2D
     *
     * Копирует позиции и углы из b2Body в TransformComponent:
     * - Только для Dynamic и Kinematic тел (Static не двигаются)
     * - Конвертирует метры → пиксели (PhysicsWorld::metersToPixels)
     * - Учитывает bottom-left origin спрайтов (позиция b2Body = центр, нужно сместить)
     *
     * @param registry EnTT registry
     *
     * @note Вызывается после каждого/всех шагов физики.
     */
    void syncTransforms(entt::registry& registry);

    /**
     * @brief Получить приоритет системы
     *
     * Физика выполняется после игровой логики (0-99),
     * но до синхронизации позиций (200+).
     *
     * @return 150
     */
    int getPriority() const override { return 150; }

    /**
     * @brief Получить имя системы
     * @return "PhysicsSystem"
     */
    const char* getName() const override { return "PhysicsSystem"; }

    /**
     * @brief Создать Box2D тело для сущности
     *
     * Создаёт b2Body на основе RigidbodyComponent и ColliderComponent.
     * Автоматически вызывается для новых сущностей при их создании.
     *
     * @param registry EnTT registry
     * @param entity Сущность для создания тела
     *
     * @note Требует наличие TransformComponent, RigidbodyComponent, ColliderComponent.
     * @note Сохраняет b2BodyId в RigidbodyComponent::box2dBodyId.
     */
    void createBody(entt::registry& registry, entt::entity entity);

    /**
     * @brief Удалить Box2D тело для сущности
     *
     * Уничтожает b2Body при удалении сущности или компонента.
     *
     * @param registry EnTT registry
     * @param entity Сущность для удаления тела
     *
     * @note Автоматически вызывается при on_destroy событиях.
     */
    void destroyBody(entt::registry& registry, entt::entity entity);

private:
    PhysicsWorld& m_physicsWorld;   ///< Ссылка на PhysicsWorld
    float m_accumulator = 0.0f;     ///< Накопитель времени для fixed timestep

    /**
     * @brief Создать b2ShapeDef из ColliderComponent
     *
     * Конвертирует данные ColliderComponent (Box, Circle, Polygon) в Box2D формат.
     *
     * @param collider Компонент коллайдера
     * @return b2ShapeDef для добавления к b2Body
     *
     * @note Конвертирует размеры из пикселей в метры.
     */
    b2ShapeDef createShapeFromCollider(const struct ColliderComponent& collider);

    /**
     * @brief Конвертировать BodyType ECS → b2BodyType
     *
     * @param type Тип тела из RigidbodyComponent
     * @return Тип тела для Box2D
     */
    b2BodyType convertBodyType(int type);
};

} // namespace simulation
