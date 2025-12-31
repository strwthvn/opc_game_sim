/**
 * @file test_collision_events.cpp
 * @brief Unit tests for CollisionEvents and PhysicsEventProcessor
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <simulation/PhysicsEventProcessor.h>
#include <simulation/systems/PhysicsSystem.h>
#include <simulation/PhysicsWorld.h>
#include <simulation/PhysicsComponents.h>
#include <core/Components.h>
#include <entt/entt.hpp>
#include <vector>

using namespace simulation;
using namespace core;

// Helper function to compare entity with null to avoid Catch2/EnTT operator ambiguity
static bool isNullEntity(entt::entity e) {
    return e == entt::null;
}

TEST_CASE("CollisionEvents: Event structures", "[CollisionEvents]") {
    SECTION("CollisionBeginEvent default values") {
        CollisionBeginEvent event;
        REQUIRE(isNullEntity(event.entityA));
        REQUIRE(isNullEntity(event.entityB));
        REQUIRE_FALSE(event.isValid());
    }

    SECTION("CollisionEndEvent default values") {
        CollisionEndEvent event;
        REQUIRE(isNullEntity(event.entityA));
        REQUIRE(isNullEntity(event.entityB));
        REQUIRE_FALSE(event.isValid());
    }

    SECTION("TriggerEnterEvent default values") {
        TriggerEnterEvent event;
        REQUIRE(isNullEntity(event.triggerEntity));
        REQUIRE(isNullEntity(event.otherEntity));
        REQUIRE_FALSE(event.isValid());
    }

    SECTION("TriggerExitEvent default values") {
        TriggerExitEvent event;
        REQUIRE(isNullEntity(event.triggerEntity));
        REQUIRE(isNullEntity(event.otherEntity));
        REQUIRE_FALSE(event.isValid());
    }

    SECTION("CollisionHitEvent default values") {
        CollisionHitEvent event;
        REQUIRE(isNullEntity(event.entityA));
        REQUIRE(isNullEntity(event.entityB));
        REQUIRE(event.approachSpeed == 0.0f);
        REQUIRE_FALSE(event.isValid());
    }
}

TEST_CASE("CollisionSignals: Signal subscription and emission", "[CollisionEvents]") {
    CollisionSignals signals;

    SECTION("OnCollisionBegin signal") {
        int callCount = 0;
        entt::entity receivedA = entt::null;
        entt::entity receivedB = entt::null;

        auto connection = signals.onCollisionBegin.connect(
            [&](const CollisionBeginEvent& event) {
                callCount++;
                receivedA = event.entityA;
                receivedB = event.entityB;
            });

        CollisionBeginEvent event;
        event.entityA = static_cast<entt::entity>(1);
        event.entityB = static_cast<entt::entity>(2);

        signals.onCollisionBegin(event);

        REQUIRE(callCount == 1);
        REQUIRE(receivedA == static_cast<entt::entity>(1));
        REQUIRE(receivedB == static_cast<entt::entity>(2));
    }

    SECTION("OnTriggerEnter signal") {
        int callCount = 0;

        auto connection = signals.onTriggerEnter.connect(
            [&](const TriggerEnterEvent& event) {
                callCount++;
            });

        TriggerEnterEvent event;
        event.triggerEntity = static_cast<entt::entity>(10);
        event.otherEntity = static_cast<entt::entity>(20);

        signals.onTriggerEnter(event);

        REQUIRE(callCount == 1);
    }

    SECTION("DisconnectAll clears all slots") {
        int beginCount = 0;
        int endCount = 0;

        signals.onCollisionBegin.connect([&](const CollisionBeginEvent&) { beginCount++; });
        signals.onCollisionEnd.connect([&](const CollisionEndEvent&) { endCount++; });

        signals.disconnectAll();

        CollisionBeginEvent beginEvent;
        CollisionEndEvent endEvent;

        signals.onCollisionBegin(beginEvent);
        signals.onCollisionEnd(endEvent);

        REQUIRE(beginCount == 0);
        REQUIRE(endCount == 0);
    }
}

TEST_CASE("PhysicsEventProcessor: Basic functionality", "[PhysicsEventProcessor]") {
    entt::registry registry;
    PhysicsWorld world(b2Vec2{0.0f, 9.8f});
    PhysicsEventProcessor processor(world, registry);

    SECTION("Processor creation and destruction") {
        // Just verify it doesn't crash
        REQUIRE(processor.getHitSpeedThreshold() == PhysicsEventProcessor::HIT_SPEED_THRESHOLD);
    }

    SECTION("Set hit speed threshold") {
        processor.setHitSpeedThreshold(2.5f);
        REQUIRE_THAT(processor.getHitSpeedThreshold(),
                     Catch::Matchers::WithinAbs(2.5f, 0.001f));
    }

    SECTION("Process events without bodies doesn't crash") {
        // Process events with empty world
        processor.processEvents();
        REQUIRE(true);
    }
}

TEST_CASE("PhysicsEventProcessor: Collision detection", "[PhysicsEventProcessor]") {
    entt::registry registry;
    PhysicsWorld world(b2Vec2{0.0f, 9.8f});
    PhysicsSystem physicsSystem(world);
    PhysicsEventProcessor processor(world, registry);

    // Track events
    std::vector<CollisionBeginEvent> beginEvents;
    std::vector<CollisionEndEvent> endEvents;

    processor.getSignals().onCollisionBegin.connect(
        [&](const CollisionBeginEvent& e) { beginEvents.push_back(e); });
    processor.getSignals().onCollisionEnd.connect(
        [&](const CollisionEndEvent& e) { endEvents.push_back(e); });

    SECTION("Two dynamic bodies collision generates BeginContact event") {
        // Create falling body
        auto fallingEntity = registry.create();
        auto& fallingTransform = registry.emplace<TransformComponent>(fallingEntity);
        fallingTransform.x = 100.0f;
        fallingTransform.y = 50.0f;

        auto& fallingRb = registry.emplace<RigidbodyComponent>(fallingEntity);
        fallingRb.bodyType = RigidbodyComponent::BodyType::Dynamic;

        auto& fallingCollider = registry.emplace<ColliderComponent>(fallingEntity);
        fallingCollider.shape = ColliderComponent::Shape::Box;
        fallingCollider.size = sf::Vector2f(32.0f, 32.0f);

        // Create ground
        auto groundEntity = registry.create();
        auto& groundTransform = registry.emplace<TransformComponent>(groundEntity);
        groundTransform.x = 50.0f;
        groundTransform.y = 200.0f;

        auto& groundRb = registry.emplace<RigidbodyComponent>(groundEntity);
        groundRb.bodyType = RigidbodyComponent::BodyType::Static;

        auto& groundCollider = registry.emplace<ColliderComponent>(groundEntity);
        groundCollider.shape = ColliderComponent::Shape::Box;
        groundCollider.size = sf::Vector2f(200.0f, 32.0f);

        physicsSystem.init(registry);

        // Simulate until collision
        for (int i = 0; i < 120; ++i) {
            physicsSystem.update(registry, PhysicsSystem::FIXED_TIMESTEP);
            processor.processEvents();
        }

        // Should have at least one begin event
        REQUIRE(beginEvents.size() >= 1);

        // Verify event has valid entities
        bool foundValidEvent = false;
        for (const auto& event : beginEvents) {
            if (event.isValid()) {
                foundValidEvent = true;
                // One of the entities should be falling, one should be ground
                bool hasFalling = (event.entityA == fallingEntity || event.entityB == fallingEntity);
                bool hasGround = (event.entityA == groundEntity || event.entityB == groundEntity);
                REQUIRE((hasFalling && hasGround));
            }
        }
        REQUIRE(foundValidEvent);
    }
}

TEST_CASE("PhysicsEventProcessor: Trigger (sensor) events", "[PhysicsEventProcessor]") {
    entt::registry registry;
    PhysicsWorld world(b2Vec2{0.0f, 0.0f}); // No gravity for predictable movement
    PhysicsSystem physicsSystem(world);
    PhysicsEventProcessor processor(world, registry);

    std::vector<TriggerEnterEvent> enterEvents;
    std::vector<TriggerExitEvent> exitEvents;

    processor.getSignals().onTriggerEnter.connect(
        [&](const TriggerEnterEvent& e) { enterEvents.push_back(e); });
    processor.getSignals().onTriggerExit.connect(
        [&](const TriggerExitEvent& e) { exitEvents.push_back(e); });

    SECTION("TriggerEnterEvent generated when body enters sensor") {
        // Create sensor (trigger zone)
        auto triggerEntity = registry.create();
        auto& triggerTransform = registry.emplace<TransformComponent>(triggerEntity);
        triggerTransform.x = 100.0f;
        triggerTransform.y = 100.0f;

        auto& triggerRb = registry.emplace<RigidbodyComponent>(triggerEntity);
        triggerRb.bodyType = RigidbodyComponent::BodyType::Static;

        auto& triggerCollider = registry.emplace<ColliderComponent>(triggerEntity);
        triggerCollider.shape = ColliderComponent::Shape::Box;
        triggerCollider.size = sf::Vector2f(64.0f, 64.0f);
        triggerCollider.isSensor = true;

        // Create moving body
        auto moverEntity = registry.create();
        auto& moverTransform = registry.emplace<TransformComponent>(moverEntity);
        moverTransform.x = 0.0f;
        moverTransform.y = 100.0f;

        auto& moverRb = registry.emplace<RigidbodyComponent>(moverEntity);
        moverRb.bodyType = RigidbodyComponent::BodyType::Dynamic;
        moverRb.linearVelocity = sf::Vector2f(100.0f, 0.0f); // Move right

        auto& moverCollider = registry.emplace<ColliderComponent>(moverEntity);
        moverCollider.shape = ColliderComponent::Shape::Box;
        moverCollider.size = sf::Vector2f(32.0f, 32.0f);

        physicsSystem.init(registry);

        // Apply velocity to Box2D body
        b2Vec2 velocity = PhysicsWorld::pixelsToMeters(b2Vec2{100.0f, 0.0f});
        b2Body_SetLinearVelocity(moverRb.box2dBodyId, velocity);

        // Simulate until body enters trigger zone
        for (int i = 0; i < 120; ++i) {
            physicsSystem.update(registry, PhysicsSystem::FIXED_TIMESTEP);
            processor.processEvents();
        }

        // Should have enter events
        REQUIRE(enterEvents.size() >= 1);

        // Verify event
        bool foundValidEvent = false;
        for (const auto& event : enterEvents) {
            if (event.isValid()) {
                foundValidEvent = true;
                REQUIRE(event.triggerEntity == triggerEntity);
                REQUIRE(event.otherEntity == moverEntity);
            }
        }
        REQUIRE(foundValidEvent);
    }
}

TEST_CASE("ColliderComponent: Collision filtering", "[ColliderComponent]") {
    entt::registry registry;
    PhysicsWorld world(b2Vec2{0.0f, 9.8f});
    PhysicsSystem physicsSystem(world);
    PhysicsEventProcessor processor(world, registry);

    std::vector<CollisionBeginEvent> beginEvents;
    processor.getSignals().onCollisionBegin.connect(
        [&](const CollisionBeginEvent& e) { beginEvents.push_back(e); });

    SECTION("Bodies with non-overlapping masks don't collide") {
        // Create two bodies with different categories that don't match masks
        auto entity1 = registry.create();
        auto& transform1 = registry.emplace<TransformComponent>(entity1);
        transform1.x = 100.0f;
        transform1.y = 0.0f;

        auto& rb1 = registry.emplace<RigidbodyComponent>(entity1);
        rb1.bodyType = RigidbodyComponent::BodyType::Dynamic;

        auto& collider1 = registry.emplace<ColliderComponent>(entity1);
        collider1.shape = ColliderComponent::Shape::Box;
        collider1.size = sf::Vector2f(32.0f, 32.0f);
        collider1.categoryBits = 0x0001; // Category A
        collider1.maskBits = 0x0002;     // Only collides with category B

        // Second body - static platform
        auto entity2 = registry.create();
        auto& transform2 = registry.emplace<TransformComponent>(entity2);
        transform2.x = 50.0f;
        transform2.y = 100.0f;

        auto& rb2 = registry.emplace<RigidbodyComponent>(entity2);
        rb2.bodyType = RigidbodyComponent::BodyType::Static;

        auto& collider2 = registry.emplace<ColliderComponent>(entity2);
        collider2.shape = ColliderComponent::Shape::Box;
        collider2.size = sf::Vector2f(200.0f, 32.0f);
        collider2.categoryBits = 0x0004; // Category C (not B!)
        collider2.maskBits = 0xFFFF;     // Collides with all

        physicsSystem.init(registry);

        // Simulate
        for (int i = 0; i < 120; ++i) {
            physicsSystem.update(registry, PhysicsSystem::FIXED_TIMESTEP);
            processor.processEvents();
        }

        // No collision events should be generated between these two
        bool collisionBetweenEntities = false;
        for (const auto& event : beginEvents) {
            if ((event.entityA == entity1 && event.entityB == entity2) ||
                (event.entityA == entity2 && event.entityB == entity1)) {
                collisionBetweenEntities = true;
            }
        }

        REQUIRE_FALSE(collisionBetweenEntities);
    }

    SECTION("Bodies with matching masks do collide") {
        beginEvents.clear();

        auto entity1 = registry.create();
        auto& transform1 = registry.emplace<TransformComponent>(entity1);
        transform1.x = 100.0f;
        transform1.y = 0.0f;

        auto& rb1 = registry.emplace<RigidbodyComponent>(entity1);
        rb1.bodyType = RigidbodyComponent::BodyType::Dynamic;

        auto& collider1 = registry.emplace<ColliderComponent>(entity1);
        collider1.shape = ColliderComponent::Shape::Box;
        collider1.size = sf::Vector2f(32.0f, 32.0f);
        collider1.categoryBits = 0x0001;
        collider1.maskBits = 0x0002; // Collides with category B

        auto entity2 = registry.create();
        auto& transform2 = registry.emplace<TransformComponent>(entity2);
        transform2.x = 50.0f;
        transform2.y = 100.0f;

        auto& rb2 = registry.emplace<RigidbodyComponent>(entity2);
        rb2.bodyType = RigidbodyComponent::BodyType::Static;

        auto& collider2 = registry.emplace<ColliderComponent>(entity2);
        collider2.shape = ColliderComponent::Shape::Box;
        collider2.size = sf::Vector2f(200.0f, 32.0f);
        collider2.categoryBits = 0x0002; // Category B - matches!
        collider2.maskBits = 0xFFFF;

        physicsSystem.init(registry);

        for (int i = 0; i < 120; ++i) {
            physicsSystem.update(registry, PhysicsSystem::FIXED_TIMESTEP);
            processor.processEvents();
        }

        // Should have collision events
        bool collisionBetweenEntities = false;
        for (const auto& event : beginEvents) {
            if ((event.entityA == entity1 && event.entityB == entity2) ||
                (event.entityA == entity2 && event.entityB == entity1)) {
                collisionBetweenEntities = true;
            }
        }

        REQUIRE(collisionBetweenEntities);
    }
}

TEST_CASE("PhysicsEventProcessor: Integration with ground collision", "[PhysicsEventProcessor]") {
    entt::registry registry;
    PhysicsWorld world(b2Vec2{0.0f, 9.8f});
    PhysicsSystem physicsSystem(world);
    PhysicsEventProcessor processor(world, registry);

    std::vector<CollisionBeginEvent> beginEvents;
    processor.getSignals().onCollisionBegin.connect(
        [&](const CollisionBeginEvent& e) { beginEvents.push_back(e); });

    SECTION("Object falls on platform and generates collision event") {
        // Falling object
        auto fallingEntity = registry.create();
        auto& fallingTransform = registry.emplace<TransformComponent>(fallingEntity);
        fallingTransform.x = 100.0f;
        fallingTransform.y = 100.0f;

        auto& fallingRb = registry.emplace<RigidbodyComponent>(fallingEntity);
        fallingRb.bodyType = RigidbodyComponent::BodyType::Dynamic;

        auto& fallingCollider = registry.emplace<ColliderComponent>(fallingEntity);
        fallingCollider.shape = ColliderComponent::Shape::Box;
        fallingCollider.size = sf::Vector2f(32.0f, 32.0f);

        // Ground platform
        auto groundEntity = registry.create();
        auto& groundTransform = registry.emplace<TransformComponent>(groundEntity);
        groundTransform.x = 0.0f;
        groundTransform.y = 300.0f;

        auto& groundRb = registry.emplace<RigidbodyComponent>(groundEntity);
        groundRb.bodyType = RigidbodyComponent::BodyType::Static;

        auto& groundCollider = registry.emplace<ColliderComponent>(groundEntity);
        groundCollider.shape = ColliderComponent::Shape::Box;
        groundCollider.size = sf::Vector2f(640.0f, 32.0f);

        physicsSystem.init(registry);

        float initialY = fallingTransform.y;

        // Simulate physics
        for (int i = 0; i < 180; ++i) {
            physicsSystem.update(registry, PhysicsSystem::FIXED_TIMESTEP);
            processor.processEvents();
        }

        // Verify falling object has moved down
        REQUIRE(fallingTransform.y > initialY);

        // Verify collision event was generated
        REQUIRE(beginEvents.size() >= 1);
    }
}
