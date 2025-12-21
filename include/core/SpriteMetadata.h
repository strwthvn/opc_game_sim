#pragma once

#include <string>
#include <vector>
#include <optional>
#include <nlohmann/json.hpp>

namespace core {

/**
 * @brief Структура для хранения одного кадра анимации
 */
struct AnimationFrame {
    int x = 0;              ///< X координата кадра в текстуре (пиксели)
    int y = 0;              ///< Y координата кадра в текстуре (пиксели)
    int duration = 100;     ///< Длительность кадра (миллисекунды)

    /**
     * @brief Создает AnimationFrame из JSON
     */
    static AnimationFrame fromJson(const nlohmann::json& json);

    /**
     * @brief Конвертирует AnimationFrame в JSON
     */
    nlohmann::json toJson() const;
};

/**
 * @brief Структура для хранения информации об анимации
 */
struct AnimationInfo {
    std::string name;                   ///< Имя анимации (например, "running", "idle")
    std::vector<AnimationFrame> frames; ///< Кадры анимации
    bool loop = true;                   ///< Зацикливать анимацию

    /**
     * @brief Создает AnimationInfo из JSON
     */
    static AnimationInfo fromJson(const nlohmann::json& json);

    /**
     * @brief Конвертирует AnimationInfo в JSON
     */
    nlohmann::json toJson() const;
};

/**
 * @brief Размер спрайта/текстуры
 */
struct SpriteSize {
    int width = 32;     ///< Ширина спрайта (пиксели)
    int height = 32;    ///< Высота спрайта (пиксели)

    /**
     * @brief Создает SpriteSize из JSON
     */
    static SpriteSize fromJson(const nlohmann::json& json);

    /**
     * @brief Конвертирует SpriteSize в JSON
     */
    nlohmann::json toJson() const;
};

/**
 * @brief Точка origin для спрайта
 */
struct SpriteOrigin {
    int x = 0;      ///< X координата origin (пиксели)
    int y = 32;     ///< Y координата origin (пиксели, по умолчанию bottom-left)

    /**
     * @brief Создает SpriteOrigin из JSON
     */
    static SpriteOrigin fromJson(const nlohmann::json& json);

    /**
     * @brief Конвертирует SpriteOrigin в JSON
     */
    nlohmann::json toJson() const;
};

/**
 * @brief Метаданные спрайта
 *
 * Содержит всю информацию о спрайте: размер, автор, версию, анимации и т.д.
 * Загружается из JSON файлов (.sprite.json).
 *
 * Пример использования:
 * @code
 * auto metadata = SpriteMetadata::loadFromFile("assets/sprites/conveyor.sprite.json");
 * if (metadata) {
 *     auto& texture = resourceManager.getTexture(metadata->texturePath);
 *     auto anim = metadata->getAnimation("running");
 * }
 * @endcode
 */
class SpriteMetadata {
public:
    /**
     * @brief Конструктор по умолчанию
     */
    SpriteMetadata() = default;

    /**
     * @brief Загружает метаданные из JSON файла
     * @param path Путь к .sprite.json файлу
     * @return SpriteMetadata если успешно, std::nullopt если ошибка
     */
    static std::optional<SpriteMetadata> loadFromFile(const std::string& path);

    /**
     * @brief Загружает метаданные из JSON объекта
     * @param json JSON объект с метаданными
     * @return SpriteMetadata если успешно, std::nullopt если ошибка
     */
    static std::optional<SpriteMetadata> loadFromJson(const nlohmann::json& json);

    /**
     * @brief Сохраняет метаданные в JSON файл
     * @param path Путь для сохранения .sprite.json файла
     * @return true если успешно сохранено
     */
    bool saveToFile(const std::string& path) const;

    /**
     * @brief Конвертирует метаданные в JSON объект
     * @return JSON представление метаданных
     */
    nlohmann::json toJson() const;

    // ========== Геттеры ==========

    /**
     * @brief Возвращает имя спрайта
     */
    const std::string& getName() const { return m_name; }

    /**
     * @brief Возвращает автора спрайта
     */
    const std::string& getAuthor() const { return m_author; }

    /**
     * @brief Возвращает версию спрайта
     */
    const std::string& getVersion() const { return m_version; }

    /**
     * @brief Возвращает дату создания
     */
    const std::string& getCreated() const { return m_created; }

    /**
     * @brief Возвращает описание спрайта
     */
    const std::string& getDescription() const { return m_description; }

    /**
     * @brief Возвращает путь к файлу текстуры (относительно .sprite.json)
     */
    const std::string& getTexturePath() const { return m_texturePath; }

    /**
     * @brief Возвращает размер спрайта
     */
    const SpriteSize& getSize() const { return m_size; }

    /**
     * @brief Возвращает origin спрайта
     */
    const SpriteOrigin& getOrigin() const { return m_origin; }

    /**
     * @brief Возвращает теги спрайта
     */
    const std::vector<std::string>& getTags() const { return m_tags; }

    /**
     * @brief Возвращает все анимации
     */
    const std::vector<AnimationInfo>& getAnimations() const { return m_animations; }

    /**
     * @brief Ищет анимацию по имени
     * @param name Имя анимации
     * @return Указатель на анимацию или nullptr если не найдена
     */
    const AnimationInfo* getAnimation(const std::string& name) const;

    /**
     * @brief Проверяет наличие анимации с заданным именем
     */
    bool hasAnimation(const std::string& name) const;

    /**
     * @brief Проверяет наличие тега
     */
    bool hasTag(const std::string& tag) const;

    // ========== Сеттеры ==========

    void setName(const std::string& name) { m_name = name; }
    void setAuthor(const std::string& author) { m_author = author; }
    void setVersion(const std::string& version) { m_version = version; }
    void setCreated(const std::string& created) { m_created = created; }
    void setDescription(const std::string& description) { m_description = description; }
    void setTexturePath(const std::string& path) { m_texturePath = path; }
    void setSize(const SpriteSize& size) { m_size = size; }
    void setOrigin(const SpriteOrigin& origin) { m_origin = origin; }
    void setTags(const std::vector<std::string>& tags) { m_tags = tags; }
    void setAnimations(const std::vector<AnimationInfo>& animations) { m_animations = animations; }

    /**
     * @brief Добавляет тег
     */
    void addTag(const std::string& tag);

    /**
     * @brief Добавляет анимацию
     */
    void addAnimation(const AnimationInfo& animation);

private:
    std::string m_name;                         ///< Имя спрайта
    std::string m_author;                       ///< Автор спрайта
    std::string m_version = "1.0";              ///< Версия спрайта
    std::string m_created;                      ///< Дата создания (ISO 8601)
    std::string m_description;                  ///< Описание спрайта
    std::string m_texturePath;                  ///< Путь к файлу текстуры
    SpriteSize m_size;                          ///< Размер спрайта
    SpriteOrigin m_origin;                      ///< Origin спрайта
    std::vector<std::string> m_tags;            ///< Теги для категоризации
    std::vector<AnimationInfo> m_animations;    ///< Анимации спрайта
};

} // namespace core
