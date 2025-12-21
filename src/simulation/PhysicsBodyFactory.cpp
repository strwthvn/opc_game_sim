#include <simulation/PhysicsBodyFactory.h>
#include <cmath>

namespace simulation {

// ============== PUBLIC METHODS ==============

b2BodyId PhysicsBodyFactory::createBody(
    const PhysicsWorld& world,
    const core::TransformComponent& transform,
    const RigidbodyComponent& rigidbody,
    const ColliderComponent& collider
) {
    // Создать определение тела
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = toBox2DBodyType(rigidbody.bodyType);

    // Конвертировать позицию из пикселей в метры
    bodyDef.position = b2Vec2{
        PhysicsWorld::pixelsToMeters(transform.x),
        PhysicsWorld::pixelsToMeters(transform.y)
    };

    // Конвертировать угол из градусов в радианы
    bodyDef.rotation = b2MakeRot(transform.rotation * B2_PI / 180.0f);

    // Применить физические свойства
    bodyDef.linearDamping = rigidbody.linearDamping;
    bodyDef.angularDamping = rigidbody.angularDamping;
    bodyDef.gravityScale = rigidbody.gravityScale;
    bodyDef.fixedRotation = rigidbody.fixedRotation;
    bodyDef.enableSleep = rigidbody.allowSleep;
    bodyDef.isBullet = rigidbody.isBullet;

    // Установить начальную скорость
    bodyDef.linearVelocity = b2Vec2{
        PhysicsWorld::pixelsToMeters(rigidbody.linearVelocity.x),
        PhysicsWorld::pixelsToMeters(rigidbody.linearVelocity.y)
    };
    bodyDef.angularVelocity = rigidbody.angularVelocity;

    // Создать тело
    b2BodyId bodyId = b2CreateBody(world.getWorldId(), &bodyDef);

    // Создать фикстуру (shape) из коллайдера
    createShape(bodyId, collider);

    return bodyId;
}

b2BodyId PhysicsBodyFactory::createBox(
    const PhysicsWorld& world,
    const core::TransformComponent& transform,
    const RigidbodyComponent& rigidbody,
    float width,
    float height
) {
    // Создать простой прямоугольный коллайдер
    ColliderComponent collider(width, height);
    return createBody(world, transform, rigidbody, collider);
}

b2BodyId PhysicsBodyFactory::createCircle(
    const PhysicsWorld& world,
    const core::TransformComponent& transform,
    const RigidbodyComponent& rigidbody,
    float radius
) {
    // Создать простой круглый коллайдер
    ColliderComponent collider(radius);
    return createBody(world, transform, rigidbody, collider);
}

void PhysicsBodyFactory::destroyBody(const PhysicsWorld& world, b2BodyId bodyId) {
    // Проверить валидность ID
    if (B2_IS_NULL(bodyId)) {
        return;
    }

    // Уничтожить тело (автоматически удаляет все связанные фикстуры)
    b2DestroyBody(bodyId);
}

// ============== PRIVATE METHODS ==============

b2BodyType PhysicsBodyFactory::toBox2DBodyType(RigidbodyComponent::BodyType type) {
    switch (type) {
        case RigidbodyComponent::BodyType::Static:
            return b2_staticBody;
        case RigidbodyComponent::BodyType::Kinematic:
            return b2_kinematicBody;
        case RigidbodyComponent::BodyType::Dynamic:
            return b2_dynamicBody;
        default:
            return b2_dynamicBody;
    }
}

b2ShapeId PhysicsBodyFactory::createShape(b2BodyId bodyId, const ColliderComponent& collider) {
    // Создать определение фикстуры
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = collider.density;
    shapeDef.material.friction = collider.friction;
    shapeDef.material.restitution = collider.restitution;
    shapeDef.isSensor = collider.isSensor;

    b2ShapeId shapeId;

    // Создать форму в зависимости от типа коллайдера
    switch (collider.shape) {
        case ColliderComponent::Shape::Box: {
            // Создать прямоугольник (box shape)
            // Box2D использует половину размера (half-extents)
            float halfWidth = PhysicsWorld::pixelsToMeters(collider.size.x * 0.5f);
            float halfHeight = PhysicsWorld::pixelsToMeters(collider.size.y * 0.5f);

            // Создать polygon (прямоугольник) с центром в origin + offset
            b2Polygon box = b2MakeBox(halfWidth, halfHeight);

            // Применить смещение, если указано
            if (collider.offset.x != 0.0f || collider.offset.y != 0.0f) {
                b2Vec2 offsetMeters{
                    PhysicsWorld::pixelsToMeters(collider.offset.x),
                    PhysicsWorld::pixelsToMeters(collider.offset.y)
                };
                box = b2MakeOffsetBox(halfWidth, halfHeight, offsetMeters, b2Rot_identity);
            }

            shapeId = b2CreatePolygonShape(bodyId, &shapeDef, &box);
            break;
        }

        case ColliderComponent::Shape::Circle: {
            // Создать круг (circle shape)
            b2Circle circle;
            circle.center = b2Vec2{
                PhysicsWorld::pixelsToMeters(collider.offset.x),
                PhysicsWorld::pixelsToMeters(collider.offset.y)
            };
            circle.radius = PhysicsWorld::pixelsToMeters(collider.radius);

            shapeId = b2CreateCircleShape(bodyId, &shapeDef, &circle);
            break;
        }

        case ColliderComponent::Shape::Polygon: {
            // Создать произвольный полигон
            if (!collider.isPolygonValid()) {
                // Фоллбэк: создать единичный квадрат если полигон невалиден
                b2Polygon box = b2MakeBox(0.5f, 0.5f);
                shapeId = b2CreatePolygonShape(bodyId, &shapeDef, &box);
                break;
            }

            // Конвертировать вершины из пикселей в метры
            b2Vec2 vertices[8]; // Box2D поддерживает до 8 вершин
            int vertexCount = static_cast<int>(collider.vertices.size());
            if (vertexCount > 8) {
                vertexCount = 8; // Ограничить до 8 вершин
            }

            for (int i = 0; i < vertexCount; ++i) {
                vertices[i] = b2Vec2{
                    PhysicsWorld::pixelsToMeters(collider.vertices[i].x + collider.offset.x),
                    PhysicsWorld::pixelsToMeters(collider.vertices[i].y + collider.offset.y)
                };
            }

            // Создать hull (выпуклую оболочку) из вершин
            b2Hull hull = b2ComputeHull(vertices, vertexCount);
            b2Polygon polygon = b2MakePolygon(&hull, 0.0f);

            shapeId = b2CreatePolygonShape(bodyId, &shapeDef, &polygon);
            break;
        }

        default:
            // Фоллбэк: создать единичный квадрат
            b2Polygon box = b2MakeBox(0.5f, 0.5f);
            shapeId = b2CreatePolygonShape(bodyId, &shapeDef, &box);
            break;
    }

    return shapeId;
}

} // namespace simulation
