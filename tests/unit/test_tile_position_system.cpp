/**
 * @file test_tile_position_system.cpp
 * @brief Unit tests for TilePositionSystem
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <core/systems/TilePositionSystem.h>
#include <core/Components.h>
#include <entt/entt.hpp>

using namespace core;

TEST_CASE("TilePositionSystem: Synchronization of tile to pixel coordinates", "[TilePositionSystem]") {
    entt::registry registry;
    TilePositionSystem system;

    SECTION("Single entity synchronization") {
        auto entity = registry.create();

        auto& tilePos = registry.emplace<TilePositionComponent>(entity);
        tilePos.tileX = 3;
        tilePos.tileY = 6;
        tilePos.widthTiles = 1;
        tilePos.heightTiles = 1;
        tilePos.autoSync = true;

        auto& transform = registry.emplace<TransformComponent>(entity);
        auto& sprite = registry.emplace<SpriteComponent>(entity);

        system.update(registry);

        // Verify TransformComponent was synced
        REQUIRE_THAT(transform.x, Catch::Matchers::WithinAbs(96.0f, 0.001f));  // 3 * 32
        REQUIRE_THAT(transform.y, Catch::Matchers::WithinAbs(224.0f, 0.001f)); // (6+1) * 32
    }

    SECTION("Multiple entities synchronization") {
        auto entity1 = registry.create();
        auto& tilePos1 = registry.emplace<TilePositionComponent>(entity1);
        tilePos1.tileX = 2;
        tilePos1.tileY = 3;
        registry.emplace<TransformComponent>(entity1);
        registry.emplace<SpriteComponent>(entity1);

        auto entity2 = registry.create();
        auto& tilePos2 = registry.emplace<TilePositionComponent>(entity2);
        tilePos2.tileX = 5;
        tilePos2.tileY = 7;
        registry.emplace<TransformComponent>(entity2);
        registry.emplace<SpriteComponent>(entity2);

        system.update(registry);

        auto& transform1 = registry.get<TransformComponent>(entity1);
        auto& transform2 = registry.get<TransformComponent>(entity2);

        REQUIRE_THAT(transform1.x, Catch::Matchers::WithinAbs(64.0f, 0.001f));   // 2 * 32
        REQUIRE_THAT(transform1.y, Catch::Matchers::WithinAbs(128.0f, 0.001f));  // (3+1) * 32
        REQUIRE_THAT(transform2.x, Catch::Matchers::WithinAbs(160.0f, 0.001f));  // 5 * 32
        REQUIRE_THAT(transform2.y, Catch::Matchers::WithinAbs(256.0f, 0.001f));  // (7+1) * 32
    }

    SECTION("AutoSync disabled - no synchronization") {
        auto entity = registry.create();

        auto& tilePos = registry.emplace<TilePositionComponent>(entity);
        tilePos.tileX = 10;
        tilePos.tileY = 20;
        tilePos.autoSync = false;  // Disable auto sync

        auto& transform = registry.emplace<TransformComponent>(entity);
        transform.x = 500.0f;
        transform.y = 600.0f;

        registry.emplace<SpriteComponent>(entity);

        system.update(registry);

        // Transform should not change when autoSync is false
        REQUIRE_THAT(transform.x, Catch::Matchers::WithinAbs(500.0f, 0.001f));
        REQUIRE_THAT(transform.y, Catch::Matchers::WithinAbs(600.0f, 0.001f));
    }
}

TEST_CASE("TilePositionSystem: Y-sorting layer calculation", "[TilePositionSystem]") {
    entt::registry registry;
    TilePositionSystem system;

    SECTION("Single entity layer calculation") {
        auto entity = registry.create();

        auto& tilePos = registry.emplace<TilePositionComponent>(entity);
        tilePos.tileX = 0;
        tilePos.tileY = 5;

        registry.emplace<TransformComponent>(entity);

        auto& sprite = registry.emplace<SpriteComponent>(entity);
        sprite.layer = toInt(RenderLayer::Objects);  // Base layer

        system.update(registry);

        // Layer should be Objects (200) + tileY (5) = 205
        REQUIRE(sprite.layer == toInt(RenderLayer::Objects) + 5);
    }

    SECTION("Multiple entities with different Y coordinates") {
        auto entity1 = registry.create();
        auto& tilePos1 = registry.emplace<TilePositionComponent>(entity1);
        tilePos1.tileY = 3;
        registry.emplace<TransformComponent>(entity1);
        auto& sprite1 = registry.emplace<SpriteComponent>(entity1);
        sprite1.layer = toInt(RenderLayer::Objects);

        auto entity2 = registry.create();
        auto& tilePos2 = registry.emplace<TilePositionComponent>(entity2);
        tilePos2.tileY = 7;
        registry.emplace<TransformComponent>(entity2);
        auto& sprite2 = registry.emplace<SpriteComponent>(entity2);
        sprite2.layer = toInt(RenderLayer::Objects);

        system.update(registry);

        // Entity2 should have higher layer (further down)
        REQUIRE(sprite1.layer == toInt(RenderLayer::Objects) + 3);
        REQUIRE(sprite2.layer == toInt(RenderLayer::Objects) + 7);
        REQUIRE(sprite2.layer > sprite1.layer);
    }

    SECTION("Overlay layer calculation") {
        auto entity = registry.create();

        auto& tilePos = registry.emplace<TilePositionComponent>(entity);
        tilePos.tileY = 10;

        registry.emplace<TransformComponent>(entity);
        registry.emplace<OverlayComponent>(entity);

        auto& sprite = registry.emplace<SpriteComponent>(entity);
        sprite.layer = toInt(RenderLayer::Overlays);

        system.update(registry);

        // Overlay layer should be Overlays (300) + tileY (10) = 310
        REQUIRE(sprite.layer == toInt(RenderLayer::Overlays) + 10);
    }
}

TEST_CASE("TilePositionSystem: Multi-tile objects", "[TilePositionSystem]") {
    entt::registry registry;
    TilePositionSystem system;

    auto entity = registry.create();

    auto& tilePos = registry.emplace<TilePositionComponent>(entity);
    tilePos.tileX = 4;
    tilePos.tileY = 8;
    tilePos.widthTiles = 3;
    tilePos.heightTiles = 2;

    auto& transform = registry.emplace<TransformComponent>(entity);
    registry.emplace<SpriteComponent>(entity);

    system.update(registry);

    // Bottom-left corner: (4*32, (8+2)*32) = (128, 320)
    REQUIRE_THAT(transform.x, Catch::Matchers::WithinAbs(128.0f, 0.001f));
    REQUIRE_THAT(transform.y, Catch::Matchers::WithinAbs(320.0f, 0.001f));
}

TEST_CASE("TilePositionSystem: System properties", "[TilePositionSystem]") {
    TilePositionSystem system;

    SECTION("System priority") {
        REQUIRE(system.getPriority() == 200);
    }

    SECTION("System name") {
        REQUIRE(std::string(system.getName()) == "TilePositionSystem");
    }
}
