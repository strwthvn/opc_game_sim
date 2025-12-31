# Unit tests
if(Catch2_FOUND)
    add_executable(UnitTests
        test_tile_position_component.cpp
        test_tile_position_system.cpp
        test_fsm_system.cpp
        test_collision_system.cpp
        test_collision_events.cpp
        test_resource_manager.cpp
        test_sprite_metadata.cpp
        test_config.cpp
        test_logger.cpp
        test_physics_world.cpp
        test_physics_body_factory.cpp
        test_physics_system.cpp
    )

    target_link_libraries(UnitTests PRIVATE
        Rendering
        Simulation
        Core
        Catch2::Catch2WithMain
        SFML::Graphics
        SFML::System
        SFML::Audio
        EnTT::EnTT
        spdlog::spdlog
        yaml-cpp::yaml-cpp
        PkgConfig::TMXLITE
        zstd::libzstd_static
        box2d::box2d
    )

    target_include_directories(UnitTests PRIVATE
        ${CMAKE_SOURCE_DIR}/include
    )

    include(CTest)
    include(Catch)
    catch_discover_tests(UnitTests)
endif()
