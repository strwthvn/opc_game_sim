# Rendering module - визуализация
add_library(Rendering STATIC)

target_sources(Rendering
    PRIVATE
        # Здесь будут добавлены исходные файлы
        # RenderSystem.cpp
        # Camera.cpp
        # TileMapRenderer.cpp
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
        sfml::graphics
        tmxlite::tmxlite
        EnTT::EnTT
    PRIVATE
)

target_compile_features(Rendering PUBLIC cxx_std_20)
