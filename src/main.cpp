#include "core/Application.h"
#include <iostream>
#include <cstdlib>

/**
 * @brief Точка входа в приложение OPC Game Simulator
 *
 * Создает и запускает главное приложение с базовой конфигурацией.
 *
 * @param argc Количество аргументов командной строки
 * @param argv Массив аргументов командной строки
 * @return Код возврата приложения (0 при успехе)
 */
int main([[maybe_unused]] int argc, [[maybe_unused]] char* argv[]) {
    try {
        // Конфигурация приложения
        core::Application::Config config;
        config.windowConfig.title = "OPC Game Simulator v0.1.0";
        config.windowConfig.width = 1280;
        config.windowConfig.height = 720;
        config.windowConfig.framerateLimit = 60;
        config.windowConfig.vsync = false;
        config.fixedTimestep = 1.0 / 60.0;  // 60 updates per second

        // Создаем и запускаем приложение
        core::Application app(config);

        std::cout << "=== OPC Game Simulator ===" << std::endl;
        std::cout << "Version: 0.1.0" << std::endl;
        std::cout << "Phase 1 - Milestone 1.1: Basic Window" << std::endl;
        std::cout << "============================" << std::endl;
        std::cout << "Press ESC to exit" << std::endl;

        return app.run();

    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return EXIT_FAILURE;
    } catch (...) {
        std::cerr << "Unknown fatal error occurred" << std::endl;
        return EXIT_FAILURE;
    }
}
