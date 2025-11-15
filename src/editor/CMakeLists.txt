# Editor module - инструменты редактирования
add_library(Editor STATIC)

target_sources(Editor
    PRIVATE
        # Здесь будут добавлены исходные файлы
        # LevelEditor.cpp
        # ObjectPlacer.cpp
        # ConnectionEditor.cpp
        # ProjectManager.cpp
)

target_include_directories(Editor
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include/editor
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src/editor
)

target_link_libraries(Editor
    PUBLIC
        Core
        Rendering
        SQLite::SQLite3
        nlohmann_json::nlohmann_json
        yaml-cpp::yaml-cpp
        EnTT::EnTT
    PRIVATE
)

target_compile_features(Editor PUBLIC cxx_std_20)
