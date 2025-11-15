# UI module - пользовательский интерфейс
add_library(UI STATIC)

target_sources(UI
    PRIVATE
        # Здесь будут добавлены исходные файлы
        # ImGuiManager.cpp
        # Inspector.cpp
        # HierarchyPanel.cpp
        # OPCUABrowser.cpp
        # PlotWindow.cpp
        # Console.cpp
)

target_include_directories(UI
    PUBLIC
        ${CMAKE_SOURCE_DIR}/include/ui
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src/ui
)

target_link_libraries(UI
    PUBLIC
        Core
        Industrial
        imgui::imgui
        implot::implot
        sfml::graphics
        EnTT::EnTT
    PRIVATE
)

target_compile_features(UI PUBLIC cxx_std_20)
