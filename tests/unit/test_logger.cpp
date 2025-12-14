/**
 * @file test_logger.cpp
 * @brief Unit tests for Logger system
 */

#include <catch2/catch_test_macros.hpp>
#include <core/Logger.h>
#include <spdlog/sinks/ostream_sink.h>
#include <filesystem>
#include <sstream>
#include <fstream>

using namespace core;
namespace fs = std::filesystem;

// Helper function to ensure clean logger state
static void ensureCleanLoggerState() {
    Logger::shutdown();  // Always shutdown first to clear any existing logger
}

TEST_CASE("Logger: Initialization and shutdown", "[Logger]") {
    ensureCleanLoggerState();
    SECTION("Initialize logger") {
        std::string logPath = "test_logs/test.log";

        Logger::initialize(true, logPath);

        auto logger = Logger::getLogger();
        REQUIRE(logger != nullptr);

        Logger::shutdown();

        // Cleanup
        fs::remove_all("test_logs");
    }

    SECTION("Initialize without file logging") {
        Logger::initialize(false);

        auto logger = Logger::getLogger();
        REQUIRE(logger != nullptr);

        Logger::shutdown();
    }
}

TEST_CASE("Logger: Log level setting", "[Logger]") {
    ensureCleanLoggerState();
    Logger::initialize(false);

    SECTION("Set different log levels") {
        Logger::setLevel(spdlog::level::trace);
        REQUIRE(Logger::getLogger()->level() == spdlog::level::trace);

        Logger::setLevel(spdlog::level::debug);
        REQUIRE(Logger::getLogger()->level() == spdlog::level::debug);

        Logger::setLevel(spdlog::level::info);
        REQUIRE(Logger::getLogger()->level() == spdlog::level::info);

        Logger::setLevel(spdlog::level::warn);
        REQUIRE(Logger::getLogger()->level() == spdlog::level::warn);

        Logger::setLevel(spdlog::level::err);
        REQUIRE(Logger::getLogger()->level() == spdlog::level::err);

        Logger::setLevel(spdlog::level::critical);
        REQUIRE(Logger::getLogger()->level() == spdlog::level::critical);
    }

    Logger::shutdown();
}

TEST_CASE("Logger: Logging messages", "[Logger]") {
    ensureCleanLoggerState();

    // Create a string stream sink to capture log output
    auto oss = std::make_shared<std::ostringstream>();
    auto ostream_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(*oss);

    Logger::initialize(false);
    Logger::getLogger()->sinks().clear();
    Logger::getLogger()->sinks().push_back(ostream_sink);
    Logger::setLevel(spdlog::level::trace);

    SECTION("Trace level") {
        LOG_TRACE("Trace message: {}", 123);
        std::string output = oss->str();
        REQUIRE(output.find("Trace message: 123") != std::string::npos);
    }

    SECTION("Debug level") {
        oss->str("");  // Clear previous output
        LOG_DEBUG("Debug message: {}", "test");
        std::string output = oss->str();
        REQUIRE(output.find("Debug message: test") != std::string::npos);
    }

    SECTION("Info level") {
        oss->str("");
        LOG_INFO("Info message");
        std::string output = oss->str();
        REQUIRE(output.find("Info message") != std::string::npos);
    }

    SECTION("Warning level") {
        oss->str("");
        LOG_WARN("Warning message");
        std::string output = oss->str();
        REQUIRE(output.find("Warning message") != std::string::npos);
    }

    SECTION("Error level") {
        oss->str("");
        LOG_ERROR("Error message");
        std::string output = oss->str();
        REQUIRE(output.find("Error message") != std::string::npos);
    }

    SECTION("Critical level") {
        oss->str("");
        LOG_CRITICAL("Critical message");
        std::string output = oss->str();
        REQUIRE(output.find("Critical message") != std::string::npos);
    }

    Logger::shutdown();
}

TEST_CASE("Logger: Log level filtering", "[Logger]") {
    ensureCleanLoggerState();

    auto oss = std::make_shared<std::ostringstream>();
    auto ostream_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(*oss);

    Logger::initialize(false);
    Logger::getLogger()->sinks().clear();
    Logger::getLogger()->sinks().push_back(ostream_sink);

    SECTION("Warn level filters out debug and trace") {
        Logger::setLevel(spdlog::level::warn);

        LOG_TRACE("Trace");
        LOG_DEBUG("Debug");
        LOG_INFO("Info");
        LOG_WARN("Warning");
        LOG_ERROR("Error");

        std::string output = oss->str();

        REQUIRE(output.find("Trace") == std::string::npos);
        REQUIRE(output.find("Debug") == std::string::npos);
        REQUIRE(output.find("Info") == std::string::npos);
        REQUIRE(output.find("Warning") != std::string::npos);
        REQUIRE(output.find("Error") != std::string::npos);
    }

    SECTION("Error level filters out everything except error and critical") {
        oss->str("");
        Logger::setLevel(spdlog::level::err);

        LOG_DEBUG("Debug");
        LOG_INFO("Info");
        LOG_WARN("Warning");
        LOG_ERROR("Error");
        LOG_CRITICAL("Critical");

        std::string output = oss->str();

        REQUIRE(output.find("Debug") == std::string::npos);
        REQUIRE(output.find("Info") == std::string::npos);
        REQUIRE(output.find("Warning") == std::string::npos);
        REQUIRE(output.find("Error") != std::string::npos);
        REQUIRE(output.find("Critical") != std::string::npos);
    }

    Logger::shutdown();
}

TEST_CASE("Logger: File logging", "[Logger]") {
    ensureCleanLoggerState();

    std::string logPath = "test_logs/file_test.log";

    // Ensure directory is clean
    fs::remove_all("test_logs");

    Logger::initialize(true, logPath);
    Logger::setLevel(spdlog::level::info);

    LOG_INFO("Test message 1");
    LOG_WARN("Test message 2");
    LOG_ERROR("Test message 3");

    Logger::shutdown();

    // Verify log file exists and contains messages
    REQUIRE(fs::exists(logPath) == true);

    std::ifstream logFile(logPath);
    std::string content((std::istreambuf_iterator<char>(logFile)),
                        std::istreambuf_iterator<char>());

    REQUIRE(content.find("Test message 1") != std::string::npos);
    REQUIRE(content.find("Test message 2") != std::string::npos);
    REQUIRE(content.find("Test message 3") != std::string::npos);

    // Cleanup
    fs::remove_all("test_logs");
}

TEST_CASE("Logger: Formatted logging", "[Logger]") {
    ensureCleanLoggerState();

    auto oss = std::make_shared<std::ostringstream>();
    auto ostream_sink = std::make_shared<spdlog::sinks::ostream_sink_mt>(*oss);

    Logger::initialize(false);
    Logger::getLogger()->sinks().clear();
    Logger::getLogger()->sinks().push_back(ostream_sink);
    Logger::setLevel(spdlog::level::info);

    SECTION("Format with multiple arguments") {
        LOG_INFO("Player {} at position ({}, {})", "Hero", 100, 200);
        std::string output = oss->str();
        REQUIRE(output.find("Player Hero at position (100, 200)") != std::string::npos);
    }

    SECTION("Format with floating point") {
        oss->str("");
        LOG_INFO("FPS: {:.2f}", 59.94f);
        std::string output = oss->str();
        REQUIRE(output.find("FPS: 59.94") != std::string::npos);
    }

    SECTION("Format with string and numbers") {
        oss->str("");
        LOG_ERROR("Failed to load texture '{}' (error code: {})", "sprite.png", 404);
        std::string output = oss->str();
        REQUIRE(output.find("Failed to load texture 'sprite.png' (error code: 404)") != std::string::npos);
    }

    Logger::shutdown();
}

TEST_CASE("Logger: Singleton behavior", "[Logger]") {
    ensureCleanLoggerState();
    Logger::initialize(false);

    auto logger1 = Logger::getLogger();
    auto logger2 = Logger::getLogger();

    // Should return the same logger instance
    REQUIRE(logger1 == logger2);
    REQUIRE(logger1.get() == logger2.get());

    Logger::shutdown();
}

TEST_CASE("Logger: Constants", "[Logger]") {
    using namespace LoggerConstants;

    SECTION("Log file size constant") {
        REQUIRE(LOG_FILE_SIZE == 1024 * 1024 * 5);  // 5 MB
    }

    SECTION("Log file count constant") {
        REQUIRE(LOG_FILE_COUNT == 3);
    }
}
