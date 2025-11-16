# Core module - базовая функциональность
add_library(Core STATIC)

target_sources(Core
    PRIVATE
        Application.cpp
        Window.cpp
        Logger.cpp
        PerformanceMetrics.cpp
        InputManager.cpp
        ResourceManager.cpp
        State.cpp
        StateManager.cpp
        states/MenuState.cpp
        states/GameState.cpp
        states/PauseState.cpp
)

target_include_directories(Core
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include/core
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src/core
)

target_link_libraries(Core
    PUBLIC
        SFML::Graphics
        SFML::Window
        SFML::System
        spdlog::spdlog
        fmt::fmt
        EnTT::EnTT
    PRIVATE
)

target_compile_features(Core PUBLIC cxx_std_20)
