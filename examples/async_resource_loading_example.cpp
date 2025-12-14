/**
 * @file async_resource_loading_example.cpp
 * @brief Примеры использования асинхронной загрузки ресурсов
 *
 * Этот файл демонстрирует различные способы использования
 * асинхронной загрузки ресурсов в ResourceManager.
 */

#include "core/ResourceManager.h"
#include "core/Logger.h"
#include <iostream>
#include <thread>
#include <chrono>

using namespace core;

/**
 * @brief Пример 1: Простая асинхронная загрузка одной текстуры
 */
void example1_simple_async_load() {
    std::cout << "\n=== Пример 1: Простая асинхронная загрузка ===" << std::endl;

    ResourceManager rm;

    // Запускаем асинхронную загрузку текстуры
    auto future = rm.loadTextureAsync("player.png", "assets/sprites/player.png");

    std::cout << "Загрузка началась, можем делать другую работу..." << std::endl;

    // Имитация другой работы
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "Делаем другую работу..." << std::endl;

    // Ждем завершения загрузки
    bool success = future.get();
    if (success) {
        std::cout << "Текстура загружена успешно!" << std::endl;
    } else {
        std::cout << "Ошибка загрузки текстуры" << std::endl;
    }
}

/**
 * @brief Пример 2: Проверка готовности ресурса без ожидания
 */
void example2_check_ready() {
    std::cout << "\n=== Пример 2: Проверка готовности ресурса ===" << std::endl;

    ResourceManager rm;

    // Запускаем асинхронную загрузку
    rm.loadTextureAsync("enemy.png", "assets/sprites/enemy.png");

    // Проверяем готовность в цикле обновления
    int attempts = 0;
    while (!rm.isTextureReady("enemy.png") && attempts < 10) {
        std::cout << "Текстура еще загружается... (попытка " << attempts + 1 << ")" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        attempts++;
    }

    if (rm.isTextureReady("enemy.png")) {
        std::cout << "Текстура готова к использованию!" << std::endl;
    }
}

/**
 * @brief Пример 3: Массовая предзагрузка с отслеживанием прогресса
 */
void example3_batch_preload() {
    std::cout << "\n=== Пример 3: Массовая предзагрузка ===" << std::endl;

    ResourceManager rm;

    // Список текстур для загрузки
    std::vector<std::string> textures = {
        "assets/sprites/player.png",
        "assets/sprites/enemy.png",
        "assets/sprites/bullet.png",
        "assets/sprites/explosion.png",
        "assets/sprites/powerup.png"
    };

    // Callback для отслеживания прогресса
    auto progressCallback = [](float progress) {
        int percent = static_cast<int>(progress * 100);
        std::cout << "Прогресс загрузки: " << percent << "%" << std::endl;
    };

    // Callback по завершении
    auto completionCallback = [](size_t loaded, size_t total) {
        std::cout << "Загрузка завершена: " << loaded << "/" << total << " текстур" << std::endl;
    };

    // Запускаем асинхронную предзагрузку
    auto future = rm.preloadTexturesAsync(textures, progressCallback, completionCallback);

    std::cout << "Предзагрузка запущена, делаем другую работу..." << std::endl;

    // Получаем результат (количество загруженных текстур)
    size_t loadedCount = future.get();
    std::cout << "Успешно загружено: " << loadedCount << " текстур" << std::endl;
}

/**
 * @brief Пример 4: Ожидание загрузки с таймаутом
 */
void example4_wait_with_timeout() {
    std::cout << "\n=== Пример 4: Ожидание с таймаутом ===" << std::endl;

    ResourceManager rm;

    // Запускаем загрузку
    rm.loadTextureAsync("large_texture.png", "assets/sprites/large_texture.png");

    // Ждем не более 1 секунды
    bool ready = rm.waitForTexture("large_texture.png", 1000);

    if (ready) {
        std::cout << "Текстура загружена в течение таймаута" << std::endl;
    } else {
        std::cout << "Превышен таймаут ожидания" << std::endl;
    }
}

/**
 * @brief Пример 5: Параллельная загрузка разных типов ресурсов
 */
void example5_parallel_loading() {
    std::cout << "\n=== Пример 5: Параллельная загрузка ===" << std::endl;

    ResourceManager rm;

    // Запускаем загрузку разных типов ресурсов параллельно
    auto textureFuture = rm.loadTextureAsync("icon.png", "assets/sprites/icon.png");
    auto fontFuture = rm.loadFontAsync("main", "assets/fonts/arial.ttf");
    auto soundFuture = rm.loadSoundAsync("shoot", "assets/sounds/shoot.wav");

    std::cout << "Все ресурсы загружаются параллельно..." << std::endl;

    // Ждем завершения всех загрузок
    bool textureOk = textureFuture.get();
    bool fontOk = fontFuture.get();
    bool soundOk = soundFuture.get();

    std::cout << "Результаты загрузки:" << std::endl;
    std::cout << "  Текстура: " << (textureOk ? "OK" : "FAIL") << std::endl;
    std::cout << "  Шрифт: " << (fontOk ? "OK" : "FAIL") << std::endl;
    std::cout << "  Звук: " << (soundOk ? "OK" : "FAIL") << std::endl;
}

/**
 * @brief Пример 6: Экран загрузки с реальным прогрессом
 */
void example6_loading_screen() {
    std::cout << "\n=== Пример 6: Экран загрузки ===" << std::endl;

    ResourceManager rm;

    // Подготавливаем списки всех ресурсов
    std::vector<std::string> textures = {
        "assets/sprites/player.png",
        "assets/sprites/enemy1.png",
        "assets/sprites/enemy2.png",
        "assets/sprites/background.png"
    };

    std::vector<std::pair<std::string, std::string>> fonts = {
        {"main", "assets/fonts/main.ttf"},
        {"title", "assets/fonts/title.ttf"}
    };

    std::vector<std::pair<std::string, std::string>> sounds = {
        {"shoot", "assets/sounds/shoot.wav"},
        {"explosion", "assets/sounds/explosion.wav"}
    };

    // Общий прогресс
    std::atomic<float> totalProgress{0.0f};
    const float texturesWeight = 0.5f;  // 50% от общего прогресса
    const float fontsWeight = 0.2f;     // 20%
    const float soundsWeight = 0.3f;    // 30%

    // Запускаем загрузку всех ресурсов
    auto texturesFuture = rm.preloadTexturesAsync(textures,
        [&](float p) { totalProgress = texturesWeight * p; });

    auto fontsFuture = rm.preloadFontsAsync(fonts,
        [&](float p) { totalProgress = texturesWeight + fontsWeight * p; });

    auto soundsFuture = rm.preloadSoundsAsync(sounds,
        [&](float p) { totalProgress = texturesWeight + fontsWeight + soundsWeight * p; });

    // Симулируем экран загрузки
    std::cout << "Экран загрузки:" << std::endl;
    while (rm.hasActiveLoads()) {
        int percent = static_cast<int>(totalProgress * 100);
        std::cout << "\rЗагрузка: [";
        int bars = percent / 5;
        for (int i = 0; i < 20; ++i) {
            if (i < bars) std::cout << "=";
            else std::cout << " ";
        }
        std::cout << "] " << percent << "% " << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::cout << std::endl;

    // Ждем завершения всех загрузок
    size_t texturesLoaded = texturesFuture.get();
    size_t fontsLoaded = fontsFuture.get();
    size_t soundsLoaded = soundsFuture.get();

    std::cout << "\nВсе ресурсы загружены!" << std::endl;
    std::cout << "  Текстуры: " << texturesLoaded << "/" << textures.size() << std::endl;
    std::cout << "  Шрифты: " << fontsLoaded << "/" << fonts.size() << std::endl;
    std::cout << "  Звуки: " << soundsLoaded << "/" << sounds.size() << std::endl;
}

/**
 * @brief Пример 7: Ленивая загрузка с фоновой предзагрузкой
 */
void example7_lazy_with_background() {
    std::cout << "\n=== Пример 7: Ленивая загрузка с фоновой предзагрузкой ===" << std::endl;

    ResourceManager rm;

    // Загружаем критичные ресурсы синхронно
    std::cout << "Загрузка критичных ресурсов..." << std::endl;
    rm.loadTexture("player.png", "assets/sprites/player.png");
    std::cout << "Критичные ресурсы загружены" << std::endl;

    // Запускаем фоновую загрузку остальных ресурсов
    std::cout << "Запуск фоновой загрузки..." << std::endl;
    std::vector<std::string> backgroundTextures = {
        "assets/sprites/enemy1.png",
        "assets/sprites/enemy2.png",
        "assets/sprites/enemy3.png"
    };
    rm.preloadTexturesAsync(backgroundTextures);

    // Можем начинать игру, пока остальные ресурсы грузятся
    std::cout << "Игра запущена! Остальные ресурсы грузятся в фоне..." << std::endl;

    // Перед использованием проверяем готовность
    if (!rm.isTextureReady("assets/sprites/enemy1.png")) {
        std::cout << "Ресурс еще не готов, ждем..." << std::endl;
        rm.waitForTexture("assets/sprites/enemy1.png");
    }

    std::cout << "Ресурс готов к использованию!" << std::endl;
}

int main() {
    // Инициализация логгера
    Logger::init(LogLevel::DEBUG);

    std::cout << "==================================================" << std::endl;
    std::cout << "  Примеры асинхронной загрузки ресурсов" << std::endl;
    std::cout << "==================================================" << std::endl;

    // Примечание: большинство примеров не будут работать без реальных файлов ресурсов
    std::cout << "\nПРИМЕЧАНИЕ: Для работы примеров необходимы файлы ресурсов." << std::endl;
    std::cout << "Примеры демонстрируют API, но могут завершаться с ошибками" << std::endl;
    std::cout << "из-за отсутствия файлов.\n" << std::endl;

    try {
        example1_simple_async_load();
        example2_check_ready();
        example3_batch_preload();
        example4_wait_with_timeout();
        example5_parallel_loading();
        example6_loading_screen();
        example7_lazy_with_background();
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "\n==================================================" << std::endl;
    std::cout << "  Все примеры выполнены" << std::endl;
    std::cout << "==================================================" << std::endl;

    return 0;
}
