cmake_minimum_required(VERSION 3.20)

# Определение проекта
project(OPCGameSimulator
    VERSION 0.1.0
    DESCRIPTION "2D PLC Simulator with OPC UA integration"
    LANGUAGES CXX
)

# Настройка C++ стандарта
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Опции сборки
option(BUILD_TESTS "Build unit and integration tests" ON)
option(BUILD_DOCS "Build documentation with Doxygen" OFF)
option(ENABLE_TRACY "Enable Tracy profiler integration" OFF)
option(ENABLE_MODBUS "Enable Modbus protocol support" OFF)
option(ENABLE_OPENAL "Enable OpenAL for 3D audio" OFF)

# Пути для выходных файлов
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Включение модулей CMake
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Интеграция vcpkg
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
        CACHE STRING "Vcpkg toolchain file")
endif()

# Поиск зависимостей
find_package(PkgConfig REQUIRED)
find_package(SFML 3 REQUIRED COMPONENTS Graphics Window System Audio)
find_package(EnTT REQUIRED)
find_package(box2d REQUIRED)
pkg_check_modules(TMXLITE REQUIRED IMPORTED_TARGET tmxlite)
find_package(zstd REQUIRED)
# Temporarily disabled for Windows/MinGW build (open62541 has MinGW compatibility issues)
# find_package(open62541 REQUIRED)
find_package(Lua REQUIRED)
find_package(sol2 REQUIRED)
find_package(Boost REQUIRED COMPONENTS signals2)
find_package(nlohmann_json REQUIRED)
find_package(yaml-cpp REQUIRED)
find_package(SQLite3 REQUIRED)
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED)
find_package(asio REQUIRED)
find_package(imgui REQUIRED)
find_package(implot REQUIRED)

# Опциональные зависимости
if(ENABLE_MODBUS)
    find_package(libmodbus REQUIRED)
endif()

if(ENABLE_OPENAL)
    find_package(OpenAL REQUIRED)
endif()

if(ENABLE_TRACY)
    find_package(Tracy REQUIRED)
endif()

# Включаемые директории
include_directories(${CMAKE_SOURCE_DIR}/include)

# Модули проекта (будут добавлены по мере реализации)
add_subdirectory(src/core)
add_subdirectory(src/simulation)
add_subdirectory(src/rendering)
# add_subdirectory(src/industrial)
# add_subdirectory(src/scripting)
# add_subdirectory(src/editor)
# add_subdirectory(src/ui)

# Главный исполняемый файл
add_executable(${PROJECT_NAME}
    src/main.cpp
)

# Линковка всех модулей (будут добавлены по мере реализации)
target_link_libraries(${PROJECT_NAME} PRIVATE
    Core
    Rendering
    Simulation
#     Industrial
#     Scripting
#     Editor
#     UI
#     SFML::Graphics
#     SFML::Window
#     SFML::System
#     SFML::Audio
#     EnTT::EnTT
#     box2d::box2d
#     PkgConfig::TMXLITE
#     open62541::open62541
#     ${LUA_LIBRARIES}
#     sol2::sol2
#     Boost::signals2
#     nlohmann_json::nlohmann_json
#     yaml-cpp::yaml-cpp
#     SQLite::SQLite3
#     spdlog::spdlog
#     fmt::fmt
#     asio::asio
#     imgui::imgui
#     implot::implot
)

# target_include_directories(${PROJECT_NAME} PRIVATE
#     ${LUA_INCLUDE_DIR}
# )

# Опциональные зависимости
if(ENABLE_MODBUS)
    target_link_libraries(${PROJECT_NAME} PRIVATE libmodbus::libmodbus)
    target_compile_definitions(${PROJECT_NAME} PRIVATE ENABLE_MODBUS)
endif()

if(ENABLE_OPENAL)
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenAL::OpenAL)
    target_compile_definitions(${PROJECT_NAME} PRIVATE ENABLE_OPENAL)
endif()

if(ENABLE_TRACY)
    target_link_libraries(${PROJECT_NAME} PRIVATE Tracy::TracyClient)
    target_compile_definitions(${PROJECT_NAME} PRIVATE ENABLE_TRACY)
endif()

# Компиляторные флаги
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4 /WX)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()

# Копирование ресурсов в директорию сборки
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})

# Тесты
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Документация
if(BUILD_DOCS)
    find_package(Doxygen)
    if(DOXYGEN_FOUND)
        add_subdirectory(docs)
    else()
        message(WARNING "Doxygen not found, documentation will not be built")
    endif()
endif()

# Информация о сборке
message(STATUS "=== OPC Game Simulator Build Configuration ===")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Tests: ${BUILD_TESTS}")
message(STATUS "Build Docs: ${BUILD_DOCS}")
message(STATUS "Tracy Profiler: ${ENABLE_TRACY}")
message(STATUS "Modbus Support: ${ENABLE_MODBUS}")
message(STATUS "OpenAL Audio: ${ENABLE_OPENAL}")
message(STATUS "==============================================")
