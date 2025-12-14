/**
 * @file test_fsm_system.cpp
 * @brief Unit tests for FSMSystem and EntityStateComponent
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <core/systems/FSMSystem.h>
#include <core/Components.h>
#include <entt/entt.hpp>

using namespace core;

TEST_CASE("EntityStateComponent: Default constructor", "[EntityStateComponent]") {
    EntityStateComponent state;

    REQUIRE(state.currentState == "");
    REQUIRE(state.previousState == "");
    REQUIRE_THAT(state.timeInState, Catch::Matchers::WithinAbs(0.0f, 0.001f));
}

TEST_CASE("EntityStateComponent: Constructor with initial state", "[EntityStateComponent]") {
    EntityStateComponent state("idle");

    REQUIRE(state.currentState == "idle");
    REQUIRE(state.previousState == "");
    REQUIRE_THAT(state.timeInState, Catch::Matchers::WithinAbs(0.0f, 0.001f));
}

TEST_CASE("EntityStateComponent: isInState()", "[EntityStateComponent]") {
    EntityStateComponent state("running");

    REQUIRE(state.isInState("running") == true);
    REQUIRE(state.isInState("idle") == false);
    REQUIRE(state.isInState("error") == false);
}

TEST_CASE("EntityStateComponent: setState() basic functionality", "[EntityStateComponent]") {
    EntityStateComponent state("idle");

    state.setState("running");

    REQUIRE(state.currentState == "running");
    REQUIRE(state.previousState == "idle");
    REQUIRE_THAT(state.timeInState, Catch::Matchers::WithinAbs(0.0f, 0.001f));
}

TEST_CASE("EntityStateComponent: State transition callbacks", "[EntityStateComponent]") {
    EntityStateComponent state("idle");

    bool onEnterCalled = false;
    bool onExitCalled = false;

    state.registerOnEnter("running", [&onEnterCalled]() {
        onEnterCalled = true;
    });

    state.registerOnExit("idle", [&onExitCalled]() {
        onExitCalled = true;
    });

    state.setState("running");

    REQUIRE(onExitCalled == true);
    REQUIRE(onEnterCalled == true);
    REQUIRE(state.currentState == "running");
    REQUIRE(state.previousState == "idle");
}

TEST_CASE("EntityStateComponent: Multiple state transitions", "[EntityStateComponent]") {
    EntityStateComponent state("idle");

    int transitionCount = 0;

    state.registerOnEnter("running", [&transitionCount]() { transitionCount++; });
    state.registerOnEnter("error", [&transitionCount]() { transitionCount++; });
    state.registerOnExit("idle", [&transitionCount]() { transitionCount++; });
    state.registerOnExit("running", [&transitionCount]() { transitionCount++; });

    // idle -> running
    state.setState("running");
    REQUIRE(transitionCount == 2);  // onExit(idle) + onEnter(running)
    REQUIRE(state.currentState == "running");
    REQUIRE(state.previousState == "idle");

    // running -> error
    state.setState("error");
    REQUIRE(transitionCount == 4);  // + onExit(running) + onEnter(error)
    REQUIRE(state.currentState == "error");
    REQUIRE(state.previousState == "running");
}

TEST_CASE("EntityStateComponent: Same state transition", "[EntityStateComponent]") {
    EntityStateComponent state("idle");

    int enterCount = 0;
    int exitCount = 0;

    state.registerOnEnter("idle", [&enterCount]() { enterCount++; });
    state.registerOnExit("idle", [&exitCount]() { exitCount++; });

    // Transition to the same state - implementation ignores this (early return)
    state.setState("idle");

    // Callbacks should NOT be called for same state (optimization)
    REQUIRE(exitCount == 0);
    REQUIRE(enterCount == 0);
    REQUIRE(state.currentState == "idle");
    // previousState remains empty since no actual transition occurred
    REQUIRE(state.previousState == "");
}

TEST_CASE("FSMSystem: Time in state tracking", "[FSMSystem]") {
    entt::registry registry;
    FSMSystem system;

    auto entity = registry.create();
    auto& state = registry.emplace<EntityStateComponent>(entity, "idle");

    // Update with 0.5 seconds
    system.update(registry, 0.5);
    REQUIRE_THAT(state.timeInState, Catch::Matchers::WithinAbs(0.5f, 0.001f));

    // Update with another 0.3 seconds
    system.update(registry, 0.3);
    REQUIRE_THAT(state.timeInState, Catch::Matchers::WithinAbs(0.8f, 0.001f));

    // Change state - timeInState should reset
    state.setState("running");
    REQUIRE_THAT(state.timeInState, Catch::Matchers::WithinAbs(0.0f, 0.001f));

    // Update again
    system.update(registry, 0.2);
    REQUIRE_THAT(state.timeInState, Catch::Matchers::WithinAbs(0.2f, 0.001f));
}

TEST_CASE("FSMSystem: Multiple entities", "[FSMSystem]") {
    entt::registry registry;
    FSMSystem system;

    auto entity1 = registry.create();
    auto& state1 = registry.emplace<EntityStateComponent>(entity1, "idle");

    auto entity2 = registry.create();
    auto& state2 = registry.emplace<EntityStateComponent>(entity2, "running");

    system.update(registry, 1.0);

    REQUIRE_THAT(state1.timeInState, Catch::Matchers::WithinAbs(1.0f, 0.001f));
    REQUIRE_THAT(state2.timeInState, Catch::Matchers::WithinAbs(1.0f, 0.001f));

    // Change state for entity1 only
    state1.setState("running");
    REQUIRE_THAT(state1.timeInState, Catch::Matchers::WithinAbs(0.0f, 0.001f));
    REQUIRE_THAT(state2.timeInState, Catch::Matchers::WithinAbs(1.0f, 0.001f));

    system.update(registry, 0.5);

    REQUIRE_THAT(state1.timeInState, Catch::Matchers::WithinAbs(0.5f, 0.001f));
    REQUIRE_THAT(state2.timeInState, Catch::Matchers::WithinAbs(1.5f, 0.001f));
}

TEST_CASE("FSMSystem: System properties", "[FSMSystem]") {
    FSMSystem system;

    SECTION("System priority") {
        REQUIRE(system.getPriority() == 150);
    }

    SECTION("System name") {
        REQUIRE(std::string(system.getName()) == "FSMSystem");
    }
}

TEST_CASE("EntityStateComponent: Callback registration and clearing", "[EntityStateComponent]") {
    EntityStateComponent state("idle");

    int callCount = 0;

    state.registerOnEnter("running", [&callCount]() { callCount++; });

    state.setState("running");
    REQUIRE(callCount == 1);

    // Register new callback for the same state (should override)
    state.registerOnEnter("running", [&callCount]() { callCount += 10; });

    state.setState("idle");
    state.setState("running");
    REQUIRE(callCount == 11);  // Should use the new callback (1 + 10)
}
