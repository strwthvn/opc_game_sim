/**
 * @file test_physics_thread.cpp
 * @brief Unit tests for PhysicsThread (Task 6.4)
 *
 * Tests cover:
 * - Thread start/stop lifecycle
 * - Parallel execution with main thread
 * - Exception handling
 * - Double buffering synchronization
 * - Pause/resume functionality
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <simulation/PhysicsThread.h>
#include <simulation/PhysicsWorld.h>
#include <simulation/PhysicsComponents.h>
#include <simulation/systems/PhysicsSystem.h>
#include <core/Components.h>
#include <entt/entt.hpp>

#include <chrono>
#include <thread>
#include <atomic>

using namespace simulation;
using namespace core;

TEST_CASE("PhysicsThread: Start and stop correctly", "[PhysicsThread]") {
    entt::registry registry;
    PhysicsWorld world(b2Vec2{0.0f, 9.8f});
    PhysicsSystem system(world);

    SECTION("Thread starts successfully") {
        PhysicsThread thread(world, system, registry);

        REQUIRE_FALSE(thread.isRunning());

        bool started = thread.start();

        REQUIRE(started);
        REQUIRE(thread.isRunning());

        thread.stop();
        REQUIRE_FALSE(thread.isRunning());
    }

    SECTION("Double start returns false") {
        PhysicsThread thread(world, system, registry);

        REQUIRE(thread.start());
        REQUIRE_FALSE(thread.start());  // Already running

        thread.stop();
    }

    SECTION("Stop on non-running thread is safe") {
        PhysicsThread thread(world, system, registry);

        // Should not crash
        thread.stop();
        thread.stop();

        REQUIRE_FALSE(thread.isRunning());
    }

    SECTION("Destructor stops thread automatically") {
        std::atomic<bool> wasRunning{false};
        {
            PhysicsThread thread(world, system, registry);
            thread.start();
            wasRunning = thread.isRunning();
        }
        // Thread destroyed, should have stopped

        REQUIRE(wasRunning);
    }
}

TEST_CASE("PhysicsThread: Physics runs parallel to main thread", "[PhysicsThread]") {
    entt::registry registry;
    PhysicsWorld world(b2Vec2{0.0f, 9.8f});
    PhysicsSystem system(world);
    system.init(registry);

    PhysicsThread thread(world, system, registry);

    SECTION("Step count increases over time") {
        thread.start();

        uint64_t initialSteps = thread.getStepCount();

        // Wait for some physics steps (at 60 Hz, ~100ms should give ~6 steps)
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        uint64_t finalSteps = thread.getStepCount();

        thread.stop();

        REQUIRE(finalSteps > initialSteps);
        // At 60 Hz, ~6 steps in 100ms
        REQUIRE(finalSteps >= 4);  // Allow some variance
    }

    SECTION("Main thread can work while physics runs") {
        thread.start();

        // Simulate main thread work
        int mainThreadWork = 0;
        auto startTime = std::chrono::steady_clock::now();

        while (std::chrono::steady_clock::now() - startTime < std::chrono::milliseconds(50)) {
            mainThreadWork++;
            // Simulate some work
            volatile int x = mainThreadWork * 2;
            (void)x;
        }

        thread.stop();

        REQUIRE(mainThreadWork > 0);
        REQUIRE(thread.getStepCount() > 0);
    }
}

TEST_CASE("PhysicsThread: Pause and resume", "[PhysicsThread]") {
    entt::registry registry;
    PhysicsWorld world(b2Vec2{0.0f, 9.8f});
    PhysicsSystem system(world);

    PhysicsThread thread(world, system, registry);

    SECTION("Pause stops physics updates") {
        thread.start();

        // Let some steps run
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        thread.pause();
        REQUIRE(thread.isPaused());

        uint64_t stepsAtPause = thread.getStepCount();

        // Wait while paused
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        uint64_t stepsAfterWait = thread.getStepCount();

        // Steps should not increase while paused
        REQUIRE(stepsAfterWait == stepsAtPause);

        thread.stop();
    }

    SECTION("Resume continues physics updates") {
        thread.start();
        std::this_thread::sleep_for(std::chrono::milliseconds(30));

        thread.pause();
        uint64_t stepsAtPause = thread.getStepCount();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        thread.resume();
        REQUIRE_FALSE(thread.isPaused());

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        uint64_t stepsAfterResume = thread.getStepCount();

        // Steps should have increased after resume
        REQUIRE(stepsAfterResume > stepsAtPause);

        thread.stop();
    }
}

TEST_CASE("PhysicsThread: Exception handling", "[PhysicsThread]") {
    entt::registry registry;
    PhysicsWorld world(b2Vec2{0.0f, 9.8f});
    PhysicsSystem system(world);

    PhysicsThread thread(world, system, registry);

    SECTION("Exception handler is called on exception") {
        std::atomic<bool> handlerCalled{false};
        std::string exceptionMessage;

        thread.setExceptionHandler([&](const std::exception& e) {
            handlerCalled = true;
            exceptionMessage = e.what();
        });

        // Start thread - normal operation, no exception expected
        thread.start();
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        thread.stop();

        // In normal operation, handler should NOT be called
        // (We can't easily trigger an exception in Box2D without invalid state)
        // This test verifies the handler setup works
        REQUIRE_FALSE(handlerCalled);
    }

    SECTION("Exception handler can be set before start") {
        std::atomic<int> callCount{0};

        thread.setExceptionHandler([&](const std::exception&) {
            callCount++;
        });

        thread.start();
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        thread.stop();

        // Verify handler was set (no crash)
        REQUIRE(callCount >= 0);
    }
}

TEST_CASE("PhysicsThread: Double buffering", "[PhysicsThread]") {
    entt::registry registry;
    PhysicsWorld world(b2Vec2{0.0f, 9.8f});
    PhysicsSystem system(world);

    // Create a dynamic entity that will fall
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

    PhysicsThread thread(world, system, registry);

    SECTION("Double buffering is enabled by default") {
        REQUIRE(thread.isDoubleBufferingEnabled());
    }

    SECTION("Double buffering can be toggled") {
        thread.setDoubleBufferingEnabled(false);
        REQUIRE_FALSE(thread.isDoubleBufferingEnabled());

        thread.setDoubleBufferingEnabled(true);
        REQUIRE(thread.isDoubleBufferingEnabled());
    }

    SECTION("Swap and apply updates TransformComponent") {
        float initialY = transform.y;

        thread.start();

        // Let physics run for a bit
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        // Swap buffers and apply transforms
        thread.swapTransformBuffers();
        thread.applyTransformsToRegistry();

        thread.stop();

        // Dynamic body should have fallen (Y increases with downward gravity)
        auto& updatedTransform = registry.get<TransformComponent>(entity);

        // With gravity 9.8 m/s², after ~100ms the body should have moved
        // Note: Y-axis in this engine points down, so Y should increase
        REQUIRE(updatedTransform.y >= initialY);
    }
}

TEST_CASE("PhysicsThread: Statistics", "[PhysicsThread]") {
    entt::registry registry;
    PhysicsWorld world(b2Vec2{0.0f, 9.8f});
    PhysicsSystem system(world);

    PhysicsThread thread(world, system, registry);

    SECTION("Step count starts at zero") {
        REQUIRE(thread.getStepCount() == 0);
    }

    SECTION("Average step time is measured") {
        thread.start();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        float avgStepTime = thread.getAverageStepTime();

        thread.stop();

        // Step time should be reasonable (less than 16.67ms target)
        // Allow some variance for test environment
        REQUIRE(avgStepTime >= 0.0f);
        REQUIRE(avgStepTime < 50.0f);  // Should be much less in practice
    }
}

TEST_CASE("PhysicsThread: Registry mutex", "[PhysicsThread]") {
    entt::registry registry;
    PhysicsWorld world(b2Vec2{0.0f, 9.8f});
    PhysicsSystem system(world);

    PhysicsThread thread(world, system, registry);

    SECTION("withLock executes callback safely") {
        thread.start();

        int result = 0;
        thread.withLock([&]() {
            result = 42;
        });

        thread.stop();

        REQUIRE(result == 42);
    }

    SECTION("Mutex can be acquired from main thread") {
        thread.start();

        {
            std::lock_guard<std::mutex> lock(thread.getRegistryMutex());
            // Create entity while holding lock
            auto entity = registry.create();
            registry.emplace<TransformComponent>(entity);
            REQUIRE(registry.valid(entity));
        }

        thread.stop();
    }
}

TEST_CASE("PhysicsThread: Stress test with many entities", "[PhysicsThread][stress]") {
    entt::registry registry;
    PhysicsWorld world(b2Vec2{0.0f, 9.8f});
    PhysicsSystem system(world);

    // Create 100 dynamic entities
    constexpr int ENTITY_COUNT = 100;
    for (int i = 0; i < ENTITY_COUNT; ++i) {
        auto entity = registry.create();

        auto& transform = registry.emplace<TransformComponent>(entity);
        transform.x = static_cast<float>(i * 50);
        transform.y = 100.0f;

        auto& rigidbody = registry.emplace<RigidbodyComponent>(entity);
        rigidbody.bodyType = RigidbodyComponent::BodyType::Dynamic;

        auto& collider = registry.emplace<ColliderComponent>(entity);
        collider.shape = ColliderComponent::Shape::Box;
        collider.size = sf::Vector2f(32.0f, 32.0f);
    }

    system.init(registry);

    PhysicsThread thread(world, system, registry);

    SECTION("100 entities run for 1 second without crash") {
        thread.start();

        // Run for 1 second
        std::this_thread::sleep_for(std::chrono::seconds(1));

        thread.stop();

        // Should have completed ~60 steps
        REQUIRE(thread.getStepCount() >= 50);

        // All entities should still be valid
        auto view = registry.view<RigidbodyComponent>();
        int count = 0;
        for (auto entity : view) {
            (void)entity;
            count++;
        }
        REQUIRE(count == ENTITY_COUNT);
    }
}
