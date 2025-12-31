# Rendering module - визуализация
add_library(Rendering STATIC)

target_sources(Rendering
    PRIVATE
        TileMapSystem.cpp
        PhysicsDebugDraw.cpp
        # RenderSystem.cpp
        # Camera.cpp
        # SpriteRenderer.cpp
)

target_include_directories(Rendering
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include/rendering
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src/rendering
)

target_link_libraries(Rendering
    PUBLIC
        Core
        Simulation
        SFML::Graphics
        PkgConfig::TMXLITE
        EnTT::EnTT
        spdlog::spdlog
        zstd::libzstd_static
    PRIVATE
)

target_compile_features(Rendering PUBLIC cxx_std_20)
