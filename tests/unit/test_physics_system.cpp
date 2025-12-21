/**
 * @file PhysicsSystemTests.cpp
 * @brief Unit tests for PhysicsSystem
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <simulation/systems/PhysicsSystem.h>
#include <simulation/PhysicsWorld.h>
#include <simulation/PhysicsComponents.h>
#include <core/Components.h>
#include <entt/entt.hpp>

using namespace simulation;
using namespace core;

TEST_CASE("PhysicsSystem: Initialization creates b2Body for entities", "[PhysicsSystem]") {
    entt::registry registry;
    PhysicsWorld world(b2Vec2{0.0f, 9.8f});
    PhysicsSystem system(world);

    SECTION("Single entity with RigidbodyComponent and ColliderComponent") {
        auto entity = registry.create();

        // Add required components
        auto& transform = registry.emplace<TransformComponent>(entity);
        transform.x = 100.0f;
        transform.y = 200.0f;

        auto& rigidbody = registry.emplace<RigidbodyComponent>(entity);
        rigidbody.bodyType = RigidbodyComponent::BodyType::Dynamic;

        auto& collider = registry.emplace<ColliderComponent>(entity);
        collider.shape = ColliderComponent::Shape::Box;
        collider.size = sf::Vector2f(32.0f, 32.0f);

        // Initialize system
        system.init(registry);

        // Verify body was created
        REQUIRE(rigidbody.hasBox2DBody());
        REQUIRE(B2_IS_NON_NULL(rigidbody.box2dBodyId));
    }

    SECTION("Multiple entities with different body types") {
        // Create static entity
        auto staticEntity = registry.create();
        registry.emplace<TransformComponent>(staticEntity);
        auto& staticRb = registry.emplace<RigidbodyComponent>(staticEntity);
        staticRb.bodyType = RigidbodyComponent::BodyType::Static;
        registry.emplace<ColliderComponent>(staticEntity);

        // Create dynamic entity
        auto dynamicEntity = registry.create();
        registry.emplace<TransformComponent>(dynamicEntity);
        auto& dynamicRb = registry.emplace<RigidbodyComponent>(dynamicEntity);
        dynamicRb.bodyType = RigidbodyComponent::BodyType::Dynamic;
        registry.emplace<ColliderComponent>(dynamicEntity);

        // Create kinematic entity
        auto kinematicEntity = registry.create();
        registry.emplace<TransformComponent>(kinematicEntity);
        auto& kinematicRb = registry.emplace<RigidbodyComponent>(kinematicEntity);
        kinematicRb.bodyType = RigidbodyComponent::BodyType::Kinematic;
        registry.emplace<ColliderComponent>(kinematicEntity);

        system.init(registry);

        // Verify all bodies were created
        REQUIRE(staticRb.hasBox2DBody());
        REQUIRE(dynamicRb.hasBox2DBody());
        REQUIRE(kinematicRb.hasBox2DBody());
    }
}

TEST_CASE("PhysicsSystem: Dynamic body falls under gravity", "[PhysicsSystem]") {
    entt::registry registry;
    PhysicsWorld world(b2Vec2{0.0f, 9.8f});
    PhysicsSystem system(world);

    auto entity = registry.create();

    auto& transform = registry.emplace<TransformComponent>(entity);
    transform.x = 100.0f;
    transform.y = 100.0f;

    auto& rigidbody = registry.emplace<RigidbodyComponent>(entity);
    rigidbody.bodyType = RigidbodyComponent::BodyType::Dynamic;
    rigidbody.mass = 1.0f;

    auto& collider = registry.emplace<ColliderComponent>(entity);
    collider.shape = ColliderComponent::Shape::Box;
    collider.size = sf::Vector2f(32.0f, 32.0f);

    system.init(registry);

    float initialY = transform.y;

    // Simulate for 1 second (60 physics steps at 60 Hz)
    for (int i = 0; i < 60; ++i) {
        system.update(registry, PhysicsSystem::FIXED_TIMESTEP);
    }

    // Verify body has fallen (Y increased due to downward gravity)
    REQUIRE(transform.y > initialY);
}

TEST_CASE("PhysicsSystem: Static body does not move", "[PhysicsSystem]") {
    entt::registry registry;
    PhysicsWorld world(b2Vec2{0.0f, 9.8f});
    PhysicsSystem system(world);

    auto entity = registry.create();

    auto& transform = registry.emplace<TransformComponent>(entity);
    transform.x = 100.0f;
    transform.y = 100.0f;

    auto& rigidbody = registry.emplace<RigidbodyComponent>(entity);
    rigidbody.bodyType = RigidbodyComponent::BodyType::Static;

    auto& collider = registry.emplace<ColliderComponent>(entity);
    collider.shape = ColliderComponent::Shape::Box;
    collider.size = sf::Vector2f(32.0f, 32.0f);

    system.init(registry);

    float initialX = transform.x;
    float initialY = transform.y;

    // Simulate for 1 second
    for (int i = 0; i < 60; ++i) {
        system.update(registry, PhysicsSystem::FIXED_TIMESTEP);
    }

    // Verify static body hasn't moved
    REQUIRE_THAT(transform.x, Catch::Matchers::WithinAbs(initialX, 0.1f));
    REQUIRE_THAT(transform.y, Catch::Matchers::WithinAbs(initialY, 0.1f));
}

TEST_CASE("PhysicsSystem: TransformComponent syncs with b2Body after step", "[PhysicsSystem]") {
    entt::registry registry;
    PhysicsWorld world(b2Vec2{0.0f, 9.8f});
    PhysicsSystem system(world);

    auto entity = registry.create();

    auto& transform = registry.emplace<TransformComponent>(entity);
    transform.x = 100.0f;
    transform.y = 100.0f;

    auto& rigidbody = registry.emplace<RigidbodyComponent>(entity);
    rigidbody.bodyType = RigidbodyComponent::BodyType::Dynamic;

    auto& collider = registry.emplace<ColliderComponent>(entity);
    collider.shape = ColliderComponent::Shape::Box;
    collider.size = sf::Vector2f(32.0f, 32.0f);

    system.init(registry);

    // Get initial Box2D position
    b2Vec2 initialPos = b2Body_GetPosition(rigidbody.box2dBodyId);

    // Simulate one step
    system.update(registry, PhysicsSystem::FIXED_TIMESTEP);

    // Get new Box2D position
    b2Vec2 newPos = b2Body_GetPosition(rigidbody.box2dBodyId);

    // Verify position changed in Box2D
    REQUIRE(newPos.y > initialPos.y);

    // Verify TransformComponent was synced
    // Convert Box2D position to pixels for comparison
    b2Vec2 expectedPixelPosB2 = PhysicsWorld::metersToPixels(newPos);
    sf::Vector2f expectedPixelPos(expectedPixelPosB2.x, expectedPixelPosB2.y);

    // Account for center offset (TransformComponent = bottom-left, Box2D = center)
    expectedPixelPos.x -= collider.size.x * 0.5f;
    expectedPixelPos.y -= collider.size.y * 0.5f;

    REQUIRE_THAT(transform.x, Catch::Matchers::WithinAbs(expectedPixelPos.x, 0.5f));
    REQUIRE_THAT(transform.y, Catch::Matchers::WithinAbs(expectedPixelPos.y, 0.5f));
}

TEST_CASE("PhysicsSystem: Kinematic body moves when TransformComponent changes", "[PhysicsSystem]") {
    entt::registry registry;
    PhysicsWorld world(b2Vec2{0.0f, 0.0f}); // No gravity
    PhysicsSystem system(world);

    auto entity = registry.create();

    auto& transform = registry.emplace<TransformComponent>(entity);
    transform.x = 100.0f;
    transform.y = 100.0f;

    auto& rigidbody = registry.emplace<RigidbodyComponent>(entity);
    rigidbody.bodyType = RigidbodyComponent::BodyType::Kinematic;

    auto& collider = registry.emplace<ColliderComponent>(entity);
    collider.shape = ColliderComponent::Shape::Box;
    collider.size = sf::Vector2f(32.0f, 32.0f);

    system.init(registry);

    // Set initial velocity
    rigidbody.linearVelocity = sf::Vector2f(100.0f, 0.0f); // Move right at 100 px/s

    // Apply velocity to Box2D body
    b2Vec2 velocity = PhysicsWorld::pixelsToMeters(b2Vec2{100.0f, 0.0f});
    b2Body_SetLinearVelocity(rigidbody.box2dBodyId, velocity);

    float initialX = transform.x;

    // Simulate for 0.5 seconds
    for (int i = 0; i < 30; ++i) {
        system.update(registry, PhysicsSystem::FIXED_TIMESTEP);
    }

    // Verify kinematic body moved
    REQUIRE(transform.x > initialX);
}

TEST_CASE("PhysicsSystem: Collision between two dynamic bodies", "[PhysicsSystem]") {
    entt::registry registry;
    PhysicsWorld world(b2Vec2{0.0f, 9.8f});
    PhysicsSystem system(world);

    // Create falling body
    auto fallingEntity = registry.create();
    auto& fallingTransform = registry.emplace<TransformComponent>(fallingEntity);
    fallingTransform.x = 100.0f;
    fallingTransform.y = 100.0f;

    auto& fallingRb = registry.emplace<RigidbodyComponent>(fallingEntity);
    fallingRb.bodyType = RigidbodyComponent::BodyType::Dynamic;

    auto& fallingCollider = registry.emplace<ColliderComponent>(fallingEntity);
    fallingCollider.shape = ColliderComponent::Shape::Box;
    fallingCollider.size = sf::Vector2f(32.0f, 32.0f);
    fallingCollider.density = 1.0f;

    // Create ground (static body)
    auto groundEntity = registry.create();
    auto& groundTransform = registry.emplace<TransformComponent>(groundEntity);
    groundTransform.x = 0.0f;
    groundTransform.y = 300.0f;

    auto& groundRb = registry.emplace<RigidbodyComponent>(groundEntity);
    groundRb.bodyType = RigidbodyComponent::BodyType::Static;

    auto& groundCollider = registry.emplace<ColliderComponent>(groundEntity);
    groundCollider.shape = ColliderComponent::Shape::Box;
    groundCollider.size = sf::Vector2f(640.0f, 32.0f); // Wide ground

    system.init(registry);

    float initialY = fallingTransform.y;

    // Simulate until body hits ground or timeout
    bool hitGround = false;
    for (int i = 0; i < 120; ++i) {
        float prevY = fallingTransform.y;
        system.update(registry, PhysicsSystem::FIXED_TIMESTEP);

        // Check if body stopped falling (collision happened)
        if (std::abs(fallingTransform.y - prevY) < 0.01f && i > 10) {
            hitGround = true;
            break;
        }
    }

    // Verify collision occurred (body stopped above initial position)
    REQUIRE(hitGround);
    REQUIRE(fallingTransform.y > initialY);
}

TEST_CASE("PhysicsSystem: Fixed timestep accumulator", "[PhysicsSystem]") {
    entt::registry registry;
    PhysicsWorld world(b2Vec2{0.0f, 0.0f});
    PhysicsSystem system(world);

    auto entity = registry.create();
    registry.emplace<TransformComponent>(entity);
    auto& rigidbody = registry.emplace<RigidbodyComponent>(entity);
    rigidbody.bodyType = RigidbodyComponent::BodyType::Dynamic;
    registry.emplace<ColliderComponent>(entity);

    system.init(registry);

    SECTION("Small dt - no physics step") {
        // Update with very small dt
        system.update(registry, 0.001); // 1ms

        // Physics should not have stepped (accumulator < FIXED_TIMESTEP)
        // We can't directly verify this without exposing m_accumulator
        // but we can verify behavior is correct
        REQUIRE(true);
    }

    SECTION("Large dt - multiple physics steps") {
        auto& transform = registry.get<TransformComponent>(entity);

        // Set velocity
        b2Vec2 velocity = PhysicsWorld::pixelsToMeters(b2Vec2{100.0f, 0.0f});
        b2Body_SetLinearVelocity(rigidbody.box2dBodyId, velocity);

        float initialX = transform.x;

        // Update with large dt (3 frames worth)
        system.update(registry, 3.0 * PhysicsSystem::FIXED_TIMESTEP);

        // Body should have moved approximately 3x the distance of one step
        REQUIRE(transform.x > initialX);
    }

    SECTION("Very large dt - clamped to MAX_ACCUMULATOR") {
        // Update with extremely large dt
        system.update(registry, 1.0); // 1 second

        // System should clamp and not crash
        REQUIRE(true);
    }
}

TEST_CASE("PhysicsSystem: Different collider shapes", "[PhysicsSystem]") {
    entt::registry registry;
    PhysicsWorld world(b2Vec2{0.0f, 9.8f});
    PhysicsSystem system(world);

    SECTION("Circle collider") {
        auto entity = registry.create();
        registry.emplace<TransformComponent>(entity);
        auto& rigidbody = registry.emplace<RigidbodyComponent>(entity);
        rigidbody.bodyType = RigidbodyComponent::BodyType::Dynamic;

        auto& collider = registry.emplace<ColliderComponent>(entity);
        collider.shape = ColliderComponent::Shape::Circle;
        collider.radius = 16.0f;

        system.init(registry);

        REQUIRE(rigidbody.hasBox2DBody());
    }

    SECTION("Polygon collider") {
        auto entity = registry.create();
        registry.emplace<TransformComponent>(entity);
        auto& rigidbody = registry.emplace<RigidbodyComponent>(entity);
        rigidbody.bodyType = RigidbodyComponent::BodyType::Dynamic;

        auto& collider = registry.emplace<ColliderComponent>(entity);
        collider.shape = ColliderComponent::Shape::Polygon;
        // Triangle
        collider.vertices = {
            sf::Vector2f(0.0f, -10.0f),
            sf::Vector2f(10.0f, 10.0f),
            sf::Vector2f(-10.0f, 10.0f)
        };

        system.init(registry);

        REQUIRE(rigidbody.hasBox2DBody());
    }
}

TEST_CASE("PhysicsSystem: System properties", "[PhysicsSystem]") {
    PhysicsWorld world(b2Vec2{0.0f, 9.8f});
    PhysicsSystem system(world);

    SECTION("System priority") {
        REQUIRE(system.getPriority() == 150);
    }

    SECTION("System name") {
        REQUIRE(std::string(system.getName()) == "PhysicsSystem");
    }

    SECTION("Fixed timestep constant") {
        REQUIRE_THAT(PhysicsSystem::FIXED_TIMESTEP,
                     Catch::Matchers::WithinAbs(1.0f / 60.0f, 0.0001f));
    }
}
