#include "core/Config.h"
#include "core/Logger.h"
#include <fstream>
#include <filesystem>

namespace core {

Config& Config::getInstance() {
    static Config instance;
    return instance;
}

bool Config::load(const std::string& path) {
    m_configPath = path;

    try {
        // Проверяем существование файла
        if (!std::filesystem::exists(path)) {
            // Не логируем здесь - логгер может быть не инициализирован
            createDefaultConfig();
            return save(path);  // Сохраняем дефолтный конфиг
        }

        // Загружаем YAML
        m_data = YAML::LoadFile(path);
        // Не логируем здесь - логгер может быть не инициализирован
        return true;

    } catch (const YAML::Exception& e) {
        // Не логируем здесь - логгер может быть не инициализирован
        createDefaultConfig();
        return false;
    } catch (const std::exception& e) {
        // Не логируем здесь - логгер может быть не инициализирован
        createDefaultConfig();
        return false;
    }
}

bool Config::save(const std::string& path) {
    try {
        // Создаем директорию если не существует
        std::filesystem::path filePath(path);
        if (filePath.has_parent_path()) {
            std::filesystem::create_directories(filePath.parent_path());
        }

        // Сохраняем YAML в файл
        std::ofstream file(path);
        if (!file.is_open()) {
            // Не логируем здесь - логгер может быть не инициализирован
            return false;
        }

        file << m_data;
        file.close();

        // Не логируем здесь - логгер может быть не инициализирован
        return true;

    } catch (const std::exception& e) {
        // Не логируем здесь - логгер может быть не инициализирован
        return false;
    }
}

bool Config::has(const std::string& key) const {
    size_t dotPos = key.find('.');
    if (dotPos == std::string::npos) {
        return m_data[key].IsDefined();
    }

    std::string section = key.substr(0, dotPos);
    std::string field = key.substr(dotPos + 1);

    return m_data[section] && m_data[section][field].IsDefined();
}

void Config::createDefaultConfig() {
    // Не логируем здесь - логгер может быть не инициализирован

    // Window settings
    m_data["window"]["width"] = 1280;
    m_data["window"]["height"] = 720;
    m_data["window"]["title"] = "OPC Game Simulator";
    m_data["window"]["vsync"] = true;
    m_data["window"]["frameRateLimit"] = 60;

    // Camera settings
    m_data["camera"]["moveSpeed"] = 600.0;
    m_data["camera"]["zoomSpeed"] = 0.1;
    m_data["camera"]["minZoom"] = 0.2;
    m_data["camera"]["maxZoom"] = 1.0;
    m_data["camera"]["defaultZoom"] = 0.5;

    // Game settings
    m_data["game"]["fixedTimestep"] = 0.01666667;  // ~60 UPS
    m_data["game"]["maxFrameTime"] = 0.25;
    m_data["game"]["metricsLogInterval"] = 5.0;

    // Audio settings
    m_data["audio"]["masterVolume"] = 100;
    m_data["audio"]["musicVolume"] = 80;
    m_data["audio"]["sfxVolume"] = 100;

    // Logging settings
    m_data["logging"]["enabled"] = true;
    m_data["logging"]["logToFile"] = true;
    m_data["logging"]["logFilePath"] = "logs/opc_game_sim.log";
}

} // namespace core
