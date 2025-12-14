#pragma once

#include <string>
#include <yaml-cpp/yaml.h>

namespace core {

/**
 * @brief Singleton класс для управления конфигурацией приложения
 *
 * Загружает и сохраняет настройки приложения из/в YAML файл.
 * Используется для вынесения захардкоженных констант в конфигурационный файл.
 *
 * Поддерживаемые секции:
 * - window: настройки окна
 * - camera: настройки камеры
 * - game: настройки игрового цикла
 * - audio: настройки звука
 */
class Config {
public:
    /**
     * @brief Получает единственный экземпляр Config
     * @return Ссылка на синглтон Config
     */
    static Config& getInstance();

    /**
     * @brief Загружает конфигурацию из YAML файла
     * @param path Путь к конфигурационному файлу
     * @return true если загрузка успешна, false в противном случае
     *
     * Если файл не существует, создается с дефолтными значениями.
     */
    bool load(const std::string& path);

    /**
     * @brief Сохраняет конфигурацию в YAML файл
     * @param path Путь к конфигурационному файлу
     * @return true если сохранение успешно, false в противном случае
     */
    bool save(const std::string& path);

    /**
     * @brief Получает значение из конфигурации
     * @tparam T Тип значения
     * @param key Ключ в формате "section.key" (например, "window.width")
     * @param defaultValue Значение по умолчанию если ключ не найден
     * @return Значение из конфигурации или defaultValue
     */
    template<typename T>
    T get(const std::string& key, const T& defaultValue) const;

    /**
     * @brief Устанавливает значение в конфигурации
     * @tparam T Тип значения
     * @param key Ключ в формате "section.key" (например, "window.width")
     * @param value Новое значение
     */
    template<typename T>
    void set(const std::string& key, const T& value);

    /**
     * @brief Проверяет существование ключа в конфигурации
     * @param key Ключ в формате "section.key"
     * @return true если ключ существует
     */
    bool has(const std::string& key) const;

    /**
     * @brief Получает путь к текущему конфигурационному файлу
     * @return Путь к файлу
     */
    std::string getConfigPath() const { return m_configPath; }

private:
    Config() = default;
    ~Config() = default;

    // Запрет копирования и перемещения
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
    Config(Config&&) = delete;
    Config& operator=(Config&&) = delete;

    /**
     * @brief Создает конфигурацию с дефолтными значениями
     */
    void createDefaultConfig();

    YAML::Node m_data;              ///< YAML данные конфигурации
    std::string m_configPath;       ///< Путь к конфигурационному файлу
};

// ============================================================
// Template implementations
// ============================================================

template<typename T>
T Config::get(const std::string& key, const T& defaultValue) const {
    try {
        // Разбиваем ключ на секцию и поле
        size_t dotPos = key.find('.');
        if (dotPos == std::string::npos) {
            // Ключ без секции - ищем в корне
            if (m_data[key]) {
                return m_data[key].as<T>();
            }
            return defaultValue;
        }

        std::string section = key.substr(0, dotPos);
        std::string field = key.substr(dotPos + 1);

        if (m_data[section] && m_data[section][field]) {
            return m_data[section][field].as<T>();
        }
    } catch (const YAML::Exception&) {
        // Ошибка преобразования типа или парсинга - возвращаем дефолт
    }

    return defaultValue;
}

template<typename T>
void Config::set(const std::string& key, const T& value) {
    // Разбиваем ключ на секцию и поле
    size_t dotPos = key.find('.');
    if (dotPos == std::string::npos) {
        // Ключ без секции - устанавливаем в корне
        m_data[key] = value;
        return;
    }

    std::string section = key.substr(0, dotPos);
    std::string field = key.substr(dotPos + 1);

    m_data[section][field] = value;
}

} // namespace core
