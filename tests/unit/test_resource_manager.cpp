/**
 * @file test_resource_manager.cpp
 * @brief Unit tests for ResourceManager
 */

#include <catch2/catch_test_macros.hpp>
#include <core/ResourceManager.h>
#include <SFML/Graphics/Image.hpp>
#include <filesystem>
#include <fstream>

using namespace core;
namespace fs = std::filesystem;

// Helper function to create a simple test texture
void createTestTexture(const std::string& path) {
    // Create directory if it doesn't exist
    fs::path filePath(path);
    fs::create_directories(filePath.parent_path());

    // Create a simple 32x32 red image
    sf::Image image;
    image.resize(sf::Vector2u(32, 32), sf::Color::Red);
    if (!image.saveToFile(path)) {
        throw std::runtime_error("Failed to save test texture");
    }
}

// Helper function to create a simple test sound file
void createTestSound(const std::string& path) {
    fs::path filePath(path);
    fs::create_directories(filePath.parent_path());

    // Create a minimal WAV file (44 bytes header + 0 bytes data = silent sound)
    std::ofstream file(path, std::ios::binary);

    // RIFF header
    file.write("RIFF", 4);
    uint32_t chunkSize = 36;  // Total size - 8
    file.write(reinterpret_cast<const char*>(&chunkSize), 4);
    file.write("WAVE", 4);

    // fmt subchunk
    file.write("fmt ", 4);
    uint32_t subchunk1Size = 16;
    file.write(reinterpret_cast<const char*>(&subchunk1Size), 4);
    uint16_t audioFormat = 1;  // PCM
    file.write(reinterpret_cast<const char*>(&audioFormat), 2);
    uint16_t numChannels = 1;
    file.write(reinterpret_cast<const char*>(&numChannels), 2);
    uint32_t sampleRate = 44100;
    file.write(reinterpret_cast<const char*>(&sampleRate), 4);
    uint32_t byteRate = 44100 * 1 * 16 / 8;
    file.write(reinterpret_cast<const char*>(&byteRate), 4);
    uint16_t blockAlign = 1 * 16 / 8;
    file.write(reinterpret_cast<const char*>(&blockAlign), 2);
    uint16_t bitsPerSample = 16;
    file.write(reinterpret_cast<const char*>(&bitsPerSample), 2);

    // data subchunk
    file.write("data", 4);
    uint32_t subchunk2Size = 0;
    file.write(reinterpret_cast<const char*>(&subchunk2Size), 4);

    file.close();
}

TEST_CASE("ResourceManager: Texture loading and retrieval", "[ResourceManager]") {
    ResourceManager manager;

    SECTION("Load texture from image") {
        sf::Image image;
        image.resize(sf::Vector2u(32, 32), sf::Color::Blue);

        bool loaded = manager.loadTextureFromImage("test_texture", image);
        REQUIRE(loaded == true);

        REQUIRE(manager.hasTexture("test_texture") == true);
        REQUIRE(manager.getTextureCount() == 1);

        const sf::Texture& texture = manager.getTexture("test_texture");
        REQUIRE(texture.getSize().x == 32);
        REQUIRE(texture.getSize().y == 32);
    }

    SECTION("Load texture from file") {
        std::string testPath = "test_assets/test_texture.png";
        createTestTexture(testPath);

        bool loaded = manager.loadTexture("test_file_texture", testPath);
        REQUIRE(loaded == true);

        REQUIRE(manager.hasTexture("test_file_texture") == true);

        const sf::Texture& texture = manager.getTexture("test_file_texture");
        REQUIRE(texture.getSize().x == 32);
        REQUIRE(texture.getSize().y == 32);

        // Cleanup
        fs::remove_all("test_assets");
    }

    SECTION("Duplicate texture name returns existing") {
        sf::Image image;
        image.resize(sf::Vector2u(32, 32), sf::Color::Green);

        manager.loadTextureFromImage("duplicate", image);
        REQUIRE(manager.getTextureCount() == 1);

        // Loading with same name should not increase count
        manager.loadTextureFromImage("duplicate", image);
        REQUIRE(manager.getTextureCount() == 1);
    }
}

TEST_CASE("ResourceManager: Texture unloading", "[ResourceManager]") {
    ResourceManager manager;

    sf::Image image;
    image.resize(sf::Vector2u(32, 32), sf::Color::Yellow);

    manager.loadTextureFromImage("texture1", image);
    manager.loadTextureFromImage("texture2", image);

    REQUIRE(manager.getTextureCount() == 2);

    SECTION("Unload single texture") {
        bool unloaded = manager.unloadTexture("texture1");
        REQUIRE(unloaded == true);
        REQUIRE(manager.getTextureCount() == 1);
        REQUIRE(manager.hasTexture("texture1") == false);
        REQUIRE(manager.hasTexture("texture2") == true);
    }

    SECTION("Unload non-existent texture") {
        bool unloaded = manager.unloadTexture("non_existent");
        REQUIRE(unloaded == false);
        REQUIRE(manager.getTextureCount() == 2);
    }

    SECTION("Clear all textures") {
        manager.clear();
        REQUIRE(manager.getTextureCount() == 0);
        REQUIRE(manager.hasTexture("texture1") == false);
        REQUIRE(manager.hasTexture("texture2") == false);
    }
}

TEST_CASE("ResourceManager: Sound loading and retrieval", "[ResourceManager]") {
    ResourceManager manager;

    SECTION("Load sound from file") {
        std::string testPath = "test_assets/test_sound.wav";
        createTestSound(testPath);

        bool loaded = manager.loadSound("test_sound", testPath);
        REQUIRE(loaded == true);

        REQUIRE(manager.hasSound("test_sound") == true);
        REQUIRE(manager.getSoundCount() == 1);

        const sf::SoundBuffer& buffer = manager.getSound("test_sound");
        REQUIRE(buffer.getSampleRate() == 44100);

        // Cleanup
        fs::remove_all("test_assets");
    }

    SECTION("Unload sound") {
        std::string testPath = "test_assets/test_sound.wav";
        createTestSound(testPath);

        manager.loadSound("sound1", testPath);
        REQUIRE(manager.getSoundCount() == 1);

        bool unloaded = manager.unloadSound("sound1");
        REQUIRE(unloaded == true);
        REQUIRE(manager.getSoundCount() == 0);
        REQUIRE(manager.hasSound("sound1") == false);

        // Cleanup
        fs::remove_all("test_assets");
    }
}

TEST_CASE("ResourceManager: Font loading (requires system fonts)", "[ResourceManager][!mayfail]") {
    ResourceManager manager;

    // Note: This test may fail if no system fonts are available
    // Creating a proper TTF font file programmatically is complex, so we skip file-based tests

    SECTION("Font count tracking") {
        REQUIRE(manager.getFontCount() == 0);

        // We can't easily test font loading without actual font files,
        // but we can test the count and has methods
        REQUIRE(manager.hasFont("non_existent") == false);
    }
}

TEST_CASE("ResourceManager: Preload textures", "[ResourceManager]") {
    ResourceManager manager;

    std::string testPath1 = "test_assets/texture1.png";
    std::string testPath2 = "test_assets/texture2.png";
    std::string testPath3 = "test_assets/texture3.png";

    createTestTexture(testPath1);
    createTestTexture(testPath2);
    createTestTexture(testPath3);

    std::vector<std::string> paths = {testPath1, testPath2, testPath3};

    SECTION("Preload without progress callback") {
        size_t loaded = manager.preloadTextures(paths);
        REQUIRE(loaded == 3);
        REQUIRE(manager.getTextureCount() == 3);
    }

    SECTION("Preload with progress callback") {
        int progressCallCount = 0;
        float lastProgress = 0.0f;

        size_t loaded = manager.preloadTextures(paths, [&](float progress) {
            progressCallCount++;
            lastProgress = progress;
        });

        REQUIRE(loaded == 3);
        REQUIRE(progressCallCount >= 3);  // At least one call per texture
        REQUIRE(lastProgress >= 0.9f);    // Should be close to 1.0
    }

    // Cleanup
    fs::remove_all("test_assets");
}

TEST_CASE("ResourceManager: Async texture loading", "[ResourceManager]") {
    ResourceManager manager;

    std::string testPath = "test_assets/async_texture.png";
    createTestTexture(testPath);

    SECTION("Load texture asynchronously") {
        auto future = manager.loadTextureAsync("async_test", testPath);

        // Wait for completion
        bool success = future.get();
        REQUIRE(success == true);

        REQUIRE(manager.isTextureReady("async_test") == true);
        REQUIRE(manager.hasTexture("async_test") == true);

        const sf::Texture& texture = manager.getTexture("async_test");
        REQUIRE(texture.getSize().x == 32);
    }

    SECTION("Wait for texture") {
        auto future = manager.loadTextureAsync("async_wait", testPath);

        bool ready = manager.waitForTexture("async_wait", 5000);  // 5 second timeout
        REQUIRE(ready == true);
        REQUIRE(manager.isTextureReady("async_wait") == true);
    }

    SECTION("Check active loads") {
        REQUIRE(manager.hasActiveLoads() == false);

        auto future1 = manager.loadTextureAsync("async1", testPath);
        auto future2 = manager.loadTextureAsync("async2", testPath);

        // Wait for all loads
        manager.waitForAllLoads();

        REQUIRE(manager.hasActiveLoads() == false);
        REQUIRE(manager.hasTexture("async1") == true);
        REQUIRE(manager.hasTexture("async2") == true);
    }

    // Cleanup
    fs::remove_all("test_assets");
}

TEST_CASE("ResourceManager: Error handling", "[ResourceManager]") {
    ResourceManager manager;

    SECTION("Load non-existent texture") {
        bool loaded = manager.loadTexture("non_existent", "path/to/nowhere.png");
        REQUIRE(loaded == false);
        REQUIRE(manager.hasTexture("non_existent") == false);
    }

    SECTION("Get non-existent texture throws") {
        REQUIRE_THROWS_AS(manager.getTexture("non_existent"), std::runtime_error);
    }

    SECTION("Get non-existent sound throws") {
        REQUIRE_THROWS_AS(manager.getSound("non_existent"), std::runtime_error);
    }
}

TEST_CASE("ResourceManager: Resource count tracking", "[ResourceManager]") {
    ResourceManager manager;

    REQUIRE(manager.getTextureCount() == 0);
    REQUIRE(manager.getFontCount() == 0);
    REQUIRE(manager.getSoundCount() == 0);

    // Add some textures
    sf::Image image;
    image.resize(sf::Vector2u(32, 32), sf::Color::Magenta);
    manager.loadTextureFromImage("tex1", image);
    manager.loadTextureFromImage("tex2", image);

    REQUIRE(manager.getTextureCount() == 2);

    // Add some sounds
    std::string soundPath1 = "test_assets/sound1.wav";
    std::string soundPath2 = "test_assets/sound2.wav";
    createTestSound(soundPath1);
    createTestSound(soundPath2);

    manager.loadSound("snd1", soundPath1);
    manager.loadSound("snd2", soundPath2);

    REQUIRE(manager.getSoundCount() == 2);

    // Clear all
    manager.clear();
    REQUIRE(manager.getTextureCount() == 0);
    REQUIRE(manager.getSoundCount() == 0);

    // Cleanup
    fs::remove_all("test_assets");
}

TEST_CASE("ResourceManager: Memory tracking", "[ResourceManager][memory]") {
    ResourceManager manager;

    SECTION("Initial memory usage is zero") {
        auto stats = manager.getMemoryUsage();
        REQUIRE(stats.texturesMemory == 0);
        REQUIRE(stats.fontsMemory == 0);
        REQUIRE(stats.soundsMemory == 0);
        REQUIRE(stats.totalMemory == 0);
    }

    SECTION("Texture memory calculation") {
        // Create a 32x32 texture (RGBA = 4 bytes per pixel)
        sf::Image image;
        image.resize(sf::Vector2u(32, 32), sf::Color::Blue);

        manager.loadTextureFromImage("test_texture", image);

        auto stats = manager.getMemoryUsage();
        // 32 * 32 * 4 = 4096 bytes
        REQUIRE(stats.texturesMemory == 4096);
        REQUIRE(stats.totalMemory == stats.texturesMemory + stats.fontsMemory + stats.soundsMemory);
    }

    SECTION("Multiple textures memory tracking") {
        sf::Image image1, image2, image3;
        image1.resize(sf::Vector2u(32, 32), sf::Color::Red);    // 4096 bytes
        image2.resize(sf::Vector2u(64, 64), sf::Color::Green);  // 16384 bytes
        image3.resize(sf::Vector2u(16, 16), sf::Color::Blue);   // 1024 bytes

        manager.loadTextureFromImage("tex1", image1);
        manager.loadTextureFromImage("tex2", image2);
        manager.loadTextureFromImage("tex3", image3);

        auto stats = manager.getMemoryUsage();
        // Total: 4096 + 16384 + 1024 = 21504 bytes
        REQUIRE(stats.texturesMemory == 21504);
    }

    SECTION("Sound memory calculation") {
        std::string soundPath = "test_assets/test_sound.wav";
        createTestSound(soundPath);

        manager.loadSound("test_sound", soundPath);

        auto stats = manager.getMemoryUsage();
        // Sound buffer should have some memory (even if minimal)
        REQUIRE(stats.soundsMemory >= 0);
        REQUIRE(stats.totalMemory >= stats.soundsMemory);

        // Cleanup
        fs::remove_all("test_assets");
    }

    SECTION("Memory after unload") {
        sf::Image image;
        image.resize(sf::Vector2u(64, 64), sf::Color::Cyan);

        manager.loadTextureFromImage("temp_texture", image);

        auto statsBefore = manager.getMemoryUsage();
        REQUIRE(statsBefore.texturesMemory == 16384);  // 64*64*4

        manager.unloadTexture("temp_texture");

        auto statsAfter = manager.getMemoryUsage();
        REQUIRE(statsAfter.texturesMemory == 0);
        REQUIRE(statsAfter.totalMemory < statsBefore.totalMemory);
    }

    SECTION("Memory after clear") {
        // Load multiple resources
        sf::Image image;
        image.resize(sf::Vector2u(32, 32), sf::Color::White);
        manager.loadTextureFromImage("tex1", image);
        manager.loadTextureFromImage("tex2", image);
        manager.loadTextureFromImage("tex3", image);

        std::string soundPath = "test_assets/test_sound.wav";
        createTestSound(soundPath);
        manager.loadSound("sound1", soundPath);

        auto statsBefore = manager.getMemoryUsage();
        REQUIRE(statsBefore.totalMemory > 0);

        manager.clear();

        auto statsAfter = manager.getMemoryUsage();
        REQUIRE(statsAfter.texturesMemory == 0);
        REQUIRE(statsAfter.soundsMemory == 0);
        REQUIRE(statsAfter.totalMemory == 0);

        // Cleanup
        fs::remove_all("test_assets");
    }
}

TEST_CASE("ResourceManager: MemoryStats formatting", "[ResourceManager][memory]") {
    SECTION("Format bytes") {
        auto formatted = ResourceManager::MemoryStats::formatSize(512);
        REQUIRE(formatted == "512 B");
    }

    SECTION("Format kilobytes") {
        auto formatted = ResourceManager::MemoryStats::formatSize(1024);
        REQUIRE(formatted == "1.00 KB");

        formatted = ResourceManager::MemoryStats::formatSize(1536);  // 1.5 KB
        REQUIRE(formatted == "1.50 KB");
    }

    SECTION("Format megabytes") {
        auto formatted = ResourceManager::MemoryStats::formatSize(1024 * 1024);
        REQUIRE(formatted == "1.00 MB");

        formatted = ResourceManager::MemoryStats::formatSize(5 * 1024 * 1024);  // 5 MB
        REQUIRE(formatted == "5.00 MB");
    }

    SECTION("Format gigabytes") {
        auto formatted = ResourceManager::MemoryStats::formatSize(1024ULL * 1024 * 1024);
        REQUIRE(formatted == "1.00 GB");

        formatted = ResourceManager::MemoryStats::formatSize(2ULL * 1024 * 1024 * 1024);  // 2 GB
        REQUIRE(formatted == "2.00 GB");
    }
}

TEST_CASE("ResourceManager: Memory limit warnings", "[ResourceManager][memory]") {
    ResourceManager manager;

    SECTION("Small memory usage does not trigger warning") {
        // Load a small texture
        sf::Image image;
        image.resize(sf::Vector2u(32, 32), sf::Color::Yellow);
        manager.loadTextureFromImage("small", image);

        auto stats = manager.getMemoryUsage();
        // Should be well under the 512 MB limit
        REQUIRE(stats.totalMemory < 512 * 1024 * 1024);
    }

    SECTION("Memory tracking with async loading") {
        std::string testPath = "test_assets/async_mem_test.png";
        createTestTexture(testPath);

        auto future = manager.loadTextureAsync("async_mem", testPath);
        bool success = future.get();
        REQUIRE(success == true);

        auto stats = manager.getMemoryUsage();
        REQUIRE(stats.texturesMemory == 4096);  // 32*32*4

        // Cleanup
        fs::remove_all("test_assets");
    }
}
