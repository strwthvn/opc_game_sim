/**
 * @file test_physics_debug_draw.cpp
 * @brief Тесты для PhysicsDebugDraw
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "rendering/PhysicsDebugDraw.h"
#include <box2d/box2d.h>

using Catch::Approx;

TEST_CASE("PhysicsDebugDraw construction", "[PhysicsDebugDraw]") {
    rendering::PhysicsDebugDraw debugDraw;
    b2DebugDraw& dd = debugDraw.getDebugDraw();

    SECTION("Context is set to this pointer") {
        REQUIRE(dd.context != nullptr);
    }

    SECTION("All function pointers are set") {
        REQUIRE(dd.DrawPolygonFcn != nullptr);
        REQUIRE(dd.DrawSolidPolygonFcn != nullptr);
        REQUIRE(dd.DrawCircleFcn != nullptr);
        REQUIRE(dd.DrawSolidCircleFcn != nullptr);
        REQUIRE(dd.DrawSolidCapsuleFcn != nullptr);
        REQUIRE(dd.DrawSegmentFcn != nullptr);
        REQUIRE(dd.DrawTransformFcn != nullptr);
        REQUIRE(dd.DrawPointFcn != nullptr);
        REQUIRE(dd.DrawStringFcn != nullptr);
    }

    SECTION("Default flags are set correctly") {
        // drawShapes should be enabled by default
        REQUIRE(dd.drawShapes == true);
        REQUIRE(dd.drawJoints == false);
        REQUIRE(dd.drawBounds == false);
        REQUIRE(dd.drawContacts == false);
        REQUIRE(dd.drawMass == false);
    }
}

TEST_CASE("PhysicsDebugDraw flag setters", "[PhysicsDebugDraw]") {
    rendering::PhysicsDebugDraw debugDraw;
    b2DebugDraw& dd = debugDraw.getDebugDraw();

    SECTION("setDrawShapes") {
        debugDraw.setDrawShapes(false);
        REQUIRE(dd.drawShapes == false);

        debugDraw.setDrawShapes(true);
        REQUIRE(dd.drawShapes == true);
    }

    SECTION("setDrawJoints") {
        debugDraw.setDrawJoints(true);
        REQUIRE(dd.drawJoints == true);

        debugDraw.setDrawJoints(false);
        REQUIRE(dd.drawJoints == false);
    }

    SECTION("setDrawBounds") {
        debugDraw.setDrawBounds(true);
        REQUIRE(dd.drawBounds == true);

        debugDraw.setDrawBounds(false);
        REQUIRE(dd.drawBounds == false);
    }

    SECTION("setDrawContacts") {
        debugDraw.setDrawContacts(true);
        REQUIRE(dd.drawContacts == true);

        debugDraw.setDrawContacts(false);
        REQUIRE(dd.drawContacts == false);
    }

    SECTION("setDrawMass") {
        debugDraw.setDrawMass(true);
        REQUIRE(dd.drawMass == true);

        debugDraw.setDrawMass(false);
        REQUIRE(dd.drawMass == false);
    }
}

TEST_CASE("PhysicsDebugDraw begin/end without target", "[PhysicsDebugDraw]") {
    rendering::PhysicsDebugDraw debugDraw;

    // Should not crash when called without valid target
    SECTION("end() without begin() should not crash") {
        REQUIRE_NOTHROW(debugDraw.end());
    }

    SECTION("begin() with nullptr should handle gracefully") {
        REQUIRE_NOTHROW(debugDraw.begin(nullptr));
        REQUIRE_NOTHROW(debugDraw.end());
    }
}

TEST_CASE("PhysicsDebugDraw with Box2D world", "[PhysicsDebugDraw][integration]") {
    // Create a Box2D world
    b2WorldDef worldDef = b2DefaultWorldDef();
    worldDef.gravity = b2Vec2{0.0f, 9.8f};
    b2WorldId worldId = b2CreateWorld(&worldDef);

    REQUIRE(b2World_IsValid(worldId));

    // Create a simple body for testing
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = b2Vec2{5.0f, 5.0f};
    b2BodyId bodyId = b2CreateBody(worldId, &bodyDef);

    // Create a box shape
    b2Polygon box = b2MakeBox(1.0f, 1.0f);
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 1.0f;
    b2CreatePolygonShape(bodyId, &shapeDef, &box);

    // Create debug draw
    rendering::PhysicsDebugDraw debugDraw;

    SECTION("b2World_Draw should not crash") {
        // We can't test actual rendering without a window,
        // but we can verify it doesn't crash
        debugDraw.begin(nullptr);  // nullptr target will prevent actual drawing
        REQUIRE_NOTHROW(b2World_Draw(worldId, &debugDraw.getDebugDraw()));
        debugDraw.end();
    }

    // Cleanup
    b2DestroyWorld(worldId);
}
