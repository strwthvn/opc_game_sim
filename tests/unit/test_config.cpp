/**
 * @file test_config.cpp
 * @brief Unit tests for Config system
 */

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>
#include <core/Config.h>
#include <filesystem>
#include <fstream>

using namespace core;
namespace fs = std::filesystem;

// Helper to create a test config file
void createTestConfigFile(const std::string& path, const std::string& content) {
    fs::path filePath(path);
    fs::create_directories(filePath.parent_path());

    std::ofstream file(path);
    file << content;
    file.close();
}

TEST_CASE("Config: Singleton instance", "[Config]") {
    Config& config1 = Config::getInstance();
    Config& config2 = Config::getInstance();

    // Should be the same instance
    REQUIRE(&config1 == &config2);
}

TEST_CASE("Config: Load and save", "[Config]") {
    Config& config = Config::getInstance();

    std::string testPath = "test_config/config.yaml";

    SECTION("Load existing config file") {
        std::string yamlContent = R"(
window:
  width: 1280
  height: 720
  title: "Test Window"
  fullscreen: false

camera:
  zoom: 0.5
  speed: 600.0

game:
  targetFPS: 60
  debug: true
)";
        createTestConfigFile(testPath, yamlContent);

        bool loaded = config.load(testPath);
        REQUIRE(loaded == true);

        // Verify loaded values
        REQUIRE(config.get<int>("window.width", 0) == 1280);
        REQUIRE(config.get<int>("window.height", 0) == 720);
        REQUIRE(config.get<std::string>("window.title", "") == "Test Window");
        REQUIRE(config.get<bool>("window.fullscreen", true) == false);

        REQUIRE_THAT(config.get<float>("camera.zoom", 0.0f),
                    Catch::Matchers::WithinAbs(0.5f, 0.001f));
        REQUIRE_THAT(config.get<double>("camera.speed", 0.0),
                    Catch::Matchers::WithinAbs(600.0, 0.001));

        REQUIRE(config.get<int>("game.targetFPS", 0) == 60);
        REQUIRE(config.get<bool>("game.debug", false) == true);

        // Cleanup
        fs::remove_all("test_config");
    }

    SECTION("Load non-existent file creates default") {
        std::string nonExistentPath = "test_config/new_config.yaml";

        // Ensure file doesn't exist
        fs::remove(nonExistentPath);

        bool loaded = config.load(nonExistentPath);
        REQUIRE(loaded == true);  // Should create default config

        // File should now exist
        REQUIRE(fs::exists(nonExistentPath) == true);

        // Cleanup
        fs::remove_all("test_config");
    }

    SECTION("Save config file") {
        std::string savePath = "test_config/save_test.yaml";

        config.set("window.width", 1920);
        config.set("window.height", 1080);
        config.set("camera.zoom", 0.75f);
        config.set("game.debug", true);

        bool saved = config.save(savePath);
        REQUIRE(saved == true);

        // Verify file exists
        REQUIRE(fs::exists(savePath) == true);

        // Load it back and verify
        Config& config2 = Config::getInstance();
        config2.load(savePath);

        REQUIRE(config2.get<int>("window.width", 0) == 1920);
        REQUIRE(config2.get<int>("window.height", 0) == 1080);
        REQUIRE_THAT(config2.get<float>("camera.zoom", 0.0f),
                    Catch::Matchers::WithinAbs(0.75f, 0.001f));
        REQUIRE(config2.get<bool>("game.debug", false) == true);

        // Cleanup
        fs::remove_all("test_config");
    }
}

TEST_CASE("Config: Get with default values", "[Config]") {
    Config& config = Config::getInstance();

    std::string testPath = "test_config/defaults.yaml";
    std::string yamlContent = R"(
test:
  existing_int: 42
  existing_string: "hello"
  existing_bool: true
)";
    createTestConfigFile(testPath, yamlContent);
    config.load(testPath);

    SECTION("Get existing values") {
        REQUIRE(config.get<int>("test.existing_int", 0) == 42);
        REQUIRE(config.get<std::string>("test.existing_string", "") == "hello");
        REQUIRE(config.get<bool>("test.existing_bool", false) == true);
    }

    SECTION("Get non-existent values returns default") {
        REQUIRE(config.get<int>("test.non_existent_int", 999) == 999);
        REQUIRE(config.get<std::string>("test.non_existent_string", "default") == "default");
        REQUIRE(config.get<bool>("test.non_existent_bool", false) == false);
    }

    SECTION("Get non-existent section returns default") {
        REQUIRE(config.get<int>("non_existent_section.value", 123) == 123);
    }

    // Cleanup
    fs::remove_all("test_config");
}

TEST_CASE("Config: Set values", "[Config]") {
    Config& config = Config::getInstance();

    std::string testPath = "test_config/set_test.yaml";
    config.load(testPath);  // Create fresh config

    SECTION("Set various types") {
        config.set("test.int_value", 100);
        config.set("test.float_value", 3.14f);
        config.set("test.double_value", 2.718);
        config.set("test.string_value", std::string("test string"));
        config.set("test.bool_value", true);

        REQUIRE(config.get<int>("test.int_value", 0) == 100);
        REQUIRE_THAT(config.get<float>("test.float_value", 0.0f),
                    Catch::Matchers::WithinAbs(3.14f, 0.001f));
        REQUIRE_THAT(config.get<double>("test.double_value", 0.0),
                    Catch::Matchers::WithinAbs(2.718, 0.001));
        REQUIRE(config.get<std::string>("test.string_value", "") == "test string");
        REQUIRE(config.get<bool>("test.bool_value", false) == true);
    }

    SECTION("Overwrite existing values") {
        config.set("test.value", 10);
        REQUIRE(config.get<int>("test.value", 0) == 10);

        config.set("test.value", 20);
        REQUIRE(config.get<int>("test.value", 0) == 20);
    }

    SECTION("Set values without section (root level)") {
        config.set("root_value", 42);
        REQUIRE(config.get<int>("root_value", 0) == 42);
    }

    // Cleanup
    fs::remove_all("test_config");
}

TEST_CASE("Config: Has key check", "[Config]") {
    Config& config = Config::getInstance();

    std::string testPath = "test_config/has_test.yaml";
    std::string yamlContent = R"(
section1:
  key1: "value1"
  key2: 123

section2:
  nested:
    deep: true
)";
    createTestConfigFile(testPath, yamlContent);
    config.load(testPath);

    SECTION("Check existing keys") {
        REQUIRE(config.has("section1.key1") == true);
        REQUIRE(config.has("section1.key2") == true);
    }

    SECTION("Check non-existent keys") {
        REQUIRE(config.has("section1.non_existent") == false);
        REQUIRE(config.has("non_existent_section.key") == false);
    }

    SECTION("Check root level keys") {
        config.set("root_key", 42);
        REQUIRE(config.has("root_key") == true);
    }

    // Cleanup
    fs::remove_all("test_config");
}

TEST_CASE("Config: Get config path", "[Config]") {
    Config& config = Config::getInstance();

    std::string testPath = "test_config/path_test.yaml";
    config.load(testPath);

    REQUIRE(config.getConfigPath() == testPath);

    // Cleanup
    fs::remove_all("test_config");
}

TEST_CASE("Config: Type conversions", "[Config]") {
    Config& config = Config::getInstance();

    std::string testPath = "test_config/type_test.yaml";
    std::string yamlContent = R"(
types:
  string_number: "42"
  int_value: 100
  float_value: 3.14
)";
    createTestConfigFile(testPath, yamlContent);
    config.load(testPath);

    SECTION("Get int as different types") {
        REQUIRE(config.get<int>("types.int_value", 0) == 100);
        REQUIRE_THAT(config.get<float>("types.int_value", 0.0f),
                    Catch::Matchers::WithinAbs(100.0f, 0.001f));
        REQUIRE_THAT(config.get<double>("types.int_value", 0.0),
                    Catch::Matchers::WithinAbs(100.0, 0.001));
    }

    SECTION("Get float as different types") {
        REQUIRE_THAT(config.get<float>("types.float_value", 0.0f),
                    Catch::Matchers::WithinAbs(3.14f, 0.001f));
        REQUIRE_THAT(config.get<double>("types.float_value", 0.0),
                    Catch::Matchers::WithinAbs(3.14, 0.001));
    }

    SECTION("String to int conversion works") {
        // YAML-cpp is smart enough to convert string "42" to int 42
        REQUIRE(config.get<int>("types.string_number", 999) == 42);
    }

    // Cleanup
    fs::remove_all("test_config");
}

TEST_CASE("Config: Multiple sections", "[Config]") {
    Config& config = Config::getInstance();

    std::string testPath = "test_config/multi_section.yaml";

    config.load(testPath);

    config.set("window.width", 1920);
    config.set("window.height", 1080);
    config.set("camera.zoom", 0.5f);
    config.set("camera.speed", 500.0f);
    config.set("game.fps", 60);

    REQUIRE(config.get<int>("window.width", 0) == 1920);
    REQUIRE(config.get<int>("window.height", 0) == 1080);
    REQUIRE_THAT(config.get<float>("camera.zoom", 0.0f),
                Catch::Matchers::WithinAbs(0.5f, 0.001f));
    REQUIRE_THAT(config.get<float>("camera.speed", 0.0f),
                Catch::Matchers::WithinAbs(500.0f, 0.001f));
    REQUIRE(config.get<int>("game.fps", 0) == 60);

    // Cleanup
    fs::remove_all("test_config");
}
