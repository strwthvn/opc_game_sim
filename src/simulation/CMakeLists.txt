# Simulation module - физика и логика
add_library(Simulation STATIC)

target_sources(Simulation
    PUBLIC
        # Header-only компоненты
        ${CMAKE_SOURCE_DIR}/include/simulation/PhysicsComponents.h
        ${CMAKE_SOURCE_DIR}/include/simulation/PhysicsWorld.h
        ${CMAKE_SOURCE_DIR}/include/simulation/PhysicsBodyFactory.h
        ${CMAKE_SOURCE_DIR}/include/simulation/PhysicsEventProcessor.h
        ${CMAKE_SOURCE_DIR}/include/simulation/events/CollisionEvents.h
        ${CMAKE_SOURCE_DIR}/include/simulation/systems/PhysicsSystem.h
    PRIVATE
        # Исходные файлы
        PhysicsWorld.cpp
        PhysicsBodyFactory.cpp
        PhysicsEventProcessor.cpp
        systems/PhysicsSystem.cpp
)

target_include_directories(Simulation
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include/simulation
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src/simulation
)

find_package(Boost REQUIRED)

target_link_libraries(Simulation
    PUBLIC
        Core
        box2d::box2d
        EnTT::EnTT
        Boost::boost  # Header-only (signals2)
    PRIVATE
)

target_compile_features(Simulation PUBLIC cxx_std_20)
