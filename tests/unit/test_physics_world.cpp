/**
 * @file test_physics_world.cpp
 * @brief Unit tests for PhysicsWorld (Box2D 3.x integration)
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <simulation/PhysicsWorld.h>
#include <box2d/box2d.h>

using namespace simulation;

TEST_CASE("PhysicsWorld: Creation with default gravity", "[PhysicsWorld]") {
    SECTION("Create world with default gravity (0, 9.8)") {
        PhysicsWorld world;

        REQUIRE(world.isValid() == true);

        b2Vec2 gravity = world.getGravity();
        REQUIRE_THAT(gravity.x, Catch::Matchers::WithinAbs(0.0f, 0.001f));
        REQUIRE_THAT(gravity.y, Catch::Matchers::WithinAbs(9.8f, 0.001f));
    }
}

TEST_CASE("PhysicsWorld: Creation with custom gravity", "[PhysicsWorld]") {
    SECTION("Create world with zero gravity (space)") {
        PhysicsWorld world(b2Vec2{0.0f, 0.0f});

        REQUIRE(world.isValid() == true);

        b2Vec2 gravity = world.getGravity();
        REQUIRE_THAT(gravity.x, Catch::Matchers::WithinAbs(0.0f, 0.001f));
        REQUIRE_THAT(gravity.y, Catch::Matchers::WithinAbs(0.0f, 0.001f));
    }

    SECTION("Create world with inverted gravity") {
        PhysicsWorld world(b2Vec2{0.0f, -9.8f});

        REQUIRE(world.isValid() == true);

        b2Vec2 gravity = world.getGravity();
        REQUIRE_THAT(gravity.x, Catch::Matchers::WithinAbs(0.0f, 0.001f));
        REQUIRE_THAT(gravity.y, Catch::Matchers::WithinAbs(-9.8f, 0.001f));
    }

    SECTION("Create world with horizontal gravity") {
        PhysicsWorld world(b2Vec2{5.0f, 0.0f});

        REQUIRE(world.isValid() == true);

        b2Vec2 gravity = world.getGravity();
        REQUIRE_THAT(gravity.x, Catch::Matchers::WithinAbs(5.0f, 0.001f));
        REQUIRE_THAT(gravity.y, Catch::Matchers::WithinAbs(0.0f, 0.001f));
    }
}

TEST_CASE("PhysicsWorld: Simulation step does not crash", "[PhysicsWorld]") {
    PhysicsWorld world;

    SECTION("Single step with standard timestep") {
        const float TIMESTEP = 1.0f / 60.0f;

        REQUIRE_NOTHROW(world.step(TIMESTEP));
        REQUIRE(world.isValid() == true);
    }

    SECTION("Multiple steps") {
        const float TIMESTEP = 1.0f / 60.0f;

        for (int i = 0; i < 100; ++i) {
            REQUIRE_NOTHROW(world.step(TIMESTEP));
        }

        REQUIRE(world.isValid() == true);
    }

    SECTION("Step with variable timesteps") {
        REQUIRE_NOTHROW(world.step(1.0f / 60.0f));
        REQUIRE_NOTHROW(world.step(1.0f / 30.0f));
        REQUIRE_NOTHROW(world.step(1.0f / 120.0f));

        REQUIRE(world.isValid() == true);
    }
}

TEST_CASE("PhysicsWorld: Gravity modification", "[PhysicsWorld]") {
    PhysicsWorld world(b2Vec2{0.0f, 9.8f});

    SECTION("Change gravity to zero") {
        world.setGravity(b2Vec2{0.0f, 0.0f});

        b2Vec2 gravity = world.getGravity();
        REQUIRE_THAT(gravity.x, Catch::Matchers::WithinAbs(0.0f, 0.001f));
        REQUIRE_THAT(gravity.y, Catch::Matchers::WithinAbs(0.0f, 0.001f));
    }

    SECTION("Change gravity to negative") {
        world.setGravity(b2Vec2{0.0f, -9.8f});

        b2Vec2 gravity = world.getGravity();
        REQUIRE_THAT(gravity.x, Catch::Matchers::WithinAbs(0.0f, 0.001f));
        REQUIRE_THAT(gravity.y, Catch::Matchers::WithinAbs(-9.8f, 0.001f));
    }

    SECTION("Change gravity multiple times") {
        world.setGravity(b2Vec2{1.0f, 2.0f});
        b2Vec2 gravity1 = world.getGravity();
        REQUIRE_THAT(gravity1.x, Catch::Matchers::WithinAbs(1.0f, 0.001f));
        REQUIRE_THAT(gravity1.y, Catch::Matchers::WithinAbs(2.0f, 0.001f));

        world.setGravity(b2Vec2{3.0f, 4.0f});
        b2Vec2 gravity2 = world.getGravity();
        REQUIRE_THAT(gravity2.x, Catch::Matchers::WithinAbs(3.0f, 0.001f));
        REQUIRE_THAT(gravity2.y, Catch::Matchers::WithinAbs(4.0f, 0.001f));

        world.setGravity(b2Vec2{0.0f, 0.0f});
        b2Vec2 gravity3 = world.getGravity();
        REQUIRE_THAT(gravity3.x, Catch::Matchers::WithinAbs(0.0f, 0.001f));
        REQUIRE_THAT(gravity3.y, Catch::Matchers::WithinAbs(0.0f, 0.001f));
    }
}

TEST_CASE("PhysicsWorld: World ID access", "[PhysicsWorld]") {
    PhysicsWorld world;

    SECTION("Get valid world ID") {
        b2WorldId worldId = world.getWorldId();

        REQUIRE(b2World_IsValid(worldId) == true);
    }

    SECTION("World ID remains valid after steps") {
        b2WorldId worldId = world.getWorldId();

        const float TIMESTEP = 1.0f / 60.0f;
        world.step(TIMESTEP);
        world.step(TIMESTEP);

        REQUIRE(b2World_IsValid(worldId) == true);
    }
}

TEST_CASE("PhysicsWorld: Unit conversion utilities", "[PhysicsWorld]") {
    SECTION("Pixels to meters conversion") {
        REQUIRE_THAT(PhysicsWorld::pixelsToMeters(32.0f),
                     Catch::Matchers::WithinAbs(1.0f, 0.001f));
        REQUIRE_THAT(PhysicsWorld::pixelsToMeters(64.0f),
                     Catch::Matchers::WithinAbs(2.0f, 0.001f));
        REQUIRE_THAT(PhysicsWorld::pixelsToMeters(16.0f),
                     Catch::Matchers::WithinAbs(0.5f, 0.001f));
        REQUIRE_THAT(PhysicsWorld::pixelsToMeters(0.0f),
                     Catch::Matchers::WithinAbs(0.0f, 0.001f));
    }

    SECTION("Meters to pixels conversion") {
        REQUIRE_THAT(PhysicsWorld::metersToPixels(1.0f),
                     Catch::Matchers::WithinAbs(32.0f, 0.001f));
        REQUIRE_THAT(PhysicsWorld::metersToPixels(2.0f),
                     Catch::Matchers::WithinAbs(64.0f, 0.001f));
        REQUIRE_THAT(PhysicsWorld::metersToPixels(0.5f),
                     Catch::Matchers::WithinAbs(16.0f, 0.001f));
        REQUIRE_THAT(PhysicsWorld::metersToPixels(0.0f),
                     Catch::Matchers::WithinAbs(0.0f, 0.001f));
    }

    SECTION("Round-trip conversion") {
        float pixels = 128.0f;
        float meters = PhysicsWorld::pixelsToMeters(pixels);
        float backToPixels = PhysicsWorld::metersToPixels(meters);

        REQUIRE_THAT(backToPixels, Catch::Matchers::WithinAbs(pixels, 0.001f));
    }

    SECTION("Vector pixels to meters conversion") {
        b2Vec2 pixels{64.0f, 96.0f};
        b2Vec2 meters = PhysicsWorld::pixelsToMeters(pixels);

        REQUIRE_THAT(meters.x, Catch::Matchers::WithinAbs(2.0f, 0.001f));
        REQUIRE_THAT(meters.y, Catch::Matchers::WithinAbs(3.0f, 0.001f));
    }

    SECTION("Vector meters to pixels conversion") {
        b2Vec2 meters{2.0f, 3.0f};
        b2Vec2 pixels = PhysicsWorld::metersToPixels(meters);

        REQUIRE_THAT(pixels.x, Catch::Matchers::WithinAbs(64.0f, 0.001f));
        REQUIRE_THAT(pixels.y, Catch::Matchers::WithinAbs(96.0f, 0.001f));
    }

    SECTION("Vector round-trip conversion") {
        b2Vec2 originalPixels{100.0f, 200.0f};
        b2Vec2 meters = PhysicsWorld::pixelsToMeters(originalPixels);
        b2Vec2 backToPixels = PhysicsWorld::metersToPixels(meters);

        REQUIRE_THAT(backToPixels.x, Catch::Matchers::WithinAbs(originalPixels.x, 0.001f));
        REQUIRE_THAT(backToPixels.y, Catch::Matchers::WithinAbs(originalPixels.y, 0.001f));
    }
}

TEST_CASE("PhysicsWorld: Creating bodies in the world", "[PhysicsWorld]") {
    PhysicsWorld world;

    SECTION("Create static body") {
        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_staticBody;
        bodyDef.position = b2Vec2{0.0f, 0.0f};

        b2BodyId bodyId = b2CreateBody(world.getWorldId(), &bodyDef);

        REQUIRE(b2Body_IsValid(bodyId) == true);
        REQUIRE(b2Body_GetType(bodyId) == b2_staticBody);

        // Cleanup
        b2DestroyBody(bodyId);
    }

    SECTION("Create dynamic body") {
        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_dynamicBody;
        bodyDef.position = b2Vec2{1.0f, 5.0f};

        b2BodyId bodyId = b2CreateBody(world.getWorldId(), &bodyDef);

        REQUIRE(b2Body_IsValid(bodyId) == true);
        REQUIRE(b2Body_GetType(bodyId) == b2_dynamicBody);

        b2Vec2 position = b2Body_GetPosition(bodyId);
        REQUIRE_THAT(position.x, Catch::Matchers::WithinAbs(1.0f, 0.001f));
        REQUIRE_THAT(position.y, Catch::Matchers::WithinAbs(5.0f, 0.001f));

        // Cleanup
        b2DestroyBody(bodyId);
    }

    SECTION("Create multiple bodies") {
        std::vector<b2BodyId> bodies;

        for (int i = 0; i < 10; ++i) {
            b2BodyDef bodyDef = b2DefaultBodyDef();
            bodyDef.type = b2_dynamicBody;
            bodyDef.position = b2Vec2{static_cast<float>(i), 0.0f};

            b2BodyId bodyId = b2CreateBody(world.getWorldId(), &bodyDef);
            bodies.push_back(bodyId);

            REQUIRE(b2Body_IsValid(bodyId) == true);
        }

        // Verify all bodies are valid
        for (const auto& bodyId : bodies) {
            REQUIRE(b2Body_IsValid(bodyId) == true);
        }

        // Cleanup
        for (const auto& bodyId : bodies) {
            b2DestroyBody(bodyId);
        }
    }
}

TEST_CASE("PhysicsWorld: Physics simulation with gravity", "[PhysicsWorld]") {
    PhysicsWorld world(b2Vec2{0.0f, 10.0f});  // Positive Y gravity

    SECTION("Dynamic body falls under gravity") {
        // Create a dynamic body at position (0, 10)
        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_dynamicBody;
        bodyDef.position = b2Vec2{0.0f, 10.0f};

        b2BodyId bodyId = b2CreateBody(world.getWorldId(), &bodyDef);

        // Add a box shape to give it mass
        b2ShapeDef shapeDef = b2DefaultShapeDef();
        shapeDef.density = 1.0f;

        b2Polygon box = b2MakeBox(0.5f, 0.5f);
        b2CreatePolygonShape(bodyId, &shapeDef, &box);

        // Get initial position
        b2Vec2 initialPos = b2Body_GetPosition(bodyId);

        // Simulate for 1 second (60 steps)
        const float TIMESTEP = 1.0f / 60.0f;
        for (int i = 0; i < 60; ++i) {
            world.step(TIMESTEP);
        }

        // Get final position
        b2Vec2 finalPos = b2Body_GetPosition(bodyId);

        // Body should have moved down (positive Y direction)
        REQUIRE(finalPos.y > initialPos.y);

        // Cleanup
        b2DestroyBody(bodyId);
    }

    SECTION("Static body does not move under gravity") {
        // Create a static body
        b2BodyDef bodyDef = b2DefaultBodyDef();
        bodyDef.type = b2_staticBody;
        bodyDef.position = b2Vec2{0.0f, 5.0f};

        b2BodyId bodyId = b2CreateBody(world.getWorldId(), &bodyDef);

        // Get initial position
        b2Vec2 initialPos = b2Body_GetPosition(bodyId);

        // Simulate for 1 second
        const float TIMESTEP = 1.0f / 60.0f;
        for (int i = 0; i < 60; ++i) {
            world.step(TIMESTEP);
        }

        // Get final position
        b2Vec2 finalPos = b2Body_GetPosition(bodyId);

        // Static body should not move
        REQUIRE_THAT(finalPos.x, Catch::Matchers::WithinAbs(initialPos.x, 0.001f));
        REQUIRE_THAT(finalPos.y, Catch::Matchers::WithinAbs(initialPos.y, 0.001f));

        // Cleanup
        b2DestroyBody(bodyId);
    }
}

TEST_CASE("PhysicsWorld: Gravity change affects simulation", "[PhysicsWorld]") {
    PhysicsWorld world(b2Vec2{0.0f, 0.0f});  // Start with no gravity

    // Create a dynamic body
    b2BodyDef bodyDef = b2DefaultBodyDef();
    bodyDef.type = b2_dynamicBody;
    bodyDef.position = b2Vec2{0.0f, 10.0f};

    b2BodyId bodyId = b2CreateBody(world.getWorldId(), &bodyDef);

    // Add a box shape to give it mass
    b2ShapeDef shapeDef = b2DefaultShapeDef();
    shapeDef.density = 1.0f;

    b2Polygon box = b2MakeBox(0.5f, 0.5f);
    b2CreatePolygonShape(bodyId, &shapeDef, &box);

    SECTION("Body does not move with zero gravity") {
        b2Vec2 initialPos = b2Body_GetPosition(bodyId);

        const float TIMESTEP = 1.0f / 60.0f;
        for (int i = 0; i < 60; ++i) {
            world.step(TIMESTEP);
        }

        b2Vec2 finalPos = b2Body_GetPosition(bodyId);

        // Should remain at same position (allowing for small numerical errors)
        REQUIRE_THAT(finalPos.x, Catch::Matchers::WithinAbs(initialPos.x, 0.1f));
        REQUIRE_THAT(finalPos.y, Catch::Matchers::WithinAbs(initialPos.y, 0.1f));
    }

    SECTION("Body moves after gravity is enabled") {
        // Enable gravity
        world.setGravity(b2Vec2{0.0f, 10.0f});

        b2Vec2 initialPos = b2Body_GetPosition(bodyId);

        const float TIMESTEP = 1.0f / 60.0f;
        for (int i = 0; i < 60; ++i) {
            world.step(TIMESTEP);
        }

        b2Vec2 finalPos = b2Body_GetPosition(bodyId);

        // Body should have moved down
        REQUIRE(finalPos.y > initialPos.y);
    }

    // Cleanup
    b2DestroyBody(bodyId);
}

TEST_CASE("PhysicsWorld: RAII behavior", "[PhysicsWorld]") {
    SECTION("World is destroyed when going out of scope") {
        b2WorldId capturedId;

        {
            PhysicsWorld world;
            capturedId = world.getWorldId();

            REQUIRE(b2World_IsValid(capturedId) == true);
            REQUIRE(world.isValid() == true);
        }

        // After PhysicsWorld destructor, world ID should be invalid
        REQUIRE(b2World_IsValid(capturedId) == false);
    }
}
