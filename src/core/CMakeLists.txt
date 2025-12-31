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
        SpriteMetadata.cpp
        AnimationData.cpp
        AudioManager.cpp
        Config.cpp
        Components.cpp
        EventBus.cpp
        State.cpp
        StateManager.cpp
        states/MenuState.cpp
        states/GameState.cpp
        states/PauseState.cpp
        states/SettingsState.cpp
        systems/RenderSystem.cpp
        systems/UpdateSystem.cpp
        systems/LifetimeSystem.cpp
        systems/CollisionSystem.cpp
        systems/FSMSystem.cpp
        systems/TilePositionSystem.cpp
        systems/AnimationSystem.cpp
        systems/AnimationSystemV2.cpp
        systems/OverlaySystem.cpp
        systems/SystemScheduler.cpp
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
        SFML::Audio
        spdlog::spdlog
        fmt::fmt
        EnTT::EnTT
        Boost::headers
        yaml-cpp::yaml-cpp
        nlohmann_json::nlohmann_json
        Simulation
        Rendering
    PRIVATE
)

target_compile_features(Core PUBLIC cxx_std_20)
