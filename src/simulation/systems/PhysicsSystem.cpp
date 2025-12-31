#include "simulation/systems/PhysicsSystem.h"
#include "simulation/PhysicsComponents.h"
#include "core/Components.h"
#include "core/Logger.h"
#include <algorithm>

namespace simulation {

PhysicsSystem::PhysicsSystem(PhysicsWorld& world)
    : m_physicsWorld(world), m_accumulator(0.0f) {
    LOG_INFO("PhysicsSystem initialized with fixed timestep: {} Hz", 1.0f / FIXED_TIMESTEP);
}

void PhysicsSystem::init(entt::registry& registry) {
    LOG_DEBUG("PhysicsSystem::init - Creating Box2D bodies for existing entities");

    // Создаём тела для всех существующих сущностей с RigidbodyComponent + ColliderComponent
    auto view = registry.view<RigidbodyComponent, ColliderComponent, core::TransformComponent>();

    int createdCount = 0;
    for (auto entity : view) {
        createBody(registry, entity);
        createdCount++;
    }

    LOG_INFO("PhysicsSystem::init - Created {} Box2D bodies", createdCount);

    // Регистрируем обработчики для новых сущностей
    // Когда к сущности добавляется RigidbodyComponent, создаём для неё тело
    registry.on_construct<RigidbodyComponent>().connect<&PhysicsSystem::createBody>(this);

    // Когда RigidbodyComponent удаляется, удаляем тело
    registry.on_destroy<RigidbodyComponent>().connect<&PhysicsSystem::destroyBody>(this);
}

void PhysicsSystem::update(entt::registry& registry, double dt) {
    // Ограничиваем dt для предотвращения spiral of death
    float deltaTime = static_cast<float>(dt);

    // Накапливаем время
    m_accumulator += deltaTime;

    // Ограничиваем накопитель (если кадр занял > 0.25 сек, пропускаем физику)
    if (m_accumulator > MAX_ACCUMULATOR) {
        LOG_WARN("PhysicsSystem: Frame time too large ({:.3f}s), clamping accumulator to {:.3f}s",
                 m_accumulator, MAX_ACCUMULATOR);
        m_accumulator = MAX_ACCUMULATOR;
    }

    // Выполняем фиксированные шаги физики
    int stepCount = 0;
    while (m_accumulator >= FIXED_TIMESTEP) {
        // Шаг симуляции Box2D
        m_physicsWorld.step(FIXED_TIMESTEP);

        m_accumulator -= FIXED_TIMESTEP;
        stepCount++;
    }

    // Синхронизируем позиции после всех шагов физики
    if (stepCount > 0) {
        syncTransforms(registry);
    }
}

void PhysicsSystem::syncTransforms(entt::registry& registry) {
    // Синхронизируем только динамические и кинематические тела
    // (статические не двигаются)
    auto view = registry.view<RigidbodyComponent, core::TransformComponent>();

    for (auto entity : view) {
        auto& rigidbody = view.get<RigidbodyComponent>(entity);
        auto& transform = view.get<core::TransformComponent>(entity);

        // Пропускаем если тело не создано в Box2D
        if (!rigidbody.hasBox2DBody()) {
            continue;
        }

        // Пропускаем статические тела (они не двигаются)
        if (rigidbody.isStatic()) {
            continue;
        }

        // Получаем позицию и угол из Box2D
        b2Vec2 position = b2Body_GetPosition(rigidbody.box2dBodyId);
        b2Rot rotation = b2Body_GetRotation(rigidbody.box2dBodyId);
        float angle = b2Rot_GetAngle(rotation);

        // Конвертируем из метров в пиксели
        b2Vec2 pixelPosB2 = PhysicsWorld::metersToPixels(position);
        sf::Vector2f pixelPos(pixelPosB2.x, pixelPosB2.y);

        // ВАЖНО: Box2D позиция = центр тела
        // TransformComponent позиция = bottom-left угол спрайта (по умолчанию)
        // Нужно сместить позицию на половину размера коллайдера

        // Получаем размер коллайдера для смещения origin
        if (registry.all_of<ColliderComponent>(entity)) {
            auto& collider = registry.get<ColliderComponent>(entity);

            // Вычисляем смещение в зависимости от типа коллайдера
            sf::Vector2f offset(0.0f, 0.0f);

            if (collider.shape == ColliderComponent::Shape::Box) {
                // Для Box: смещаем на половину размера влево и вверх
                offset.x = -collider.size.x * 0.5f;
                offset.y = -collider.size.y * 0.5f;
            } else if (collider.shape == ColliderComponent::Shape::Circle) {
                // Для Circle: смещаем на радиус
                offset.x = -collider.radius;
                offset.y = -collider.radius;
            }

            // Применяем смещение
            pixelPos.x += offset.x;
            pixelPos.y += offset.y;
        }

        // Обновляем TransformComponent
        transform.x = pixelPos.x;
        transform.y = pixelPos.y;
        transform.rotation = angle * (180.0f / B2_PI); // Радианы → градусы
    }
}

void PhysicsSystem::createBody(entt::registry& registry, entt::entity entity) {
    // Проверяем наличие всех необходимых компонентов
    if (!registry.all_of<RigidbodyComponent, ColliderComponent, core::TransformComponent>(entity)) {
        LOG_WARN("PhysicsSystem::createBody - Entity missing required components (Rigidbody, Collider, Transform)");
        return;
    }

    auto& rigidbody = registry.get<RigidbodyComponent>(entity);
    auto& collider = registry.get<ColliderComponent>(entity);
    auto& transform = registry.get<core::TransformComponent>(entity);

    // Если тело уже создано, пропускаем
    if (rigidbody.hasBox2DBody()) {
        LOG_WARN("PhysicsSystem::createBody - Body already exists for entity");
        return;
    }

    // Создаём b2BodyDef
    b2BodyDef bodyDef = b2DefaultBodyDef();

    // Устанавливаем тип тела
    if (rigidbody.bodyType == RigidbodyComponent::BodyType::Static) {
        bodyDef.type = b2_staticBody;
    } else if (rigidbody.bodyType == RigidbodyComponent::BodyType::Kinematic) {
        bodyDef.type = b2_kinematicBody;
    } else {
        bodyDef.type = b2_dynamicBody;
    }

    // ВАЖНО: TransformComponent содержит bottom-left угол спрайта
    // Box2D требует центр тела
    // Нужно сместить позицию на половину размера коллайдера
    sf::Vector2f centerOffset(0.0f, 0.0f);

    if (collider.shape == ColliderComponent::Shape::Box) {
        centerOffset.x = collider.size.x * 0.5f;
        centerOffset.y = collider.size.y * 0.5f;
    } else if (collider.shape == ColliderComponent::Shape::Circle) {
        centerOffset.x = collider.radius;
        centerOffset.y = collider.radius;
    }

    // Конвертируем позицию из пикселей в метры (с учётом смещения к центру)
    sf::Vector2f centerPos(transform.x + centerOffset.x, transform.y + centerOffset.y);
    bodyDef.position = PhysicsWorld::pixelsToMeters(b2Vec2{centerPos.x, centerPos.y});

    // Конвертируем угол из градусов в радианы
    bodyDef.rotation = b2MakeRot(transform.rotation * (B2_PI / 180.0f));

    // Физические свойства
    bodyDef.linearDamping = rigidbody.linearDamping;
    bodyDef.angularDamping = rigidbody.angularDamping;
    bodyDef.gravityScale = rigidbody.gravityScale;
    bodyDef.fixedRotation = rigidbody.fixedRotation;
    bodyDef.enableSleep = rigidbody.allowSleep;
    bodyDef.isBullet = rigidbody.isBullet;

    // Начальная скорость
    sf::Vector2f linearVel = rigidbody.linearVelocity;
    bodyDef.linearVelocity = PhysicsWorld::pixelsToMeters(b2Vec2{linearVel.x, linearVel.y});
    bodyDef.angularVelocity = rigidbody.angularVelocity;

    // Сохраняем entity в userData для получения из Box2D событий
    // Добавляем 1 к entity чтобы избежать nullptr для entity = 0
    // При извлечении нужно вычитать 1
    bodyDef.userData = reinterpret_cast<void*>(static_cast<uintptr_t>(entity) + 1);

    // Создаём тело в Box2D
    b2BodyId bodyId = b2CreateBody(m_physicsWorld.getWorldId(), &bodyDef);

    // Создаём shape для тела
    b2ShapeDef shapeDef = createShapeFromCollider(collider);

    // Добавляем shape к телу
    if (collider.shape == ColliderComponent::Shape::Box) {
        // Создаём box shape
        b2Vec2 halfSize = PhysicsWorld::pixelsToMeters(
            b2Vec2{collider.size.x * 0.5f, collider.size.y * 0.5f}
        );
        b2Polygon box = b2MakeBox(halfSize.x, halfSize.y);
        b2CreatePolygonShape(bodyId, &shapeDef, &box);

    } else if (collider.shape == ColliderComponent::Shape::Circle) {
        // Создаём circle shape
        b2Circle circle;
        circle.center = b2Vec2_zero;
        circle.radius = PhysicsWorld::pixelsToMeters(collider.radius);
        b2CreateCircleShape(bodyId, &shapeDef, &circle);

    } else if (collider.shape == ColliderComponent::Shape::Polygon) {
        // Создаём polygon shape
        if (!collider.isPolygonValid()) {
            LOG_ERROR("PhysicsSystem::createBody - Invalid polygon (vertices: {})", collider.vertices.size());
            b2DestroyBody(bodyId);
            return;
        }

        // Конвертируем вершины из пикселей в метры
        b2Vec2 vertices[8];
        for (size_t i = 0; i < collider.vertices.size(); ++i) {
            vertices[i] = PhysicsWorld::pixelsToMeters(
                b2Vec2{collider.vertices[i].x, collider.vertices[i].y}
            );
        }

        b2Hull hull = b2ComputeHull(vertices, static_cast<int>(collider.vertices.size()));
        b2Polygon polygon = b2MakePolygon(&hull, 0.0f);
        b2CreatePolygonShape(bodyId, &shapeDef, &polygon);
    }

    // Сохраняем ID тела в компоненте
    rigidbody.box2dBodyId = bodyId;

    LOG_DEBUG("PhysicsSystem::createBody - Created body for entity (type: {})",
              static_cast<int>(rigidbody.bodyType));
}

void PhysicsSystem::destroyBody(entt::registry& registry, entt::entity entity) {
    auto& rigidbody = registry.get<RigidbodyComponent>(entity);

    // Если тело не создано, ничего не делаем
    if (!rigidbody.hasBox2DBody()) {
        return;
    }

    // Удаляем тело из Box2D
    b2DestroyBody(rigidbody.box2dBodyId);

    // Сбрасываем ID
    rigidbody.box2dBodyId = b2_nullBodyId;

    LOG_DEBUG("PhysicsSystem::destroyBody - Destroyed body for entity");
}

b2ShapeDef PhysicsSystem::createShapeFromCollider(const ColliderComponent& collider) {
    b2ShapeDef shapeDef = b2DefaultShapeDef();

    // Физические свойства
    shapeDef.density = collider.density;
    shapeDef.material.friction = collider.friction;
    shapeDef.material.restitution = collider.restitution;
    shapeDef.isSensor = collider.isSensor;

    // Фильтрация коллизий
    shapeDef.filter.categoryBits = collider.categoryBits;
    shapeDef.filter.maskBits = collider.maskBits;
    shapeDef.filter.groupIndex = collider.groupIndex;

    // Включаем генерацию событий (Box2D 3.x)
    // enableContactEvents: события начала/конца контакта (для kinematic/dynamic)
    // enableSensorEvents: события сенсоров (должно быть включено на ОБОИХ shapes -
    //                     и на sensor, и на visitor - для генерации событий)
    // enableHitEvents: события высокоскоростных столкновений
    shapeDef.enableContactEvents = true;
    shapeDef.enableSensorEvents = true;  // Включаем для всех shapes
    shapeDef.enableHitEvents = true;

    return shapeDef;
}

b2BodyType PhysicsSystem::convertBodyType(int type) {
    switch (type) {
        case 0: return b2_staticBody;
        case 1: return b2_kinematicBody;
        case 2: return b2_dynamicBody;
        default: return b2_dynamicBody;
    }
}

} // namespace simulation
