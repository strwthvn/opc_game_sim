/**
 * @file test_physics_body_factory.cpp
 * @brief Unit tests for PhysicsBodyFactory (Box2D body creation from ECS components)
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <simulation/PhysicsBodyFactory.h>
#include <simulation/PhysicsWorld.h>
#include <simulation/PhysicsComponents.h>
#include <core/Components.h>
#include <box2d/box2d.h>

using namespace simulation;
using namespace core;

TEST_CASE("PhysicsBodyFactory: Create box body (static)", "[PhysicsBodyFactory]") {
    PhysicsWorld world(b2Vec2{0.0f, 9.8f});

    SECTION("Create static box from components") {
        // Create components
        TransformComponent transform{100.0f, 200.0f, 0.0f};
        RigidbodyComponent rigidbody = RigidbodyComponent::createStatic();
        ColliderComponent collider(64.0f, 32.0f); // 64x32 pixels

        // Create body
        b2BodyId bodyId = PhysicsBodyFactory::createBody(world, transform, rigidbody, collider);

        REQUIRE(b2Body_IsValid(bodyId) == true);
        REQUIRE(b2Body_GetType(bodyId) == b2_staticBody);

        // Check position (converted from pixels to meters)
        b2Vec2 position = b2Body_GetPosition(bodyId);
        REQUIRE_THAT(position.x, Catch::Matchers::WithinAbs(100.0f / 32.0f, 0.01f));
        REQUIRE_THAT(position.y, Catch::Matchers::WithinAbs(200.0f / 32.0f, 0.01f));

        // Cleanup
        PhysicsBodyFactory::destroyBody(world, bodyId);
    }
}

TEST_CASE("PhysicsBodyFactory: Create box body (dynamic)", "[PhysicsBodyFactory]") {
    PhysicsWorld world(b2Vec2{0.0f, 9.8f});

    SECTION("Create dynamic box from components") {
        TransformComponent transform{64.0f, 128.0f, 0.0f};
        RigidbodyComponent rigidbody = RigidbodyComponent::createDynamic(2.0f);
        ColliderComponent collider(32.0f, 32.0f);
        collider.density = 2.0f;
        collider.friction = 0.5f;
        collider.restitution = 0.3f;

        b2BodyId bodyId = PhysicsBodyFactory::createBody(world, transform, rigidbody, collider);

        REQUIRE(b2Body_IsValid(bodyId) == true);
        REQUIRE(b2Body_GetType(bodyId) == b2_dynamicBody);

        // Check position
        b2Vec2 position = b2Body_GetPosition(bodyId);
        REQUIRE_THAT(position.x, Catch::Matchers::WithinAbs(64.0f / 32.0f, 0.01f));
        REQUIRE_THAT(position.y, Catch::Matchers::WithinAbs(128.0f / 32.0f, 0.01f));

        // Cleanup
        PhysicsBodyFactory::destroyBody(world, bodyId);
    }
}

TEST_CASE("PhysicsBodyFactory: Create box body (kinematic)", "[PhysicsBodyFactory]") {
    PhysicsWorld world(b2Vec2{0.0f, 9.8f});

    SECTION("Create kinematic box from components") {
        TransformComponent transform{200.0f, 300.0f, 0.0f};
        RigidbodyComponent rigidbody = RigidbodyComponent::createKinematic();
        ColliderComponent collider(64.0f, 64.0f);

        b2BodyId bodyId = PhysicsBodyFactory::createBody(world, transform, rigidbody, collider);

        REQUIRE(b2Body_IsValid(bodyId) == true);
        REQUIRE(b2Body_GetType(bodyId) == b2_kinematicBody);

        // Cleanup
        PhysicsBodyFactory::destroyBody(world, bodyId);
    }
}

TEST_CASE("PhysicsBodyFactory: Create box with simplified method", "[PhysicsBodyFactory]") {
    PhysicsWorld world;

    SECTION("Create static platform using createBox helper") {
        TransformComponent transform{0.0f, 0.0f, 0.0f};
        RigidbodyComponent rigidbody = RigidbodyComponent::createStatic();

        b2BodyId bodyId = PhysicsBodyFactory::createBox(world, transform, rigidbody, 128.0f, 32.0f);

        REQUIRE(b2Body_IsValid(bodyId) == true);
        REQUIRE(b2Body_GetType(bodyId) == b2_staticBody);

        // Cleanup
        PhysicsBodyFactory::destroyBody(world, bodyId);
    }

    SECTION("Create dynamic box using createBox helper") {
        TransformComponent transform{100.0f, 100.0f, 0.0f};
        RigidbodyComponent rigidbody = RigidbodyComponent::createDynamic(1.5f);

        b2BodyId bodyId = PhysicsBodyFactory::createBox(world, transform, rigidbody, 64.0f, 64.0f);

        REQUIRE(b2Body_IsValid(bodyId) == true);
        REQUIRE(b2Body_GetType(bodyId) == b2_dynamicBody);

        // Cleanup
        PhysicsBodyFactory::destroyBody(world, bodyId);
    }
}

TEST_CASE("PhysicsBodyFactory: Create circle body", "[PhysicsBodyFactory]") {
    PhysicsWorld world(b2Vec2{0.0f, 9.8f});

    SECTION("Create dynamic circle using createCircle helper") {
        TransformComponent transform{150.0f, 250.0f, 0.0f};
        RigidbodyComponent rigidbody = RigidbodyComponent::createDynamic(1.0f);

        b2BodyId bodyId = PhysicsBodyFactory::createCircle(world, transform, rigidbody, 16.0f);

        REQUIRE(b2Body_IsValid(bodyId) == true);
        REQUIRE(b2Body_GetType(bodyId) == b2_dynamicBody);

        // Check position
        b2Vec2 position = b2Body_GetPosition(bodyId);
        REQUIRE_THAT(position.x, Catch::Matchers::WithinAbs(150.0f / 32.0f, 0.01f));
        REQUIRE_THAT(position.y, Catch::Matchers::WithinAbs(250.0f / 32.0f, 0.01f));

        // Cleanup
        PhysicsBodyFactory::destroyBody(world, bodyId);
    }

    SECTION("Create static circle") {
        TransformComponent transform{50.0f, 50.0f, 0.0f};
        RigidbodyComponent rigidbody = RigidbodyComponent::createStatic();

        b2BodyId bodyId = PhysicsBodyFactory::createCircle(world, transform, rigidbody, 20.0f);

        REQUIRE(b2Body_IsValid(bodyId) == true);
        REQUIRE(b2Body_GetType(bodyId) == b2_staticBody);

        // Cleanup
        PhysicsBodyFactory::destroyBody(world, bodyId);
    }
}

TEST_CASE("PhysicsBodyFactory: Collider parameters are applied", "[PhysicsBodyFactory]") {
    PhysicsWorld world;

    SECTION("Density, friction, restitution applied to shape") {
        TransformComponent transform{0.0f, 0.0f, 0.0f};
        RigidbodyComponent rigidbody = RigidbodyComponent::createDynamic(1.0f);
        ColliderComponent collider(32.0f, 32.0f);
        collider.density = 2.5f;
        collider.friction = 0.8f;
        collider.restitution = 0.6f;

        b2BodyId bodyId = PhysicsBodyFactory::createBody(world, transform, rigidbody, collider);

        REQUIRE(b2Body_IsValid(bodyId) == true);

        // Get the shape (first shape attached to body)
        int shapeCount = b2Body_GetShapeCount(bodyId);
        REQUIRE(shapeCount == 1);

        b2ShapeId shapes[1];
        b2Body_GetShapes(bodyId, shapes, 1);
        b2ShapeId shapeId = shapes[0];

        // Check shape properties
        REQUIRE_THAT(b2Shape_GetDensity(shapeId), Catch::Matchers::WithinAbs(2.5f, 0.01f));
        REQUIRE_THAT(b2Shape_GetFriction(shapeId), Catch::Matchers::WithinAbs(0.8f, 0.01f));
        REQUIRE_THAT(b2Shape_GetRestitution(shapeId), Catch::Matchers::WithinAbs(0.6f, 0.01f));

        // Cleanup
        PhysicsBodyFactory::destroyBody(world, bodyId);
    }
}

TEST_CASE("PhysicsBodyFactory: Rigidbody parameters are applied", "[PhysicsBodyFactory]") {
    PhysicsWorld world;

    SECTION("Fixed rotation applied") {
        TransformComponent transform{0.0f, 0.0f, 0.0f};
        RigidbodyComponent rigidbody = RigidbodyComponent::createDynamic(1.0f);
        rigidbody.fixedRotation = true;
        ColliderComponent collider(32.0f, 32.0f);

        b2BodyId bodyId = PhysicsBodyFactory::createBody(world, transform, rigidbody, collider);

        REQUIRE(b2Body_IsValid(bodyId) == true);
        REQUIRE(b2Body_IsFixedRotation(bodyId) == true);

        // Cleanup
        PhysicsBodyFactory::destroyBody(world, bodyId);
    }

    SECTION("Gravity scale applied") {
        TransformComponent transform{0.0f, 0.0f, 0.0f};
        RigidbodyComponent rigidbody = RigidbodyComponent::createDynamic(1.0f);
        rigidbody.gravityScale = 2.0f;
        ColliderComponent collider(32.0f, 32.0f);

        b2BodyId bodyId = PhysicsBodyFactory::createBody(world, transform, rigidbody, collider);

        REQUIRE(b2Body_IsValid(bodyId) == true);
        REQUIRE_THAT(b2Body_GetGravityScale(bodyId), Catch::Matchers::WithinAbs(2.0f, 0.01f));

        // Cleanup
        PhysicsBodyFactory::destroyBody(world, bodyId);
    }

    SECTION("Linear and angular damping applied") {
        TransformComponent transform{0.0f, 0.0f, 0.0f};
        RigidbodyComponent rigidbody = RigidbodyComponent::createDynamic(1.0f);
        rigidbody.linearDamping = 0.5f;
        rigidbody.angularDamping = 0.3f;
        ColliderComponent collider(32.0f, 32.0f);

        b2BodyId bodyId = PhysicsBodyFactory::createBody(world, transform, rigidbody, collider);

        REQUIRE(b2Body_IsValid(bodyId) == true);
        REQUIRE_THAT(b2Body_GetLinearDamping(bodyId), Catch::Matchers::WithinAbs(0.5f, 0.01f));
        REQUIRE_THAT(b2Body_GetAngularDamping(bodyId), Catch::Matchers::WithinAbs(0.3f, 0.01f));

        // Cleanup
        PhysicsBodyFactory::destroyBody(world, bodyId);
    }

    SECTION("Initial velocity applied") {
        TransformComponent transform{0.0f, 0.0f, 0.0f};
        RigidbodyComponent rigidbody = RigidbodyComponent::createDynamic(1.0f);
        rigidbody.linearVelocity = sf::Vector2f(100.0f, 50.0f); // pixels/sec
        rigidbody.angularVelocity = 1.5f; // rad/sec
        ColliderComponent collider(32.0f, 32.0f);
        collider.density = 1.0f;

        b2BodyId bodyId = PhysicsBodyFactory::createBody(world, transform, rigidbody, collider);

        REQUIRE(b2Body_IsValid(bodyId) == true);

        b2Vec2 linearVel = b2Body_GetLinearVelocity(bodyId);
        REQUIRE_THAT(linearVel.x, Catch::Matchers::WithinAbs(100.0f / 32.0f, 0.01f));
        REQUIRE_THAT(linearVel.y, Catch::Matchers::WithinAbs(50.0f / 32.0f, 0.01f));

        float angularVel = b2Body_GetAngularVelocity(bodyId);
        REQUIRE_THAT(angularVel, Catch::Matchers::WithinAbs(1.5f, 0.01f));

        // Cleanup
        PhysicsBodyFactory::destroyBody(world, bodyId);
    }
}

TEST_CASE("PhysicsBodyFactory: Coordinate conversion is correct", "[PhysicsBodyFactory]") {
    PhysicsWorld world;

    SECTION("Pixel coordinates converted to meters") {
        TransformComponent transform{320.0f, 640.0f, 0.0f};
        RigidbodyComponent rigidbody = RigidbodyComponent::createDynamic(1.0f);
        ColliderComponent collider(64.0f, 64.0f);

        b2BodyId bodyId = PhysicsBodyFactory::createBody(world, transform, rigidbody, collider);

        b2Vec2 position = b2Body_GetPosition(bodyId);

        // 320 pixels = 10 meters, 640 pixels = 20 meters (at 32 pixels/meter)
        REQUIRE_THAT(position.x, Catch::Matchers::WithinAbs(10.0f, 0.01f));
        REQUIRE_THAT(position.y, Catch::Matchers::WithinAbs(20.0f, 0.01f));

        // Cleanup
        PhysicsBodyFactory::destroyBody(world, bodyId);
    }

    SECTION("Rotation in degrees converted to radians") {
        TransformComponent transform{0.0f, 0.0f, 90.0f}; // 90 degrees
        RigidbodyComponent rigidbody = RigidbodyComponent::createDynamic(1.0f);
        ColliderComponent collider(32.0f, 32.0f);

        b2BodyId bodyId = PhysicsBodyFactory::createBody(world, transform, rigidbody, collider);

        b2Rot rotation = b2Body_GetRotation(bodyId);
        float angle = b2Rot_GetAngle(rotation);

        // 90 degrees = π/2 radians ≈ 1.5708
        REQUIRE_THAT(angle, Catch::Matchers::WithinAbs(1.5708f, 0.01f));

        // Cleanup
        PhysicsBodyFactory::destroyBody(world, bodyId);
    }
}

TEST_CASE("PhysicsBodyFactory: Destroy body", "[PhysicsBodyFactory]") {
    PhysicsWorld world;

    SECTION("Destroy valid body") {
        TransformComponent transform{0.0f, 0.0f, 0.0f};
        RigidbodyComponent rigidbody = RigidbodyComponent::createDynamic(1.0f);
        ColliderComponent collider(32.0f, 32.0f);

        b2BodyId bodyId = PhysicsBodyFactory::createBody(world, transform, rigidbody, collider);
        REQUIRE(b2Body_IsValid(bodyId) == true);

        PhysicsBodyFactory::destroyBody(world, bodyId);

        // After destruction, body should be invalid
        REQUIRE(b2Body_IsValid(bodyId) == false);
    }

    SECTION("Destroy null body (should not crash)") {
        b2BodyId nullBody = b2_nullBodyId;

        REQUIRE_NOTHROW(PhysicsBodyFactory::destroyBody(world, nullBody));
    }
}

TEST_CASE("PhysicsBodyFactory: Multiple bodies creation", "[PhysicsBodyFactory]") {
    PhysicsWorld world;

    SECTION("Create and destroy multiple bodies") {
        std::vector<b2BodyId> bodies;

        // Create 10 bodies
        for (int i = 0; i < 10; ++i) {
            TransformComponent transform{static_cast<float>(i * 64), 100.0f, 0.0f};
            RigidbodyComponent rigidbody = RigidbodyComponent::createDynamic(1.0f);
            ColliderComponent collider(32.0f, 32.0f);

            b2BodyId bodyId = PhysicsBodyFactory::createBody(world, transform, rigidbody, collider);
            bodies.push_back(bodyId);

            REQUIRE(b2Body_IsValid(bodyId) == true);
        }

        // All bodies should be valid
        for (const auto& bodyId : bodies) {
            REQUIRE(b2Body_IsValid(bodyId) == true);
        }

        // Destroy all bodies
        for (const auto& bodyId : bodies) {
            PhysicsBodyFactory::destroyBody(world, bodyId);
        }

        // All bodies should be invalid after destruction
        for (const auto& bodyId : bodies) {
            REQUIRE(b2Body_IsValid(bodyId) == false);
        }
    }
}

TEST_CASE("PhysicsBodyFactory: Sensor collider", "[PhysicsBodyFactory]") {
    PhysicsWorld world;

    SECTION("Create sensor (trigger) collider") {
        TransformComponent transform{0.0f, 0.0f, 0.0f};
        RigidbodyComponent rigidbody = RigidbodyComponent::createStatic();
        ColliderComponent collider(64.0f, 64.0f);
        collider.isSensor = true;

        b2BodyId bodyId = PhysicsBodyFactory::createBody(world, transform, rigidbody, collider);

        REQUIRE(b2Body_IsValid(bodyId) == true);

        // Get the shape and verify it's a sensor
        b2ShapeId shapes[1];
        b2Body_GetShapes(bodyId, shapes, 1);
        b2ShapeId shapeId = shapes[0];

        REQUIRE(b2Shape_IsSensor(shapeId) == true);

        // Cleanup
        PhysicsBodyFactory::destroyBody(world, bodyId);
    }
}

TEST_CASE("PhysicsBodyFactory: No memory leaks on destruction", "[PhysicsBodyFactory]") {
    PhysicsWorld world;

    SECTION("Create and destroy many bodies rapidly") {
        // Rapid creation and destruction cycle
        for (int cycle = 0; cycle < 100; ++cycle) {
            TransformComponent transform{100.0f, 100.0f, 0.0f};
            RigidbodyComponent rigidbody = RigidbodyComponent::createDynamic(1.0f);
            ColliderComponent collider(32.0f, 32.0f);

            b2BodyId bodyId = PhysicsBodyFactory::createBody(world, transform, rigidbody, collider);
            REQUIRE(b2Body_IsValid(bodyId) == true);

            PhysicsBodyFactory::destroyBody(world, bodyId);
            REQUIRE(b2Body_IsValid(bodyId) == false);
        }

        // World should still be valid
        REQUIRE(world.isValid() == true);
    }
}

TEST_CASE("PhysicsBodyFactory: Polygon collider", "[PhysicsBodyFactory]") {
    PhysicsWorld world;

    SECTION("Create body with polygon collider (triangle)") {
        TransformComponent transform{0.0f, 0.0f, 0.0f};
        RigidbodyComponent rigidbody = RigidbodyComponent::createDynamic(1.0f);

        // Create triangle polygon
        std::vector<sf::Vector2f> vertices = {
            sf::Vector2f(0.0f, -16.0f),  // Top
            sf::Vector2f(-16.0f, 16.0f), // Bottom-left
            sf::Vector2f(16.0f, 16.0f)   // Bottom-right
        };
        ColliderComponent collider(vertices);

        b2BodyId bodyId = PhysicsBodyFactory::createBody(world, transform, rigidbody, collider);

        REQUIRE(b2Body_IsValid(bodyId) == true);

        // Verify shape was created
        int shapeCount = b2Body_GetShapeCount(bodyId);
        REQUIRE(shapeCount == 1);

        // Cleanup
        PhysicsBodyFactory::destroyBody(world, bodyId);
    }

    SECTION("Create body with polygon collider (hexagon)") {
        TransformComponent transform{100.0f, 100.0f, 0.0f};
        RigidbodyComponent rigidbody = RigidbodyComponent::createDynamic(1.0f);

        // Create hexagon (6 vertices)
        std::vector<sf::Vector2f> vertices = {
            sf::Vector2f(16.0f, 0.0f),
            sf::Vector2f(8.0f, 14.0f),
            sf::Vector2f(-8.0f, 14.0f),
            sf::Vector2f(-16.0f, 0.0f),
            sf::Vector2f(-8.0f, -14.0f),
            sf::Vector2f(8.0f, -14.0f)
        };
        ColliderComponent collider(vertices);

        b2BodyId bodyId = PhysicsBodyFactory::createBody(world, transform, rigidbody, collider);

        REQUIRE(b2Body_IsValid(bodyId) == true);

        // Verify shape was created
        int shapeCount = b2Body_GetShapeCount(bodyId);
        REQUIRE(shapeCount == 1);

        // Cleanup
        PhysicsBodyFactory::destroyBody(world, bodyId);
    }
}

TEST_CASE("PhysicsBodyFactory: Collider with offset", "[PhysicsBodyFactory]") {
    PhysicsWorld world;

    SECTION("Create box collider with offset") {
        TransformComponent transform{0.0f, 0.0f, 0.0f};
        RigidbodyComponent rigidbody = RigidbodyComponent::createDynamic(1.0f);
        ColliderComponent collider(32.0f, 32.0f);
        collider.offset = sf::Vector2f(16.0f, 16.0f); // Offset by half a tile

        b2BodyId bodyId = PhysicsBodyFactory::createBody(world, transform, rigidbody, collider);

        REQUIRE(b2Body_IsValid(bodyId) == true);

        // Verify shape was created (offset is baked into shape)
        int shapeCount = b2Body_GetShapeCount(bodyId);
        REQUIRE(shapeCount == 1);

        // Cleanup
        PhysicsBodyFactory::destroyBody(world, bodyId);
    }

    SECTION("Create circle collider with offset") {
        TransformComponent transform{100.0f, 100.0f, 0.0f};
        RigidbodyComponent rigidbody = RigidbodyComponent::createDynamic(1.0f);
        ColliderComponent collider(16.0f); // Circle with radius 16
        collider.offset = sf::Vector2f(10.0f, 10.0f);

        b2BodyId bodyId = PhysicsBodyFactory::createBody(world, transform, rigidbody, collider);

        REQUIRE(b2Body_IsValid(bodyId) == true);

        int shapeCount = b2Body_GetShapeCount(bodyId);
        REQUIRE(shapeCount == 1);

        // Cleanup
        PhysicsBodyFactory::destroyBody(world, bodyId);
    }
}
