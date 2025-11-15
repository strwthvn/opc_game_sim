# Scripting module - Lua интеграция
add_library(Scripting STATIC)

target_sources(Scripting
    PRIVATE
        # Здесь будут добавлены исходные файлы
        # ScriptEngine.cpp
        # ScriptComponent.cpp
        # LuaBindings.cpp
)

target_include_directories(Scripting
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include/scripting
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src/scripting
        ${LUA_INCLUDE_DIR}
)

target_link_libraries(Scripting
    PUBLIC
        Core
        ${LUA_LIBRARIES}
        sol2::sol2
        EnTT::EnTT
    PRIVATE
)

target_compile_features(Scripting PUBLIC cxx_std_20)
