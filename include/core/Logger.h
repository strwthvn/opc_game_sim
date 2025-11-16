#pragma once

#include <spdlog/spdlog.h>
#include <memory>
#include <string>

namespace core {

/**
 * @brief Константы логирования
 */
namespace LoggerConstants {
    /// Максимальный размер файла лога (5 МБ)
    constexpr size_t LOG_FILE_SIZE = 1024 * 1024 * 5;

    /// Максимальное количество файлов логов
    constexpr size_t LOG_FILE_COUNT = 3;
}

/**
 * @brief Система логирования приложения на основе spdlog
 *
 * Предоставляет централизованную систему логирования с поддержкой:
 * - Различных уровней логирования (trace, debug, info, warn, error, critical)
 * - Цветного вывода в консоль
 * - Логирования в файл с ротацией
 * - Форматирования сообщений
 */
class Logger {
public:
    /**
     * @brief Инициализирует систему логирования
     * @param logToFile Включить логирование в файл
     * @param logFilePath Путь к файлу логов
     */
    static void initialize(bool logToFile = true,
                          const std::string& logFilePath = "logs/opc_game_sim.log");

    /**
     * @brief Завершает работу системы логирования
     */
    static void shutdown();

    /**
     * @brief Возвращает логгер приложения
     * @return Shared pointer на logger
     */
    static std::shared_ptr<spdlog::logger>& getLogger();

    /**
     * @brief Устанавливает уровень логирования
     * @param level Уровень логирования
     */
    static void setLevel(spdlog::level::level_enum level);

private:
    static std::shared_ptr<spdlog::logger> s_logger;
};

} // namespace core

// Макросы для удобного логирования
#define LOG_TRACE(...)    ::core::Logger::getLogger()->trace(__VA_ARGS__)
#define LOG_DEBUG(...)    ::core::Logger::getLogger()->debug(__VA_ARGS__)
#define LOG_INFO(...)     ::core::Logger::getLogger()->info(__VA_ARGS__)
#define LOG_WARN(...)     ::core::Logger::getLogger()->warn(__VA_ARGS__)
#define LOG_ERROR(...)    ::core::Logger::getLogger()->error(__VA_ARGS__)
#define LOG_CRITICAL(...) ::core::Logger::getLogger()->critical(__VA_ARGS__)
