#pragma once

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <unordered_map>
#include <string>
#include <memory>
#include <vector>
#include <functional>

namespace core {

/**
 * @brief Менеджер ресурсов приложения
 *
 * Централизованное управление ресурсами (шрифты, текстуры, звуки).
 * Предотвращает дублирование ресурсов в памяти и упрощает загрузку.
 *
 * Использование:
 * @code
 * auto& font = resourceManager.getFont("default");
 * auto& texture = resourceManager.getTexture("player.png");
 * @endcode
 */
class ResourceManager {
public:
    /**
     * @brief Конструктор
     */
    ResourceManager();

    /**
     * @brief Деструктор
     */
    ~ResourceManager() = default;

    // Запрет копирования и перемещения
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;
    ResourceManager(ResourceManager&&) = delete;
    ResourceManager& operator=(ResourceManager&&) = delete;

    /**
     * @brief Получает шрифт по имени, загружает если еще не загружен
     * @param name Имя шрифта ("default", "title", и т.д.)
     * @return Ссылка на шрифт
     * @throws std::runtime_error если шрифт не удалось загрузить
     */
    const sf::Font& getFont(const std::string& name);

    /**
     * @brief Получает текстуру по пути, загружает если еще не загружена
     * @param path Путь к текстуре относительно assets/textures/
     * @return Ссылка на текстуру
     * @throws std::runtime_error если текстуру не удалось загрузить
     */
    const sf::Texture& getTexture(const std::string& path);

    /**
     * @brief Загружает шрифт из файла
     * @param name Имя шрифта для идентификации
     * @param path Путь к файлу шрифта
     * @return true если успешно загружено
     */
    bool loadFont(const std::string& name, const std::string& path);

    /**
     * @brief Загружает текстуру из файла
     * @param name Имя текстуры для идентификации
     * @param path Путь к файлу текстуры
     * @return true если успешно загружено
     */
    bool loadTexture(const std::string& name, const std::string& path);

    /**
     * @brief Загружает текстуру из файла (имя = путь)
     * @param path Путь к файлу текстуры (также используется как имя)
     * @return true если успешно загружено
     */
    bool loadTexture(const std::string& path);

    /**
     * @brief Загружает текстуру из SFML Image
     * @param name Имя текстуры для идентификации
     * @param image Изображение для загрузки
     * @return true если успешно загружено
     */
    bool loadTextureFromImage(const std::string& name, const sf::Image& image);

    /**
     * @brief Предзагружает несколько текстур
     *
     * Полезно для экрана загрузки - загружает все текстуры заранее.
     *
     * @param paths Вектор путей к текстурам
     * @param progressCallback Опциональный callback для отслеживания прогресса (0.0-1.0)
     * @return Количество успешно загруженных текстур
     */
    size_t preloadTextures(const std::vector<std::string>& paths,
                          std::function<void(float)> progressCallback = nullptr);

    /**
     * @brief Предзагружает несколько шрифтов
     *
     * Полезно для экрана загрузки - загружает все шрифты заранее.
     *
     * @param fontConfigs Вектор пар {имя, путь} для шрифтов
     * @param progressCallback Опциональный callback для отслеживания прогресса (0.0-1.0)
     * @return Количество успешно загруженных шрифтов
     */
    size_t preloadFonts(const std::vector<std::pair<std::string, std::string>>& fontConfigs,
                       std::function<void(float)> progressCallback = nullptr);

    /**
     * @brief Проверяет, загружен ли шрифт
     * @param name Имя шрифта
     * @return true если шрифт загружен
     */
    bool hasFont(const std::string& name) const;

    /**
     * @brief Проверяет, загружена ли текстура
     * @param name Имя текстуры
     * @return true если текстура загружена
     */
    bool hasTexture(const std::string& name) const;

    /**
     * @brief Очищает все загруженные ресурсы
     */
    void clear();

    /**
     * @brief Возвращает количество загруженных шрифтов
     */
    size_t getFontCount() const { return m_fonts.size(); }

    /**
     * @brief Возвращает количество загруженных текстур
     */
    size_t getTextureCount() const { return m_textures.size(); }

private:
    /**
     * @brief Загружает системный шрифт по умолчанию
     * @return Путь к системному шрифту или пустую строку
     */
    std::string getSystemFontPath() const;

    std::unordered_map<std::string, sf::Font> m_fonts;       ///< Кеш шрифтов
    std::unordered_map<std::string, sf::Texture> m_textures; ///< Кеш текстур
};

} // namespace core
