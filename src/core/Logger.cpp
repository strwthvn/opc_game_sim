#include "core/Logger.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <filesystem>

namespace core {

std::shared_ptr<spdlog::logger> Logger::s_logger;

void Logger::initialize(bool logToFile, const std::string& logFilePath) {
    std::vector<spdlog::sink_ptr> sinks;

    // Консольный sink с цветным выводом
    auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    consoleSink->set_pattern("[%T.%e] [%^%l%$] [%n] %v");
    sinks.push_back(consoleSink);

    // Файловый sink с ротацией (опционально)
    if (logToFile) {
        // Создаем директорию для логов если её нет
        std::filesystem::path logPath(logFilePath);
        std::filesystem::create_directories(logPath.parent_path());

        // Ротация: максимум 5 МБ на файл, до 3 файлов
        auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            logFilePath, LoggerConstants::LOG_FILE_SIZE, LoggerConstants::LOG_FILE_COUNT);
        fileSink->set_pattern("[%Y-%m-%d %T.%e] [%l] [%n] %v");
        sinks.push_back(fileSink);
    }

    // Создаем логгер с несколькими sinks
    s_logger = std::make_shared<spdlog::logger>("OPC_GAME_SIM", sinks.begin(), sinks.end());
    s_logger->set_level(spdlog::level::debug);
    s_logger->flush_on(spdlog::level::err);

    // Регистрируем как глобальный логгер
    spdlog::register_logger(s_logger);

    LOG_INFO("Logger initialized successfully");
}

void Logger::shutdown() {
    if (s_logger) {
        LOG_INFO("Shutting down logger");
        s_logger->flush();
        spdlog::drop_all();
        s_logger.reset();
    }
}

std::shared_ptr<spdlog::logger>& Logger::getLogger() {
    if (!s_logger) {
        // Автоматическая инициализация если забыли вызвать initialize()
        initialize();
    }
    return s_logger;
}

void Logger::setLevel(spdlog::level::level_enum level) {
    if (s_logger) {
        s_logger->set_level(level);
    }
}

} // namespace core
