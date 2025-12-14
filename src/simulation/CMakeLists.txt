# Simulation module - физика и логика
add_library(Simulation STATIC)

target_sources(Simulation
    PUBLIC
        # Header-only компоненты
        ${CMAKE_SOURCE_DIR}/include/simulation/PhysicsComponents.h
    PRIVATE
        # Здесь будут добавлены исходные файлы
        # PhysicsSystem.cpp
        # SimulationWorld.cpp
)

target_include_directories(Simulation
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include/simulation
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src/simulation
)

target_link_libraries(Simulation
    PUBLIC
        Core
        box2d::box2d
        EnTT::EnTT
    PRIVATE
)

target_compile_features(Simulation PUBLIC cxx_std_20)
