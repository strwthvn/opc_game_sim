/**
 * @file test_collision_system.cpp
 * @brief Unit tests for CollisionSystem
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <core/systems/CollisionSystem.h>
#include <core/Components.h>
#include <entt/entt.hpp>

using namespace core;

TEST_CASE("CollisionComponent: Default constructor", "[CollisionComponent]") {
    CollisionComponent collision;

    REQUIRE(collision.isSolid == true);
    REQUIRE(collision.isTrigger == false);
    REQUIRE(collision.layer == "default");
    REQUIRE_THAT(collision.bounds.size.x, Catch::Matchers::WithinAbs(TILE_SIZE, 0.001f));
    REQUIRE_THAT(collision.bounds.size.y, Catch::Matchers::WithinAbs(TILE_SIZE, 0.001f));
}

TEST_CASE("CollisionComponent: Constructor with parameters", "[CollisionComponent]") {
    CollisionComponent collision(true, false, "player");

    REQUIRE(collision.isSolid == true);
    REQUIRE(collision.isTrigger == false);
    REQUIRE(collision.layer == "player");
}

TEST_CASE("CollisionComponent: getWorldBounds()", "[CollisionComponent]") {
    CollisionComponent collision;
    collision.bounds = sf::FloatRect(sf::Vector2f(5.0f, 10.0f), sf::Vector2f(32.0f, 32.0f));

    TransformComponent transform;
    transform.x = 100.0f;
    transform.y = 200.0f;

    sf::FloatRect worldBounds = collision.getWorldBounds(transform);

    REQUIRE_THAT(worldBounds.position.x, Catch::Matchers::WithinAbs(105.0f, 0.001f));
    REQUIRE_THAT(worldBounds.position.y, Catch::Matchers::WithinAbs(210.0f, 0.001f));
    REQUIRE_THAT(worldBounds.size.x, Catch::Matchers::WithinAbs(32.0f, 0.001f));
    REQUIRE_THAT(worldBounds.size.y, Catch::Matchers::WithinAbs(32.0f, 0.001f));
}

TEST_CASE("CollisionComponent: setFromTileSize()", "[CollisionComponent]") {
    CollisionComponent collision;

    collision.setFromTileSize(3, 2);

    REQUIRE_THAT(collision.bounds.size.x, Catch::Matchers::WithinAbs(96.0f, 0.001f));  // 3 * 32
    REQUIRE_THAT(collision.bounds.size.y, Catch::Matchers::WithinAbs(64.0f, 0.001f));  // 2 * 32
}

TEST_CASE("CollisionSystem: Basic AABB collision detection", "[CollisionSystem]") {
    entt::registry registry;
    CollisionSystem system;

    // Create two entities with overlapping collision boxes
    auto entity1 = registry.create();
    auto& transform1 = registry.emplace<TransformComponent>(entity1);
    transform1.x = 0.0f;
    transform1.y = 0.0f;

    auto& collision1 = registry.emplace<CollisionComponent>(entity1);
    collision1.bounds = sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(32.0f, 32.0f));
    collision1.isSolid = true;

    auto entity2 = registry.create();
    auto& transform2 = registry.emplace<TransformComponent>(entity2);
    transform2.x = 16.0f;  // Overlapping with entity1
    transform2.y = 16.0f;

    auto& collision2 = registry.emplace<CollisionComponent>(entity2);
    collision2.bounds = sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(32.0f, 32.0f));
    collision2.isSolid = true;

    bool collision1Enter = false;
    bool collision2Enter = false;

    collision1.onCollisionEnter = [&collision1Enter](entt::entity) { collision1Enter = true; };
    collision2.onCollisionEnter = [&collision2Enter](entt::entity) { collision2Enter = true; };

    // First update should trigger onCollisionEnter
    system.update(registry, 0.016);

    REQUIRE(collision1Enter == true);
    REQUIRE(collision2Enter == true);
}

TEST_CASE("CollisionSystem: No collision when separated", "[CollisionSystem]") {
    entt::registry registry;
    CollisionSystem system;

    auto entity1 = registry.create();
    auto& transform1 = registry.emplace<TransformComponent>(entity1);
    transform1.x = 0.0f;
    transform1.y = 0.0f;

    auto& collision1 = registry.emplace<CollisionComponent>(entity1);
    collision1.bounds = sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(32.0f, 32.0f));

    auto entity2 = registry.create();
    auto& transform2 = registry.emplace<TransformComponent>(entity2);
    transform2.x = 100.0f;  // Far away from entity1
    transform2.y = 100.0f;

    auto& collision2 = registry.emplace<CollisionComponent>(entity2);
    collision2.bounds = sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(32.0f, 32.0f));

    bool collision1Enter = false;
    bool collision2Enter = false;

    collision1.onCollisionEnter = [&collision1Enter](entt::entity) { collision1Enter = true; };
    collision2.onCollisionEnter = [&collision2Enter](entt::entity) { collision2Enter = true; };

    system.update(registry, 0.016);

    REQUIRE(collision1Enter == false);
    REQUIRE(collision2Enter == false);
}

TEST_CASE("CollisionSystem: Collision Enter/Stay/Exit events", "[CollisionSystem]") {
    entt::registry registry;
    CollisionSystem system;

    auto entity1 = registry.create();
    auto& transform1 = registry.emplace<TransformComponent>(entity1);
    transform1.x = 0.0f;
    transform1.y = 0.0f;

    auto& collision1 = registry.emplace<CollisionComponent>(entity1);
    collision1.bounds = sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(32.0f, 32.0f));

    auto entity2 = registry.create();
    auto& transform2 = registry.emplace<TransformComponent>(entity2);
    transform2.x = 16.0f;
    transform2.y = 16.0f;

    auto& collision2 = registry.emplace<CollisionComponent>(entity2);
    collision2.bounds = sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(32.0f, 32.0f));

    int enterCount = 0;
    int stayCount = 0;
    int exitCount = 0;

    collision1.onCollisionEnter = [&enterCount](entt::entity) { enterCount++; };
    collision1.onCollisionStay = [&stayCount](entt::entity) { stayCount++; };
    collision1.onCollisionExit = [&exitCount](entt::entity) { exitCount++; };

    // First update - should trigger Enter
    system.update(registry, 0.016);
    REQUIRE(enterCount == 1);
    REQUIRE(stayCount == 0);
    REQUIRE(exitCount == 0);

    // Second update - should trigger Stay
    system.update(registry, 0.016);
    REQUIRE(enterCount == 1);
    REQUIRE(stayCount == 1);
    REQUIRE(exitCount == 0);

    // Move entity2 away - should trigger Exit
    transform2.x = 100.0f;
    transform2.y = 100.0f;
    system.update(registry, 0.016);
    REQUIRE(enterCount == 1);
    REQUIRE(stayCount == 1);
    REQUIRE(exitCount == 1);
}

TEST_CASE("CollisionSystem: Trigger collision (non-solid)", "[CollisionSystem]") {
    entt::registry registry;
    CollisionSystem system;

    auto entity1 = registry.create();
    auto& transform1 = registry.emplace<TransformComponent>(entity1);
    transform1.x = 0.0f;
    transform1.y = 0.0f;

    auto& collision1 = registry.emplace<CollisionComponent>(entity1);
    collision1.bounds = sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(32.0f, 32.0f));
    collision1.isSolid = false;
    collision1.isTrigger = true;

    auto entity2 = registry.create();
    auto& transform2 = registry.emplace<TransformComponent>(entity2);
    transform2.x = 16.0f;
    transform2.y = 16.0f;

    auto& collision2 = registry.emplace<CollisionComponent>(entity2);
    collision2.bounds = sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(32.0f, 32.0f));

    bool triggerEntered = false;

    collision1.onCollisionEnter = [&triggerEntered](entt::entity) { triggerEntered = true; };

    system.update(registry, 0.016);

    // Trigger should still detect collision even though it's not solid
    REQUIRE(triggerEntered == true);
}

TEST_CASE("CollisionSystem: Multiple simultaneous collisions", "[CollisionSystem]") {
    entt::registry registry;
    CollisionSystem system;

    // Create a central entity
    auto centerEntity = registry.create();
    auto& centerTransform = registry.emplace<TransformComponent>(centerEntity);
    centerTransform.x = 50.0f;
    centerTransform.y = 50.0f;

    auto& centerCollision = registry.emplace<CollisionComponent>(centerEntity);
    centerCollision.bounds = sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(32.0f, 32.0f));

    int collisionCount = 0;
    centerCollision.onCollisionEnter = [&collisionCount](entt::entity) { collisionCount++; };

    // Create three entities overlapping with the center
    for (int i = 0; i < 3; ++i) {
        auto entity = registry.create();
        auto& transform = registry.emplace<TransformComponent>(entity);
        transform.x = 50.0f + i * 10.0f;
        transform.y = 50.0f + i * 10.0f;

        auto& collision = registry.emplace<CollisionComponent>(entity);
        collision.bounds = sf::FloatRect(sf::Vector2f(0.0f, 0.0f), sf::Vector2f(32.0f, 32.0f));
    }

    system.update(registry, 0.016);

    // Center entity should collide with all 3 entities
    REQUIRE(collisionCount == 3);
}

TEST_CASE("CollisionSystem: System properties", "[CollisionSystem]") {
    CollisionSystem system;

    SECTION("System priority") {
        REQUIRE(system.getPriority() == 100);
    }

    SECTION("System name") {
        REQUIRE(std::string(system.getName()) == "CollisionSystem");
    }
}
