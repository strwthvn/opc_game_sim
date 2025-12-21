#pragma once

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Audio/SoundBuffer.hpp>
#include "core/SpriteMetadata.h"
#include <unordered_map>
#include <string>
#include <memory>
#include <vector>
#include <functional>
#include <future>
#include <mutex>
#include <atomic>

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
     * @brief Получает звуковой буфер по имени, загружает если еще не загружен
     * @param name Имя звука для идентификации
     * @return Ссылка на звуковой буфер
     * @throws std::runtime_error если буфер не удалось загрузить
     */
    const sf::SoundBuffer& getSound(const std::string& name);

    /**
     * @brief Загружает звуковой буфер из файла
     * @param name Имя звука для идентификации
     * @param path Путь к файлу звука
     * @return true если успешно загружено
     */
    bool loadSound(const std::string& name, const std::string& path);

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
     * @brief Предзагружает несколько звуков
     *
     * Полезно для экрана загрузки - загружает все звуки заранее.
     *
     * @param soundConfigs Вектор пар {имя, путь} для звуков
     * @param progressCallback Опциональный callback для отслеживания прогресса (0.0-1.0)
     * @return Количество успешно загруженных звуков
     */
    size_t preloadSounds(const std::vector<std::pair<std::string, std::string>>& soundConfigs,
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
     * @brief Проверяет, загружен ли звук
     * @param name Имя звука
     * @return true если звук загружен
     */
    bool hasSound(const std::string& name) const;

    /**
     * @brief Выгружает отдельную текстуру
     * @param name Имя текстуры
     * @return true если текстура была выгружена, false если не найдена
     */
    bool unloadTexture(const std::string& name);

    /**
     * @brief Выгружает отдельный шрифт
     * @param name Имя шрифта
     * @return true если шрифт был выгружен, false если не найден
     */
    bool unloadFont(const std::string& name);

    /**
     * @brief Выгружает отдельный звук
     * @param name Имя звука
     * @return true если звук был выгружен, false если не найден
     */
    bool unloadSound(const std::string& name);

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

    /**
     * @brief Возвращает количество загруженных звуков
     */
    size_t getSoundCount() const { return m_soundBuffers.size(); }

    // ========== Асинхронная загрузка ==========

    /**
     * @brief Асинхронно загружает текстуру
     *
     * Загрузка происходит в отдельном потоке. Используйте isTextureReady()
     * для проверки готовности или waitForTexture() для ожидания.
     *
     * @param name Имя текстуры для идентификации
     * @param path Путь к файлу текстуры
     * @return future для проверки статуса загрузки (true если успешно)
     */
    std::future<bool> loadTextureAsync(const std::string& name, const std::string& path);

    /**
     * @brief Асинхронно загружает текстуру (имя = путь)
     * @param path Путь к файлу текстуры (также используется как имя)
     * @return future для проверки статуса загрузки
     */
    std::future<bool> loadTextureAsync(const std::string& path);

    /**
     * @brief Асинхронно загружает шрифт
     * @param name Имя шрифта для идентификации
     * @param path Путь к файлу шрифта
     * @return future для проверки статуса загрузки
     */
    std::future<bool> loadFontAsync(const std::string& name, const std::string& path);

    /**
     * @brief Асинхронно загружает звук
     * @param name Имя звука для идентификации
     * @param path Путь к файлу звука
     * @return future для проверки статуса загрузки
     */
    std::future<bool> loadSoundAsync(const std::string& name, const std::string& path);

    /**
     * @brief Проверяет, загружена ли текстура (синхронно или асинхронно)
     * @param name Имя текстуры
     * @return true если текстура полностью загружена и доступна
     */
    bool isTextureReady(const std::string& name) const;

    /**
     * @brief Проверяет, загружен ли шрифт (синхронно или асинхронно)
     * @param name Имя шрифта
     * @return true если шрифт полностью загружен и доступен
     */
    bool isFontReady(const std::string& name) const;

    /**
     * @brief Проверяет, загружен ли звук (синхронно или асинхронно)
     * @param name Имя звука
     * @return true если звук полностью загружен и доступен
     */
    bool isSoundReady(const std::string& name) const;

    /**
     * @brief Ожидает завершения загрузки текстуры
     *
     * Блокирует выполнение до тех пор, пока текстура не будет загружена.
     * Если текстура уже загружена, возвращает управление немедленно.
     *
     * @param name Имя текстуры
     * @param timeoutMs Таймаут в миллисекундах (0 = бесконечное ожидание)
     * @return true если текстура успешно загружена, false если таймаут или ошибка
     */
    bool waitForTexture(const std::string& name, int timeoutMs = 0);

    /**
     * @brief Ожидает завершения загрузки шрифта
     * @param name Имя шрифта
     * @param timeoutMs Таймаут в миллисекундах (0 = бесконечное ожидание)
     * @return true если шрифт успешно загружен
     */
    bool waitForFont(const std::string& name, int timeoutMs = 0);

    /**
     * @brief Ожидает завершения загрузки звука
     * @param name Имя звука
     * @param timeoutMs Таймаут в миллисекундах (0 = бесконечное ожидание)
     * @return true если звук успешно загружен
     */
    bool waitForSound(const std::string& name, int timeoutMs = 0);

    /**
     * @brief Предзагружает текстуры асинхронно
     *
     * Запускает загрузку всех текстур в фоновом режиме. Прогресс можно
     * отслеживать через callback, который вызывается после каждой загрузки.
     *
     * @param paths Вектор путей к текстурам
     * @param progressCallback Callback для отслеживания прогресса (0.0-1.0)
     * @param completionCallback Callback, вызываемый по завершении загрузки всех ресурсов
     * @return future для проверки завершения загрузки
     */
    std::future<size_t> preloadTexturesAsync(
        const std::vector<std::string>& paths,
        std::function<void(float)> progressCallback = nullptr,
        std::function<void(size_t loaded, size_t total)> completionCallback = nullptr);

    /**
     * @brief Предзагружает шрифты асинхронно
     * @param fontConfigs Вектор пар {имя, путь}
     * @param progressCallback Callback для прогресса
     * @param completionCallback Callback по завершении
     * @return future для проверки завершения
     */
    std::future<size_t> preloadFontsAsync(
        const std::vector<std::pair<std::string, std::string>>& fontConfigs,
        std::function<void(float)> progressCallback = nullptr,
        std::function<void(size_t loaded, size_t total)> completionCallback = nullptr);

    /**
     * @brief Предзагружает звуки асинхронно
     * @param soundConfigs Вектор пар {имя, путь}
     * @param progressCallback Callback для прогресса
     * @param completionCallback Callback по завершении
     * @return future для проверки завершения
     */
    std::future<size_t> preloadSoundsAsync(
        const std::vector<std::pair<std::string, std::string>>& soundConfigs,
        std::function<void(float)> progressCallback = nullptr,
        std::function<void(size_t loaded, size_t total)> completionCallback = nullptr);

    /**
     * @brief Ожидает завершения всех активных асинхронных загрузок
     *
     * Блокирует выполнение до тех пор, пока все запущенные асинхронные
     * операции загрузки не завершатся.
     */
    void waitForAllLoads();

    /**
     * @brief Проверяет наличие активных асинхронных загрузок
     * @return true если есть незавершенные операции загрузки
     */
    bool hasActiveLoads() const;

    // ========== Метаданные спрайтов ==========

    /**
     * @brief Загружает метаданные спрайта из .sprite.json файла
     *
     * Загружает JSON файл с метаданными спрайта. Автоматически загружает
     * связанную текстуру, если она еще не загружена.
     *
     * @param path Путь к .sprite.json файлу
     * @return Указатель на метаданные или nullptr если ошибка
     */
    const SpriteMetadata* loadSpriteMetadata(const std::string& path);

    /**
     * @brief Получает метаданные спрайта по имени
     * @param name Имя спрайта (из поля "name" в JSON)
     * @return Указатель на метаданные или nullptr если не найдено
     */
    const SpriteMetadata* getSpriteMetadata(const std::string& name) const;

    /**
     * @brief Проверяет, загружены ли метаданные спрайта
     * @param name Имя спрайта
     * @return true если метаданные загружены
     */
    bool hasSpriteMetadata(const std::string& name) const;

    /**
     * @brief Выгружает метаданные спрайта
     * @param name Имя спрайта
     * @return true если метаданные были выгружены
     */
    bool unloadSpriteMetadata(const std::string& name);

    /**
     * @brief Асинхронно загружает метаданные спрайта
     *
     * Загружает метаданные спрайта и связанную текстуру в отдельном потоке.
     *
     * @param path Путь к .sprite.json файлу
     * @return future для проверки статуса загрузки
     */
    std::future<bool> loadSpriteMetadataAsync(const std::string& path);

    /**
     * @brief Возвращает количество загруженных метаданных спрайтов
     */
    size_t getSpriteMetadataCount() const { return m_spriteMetadata.size(); }

    /**
     * @brief Получает список всех загруженных имен спрайтов
     */
    std::vector<std::string> getSpriteNames() const;

    // ========== Отслеживание памяти ==========

    /**
     * @brief Структура для статистики использования памяти
     */
    struct MemoryStats {
        size_t texturesMemory = 0;  ///< Память, занятая текстурами (байты)
        size_t fontsMemory = 0;     ///< Память, занятая шрифтами (байты)
        size_t soundsMemory = 0;    ///< Память, занятая звуками (байты)
        size_t totalMemory = 0;     ///< Общая занятая память (байты)

        /**
         * @brief Форматирует размер в читаемый вид
         * @param bytes Размер в байтах
         * @return Строка вида "123.45 KB" или "2.34 MB"
         */
        static std::string formatSize(size_t bytes);
    };

    /**
     * @brief Возвращает статистику использования памяти
     *
     * Вычисляет объем памяти, занимаемый всеми загруженными ресурсами.
     * Для текстур используется формула: width * height * 4 (RGBA).
     * Для звуков: sampleCount * channelCount * sizeof(int16_t).
     *
     * @return Структура со статистикой памяти
     */
    MemoryStats getMemoryUsage() const;

private:
    /**
     * @brief Загружает системный шрифт по умолчанию
     * @return Путь к системному шрифту или пустую строку
     */
    std::string getSystemFontPath() const;

    /**
     * @brief Вычисляет размер текстуры в памяти
     * @param texture Текстура
     * @return Размер в байтах (width * height * 4 для RGBA)
     */
    static size_t calculateTextureSize(const sf::Texture& texture);

    /**
     * @brief Вычисляет размер звукового буфера в памяти
     * @param buffer Звуковой буфер
     * @return Размер в байтах
     */
    static size_t calculateSoundSize(const sf::SoundBuffer& buffer);

    /**
     * @brief Проверяет превышение лимита памяти и выводит предупреждение
     * @param stats Статистика памяти
     */
    void checkMemoryLimit(const MemoryStats& stats) const;

    static constexpr size_t MEMORY_LIMIT_WARNING = 512 * 1024 * 1024; ///< Лимит памяти (512 МБ)

    std::unordered_map<std::string, sf::Font> m_fonts;          ///< Кеш шрифтов
    std::unordered_map<std::string, sf::Texture> m_textures;    ///< Кеш текстур
    std::unordered_map<std::string, sf::SoundBuffer> m_soundBuffers; ///< Кеш звуковых буферов
    std::unordered_map<std::string, SpriteMetadata> m_spriteMetadata; ///< Кеш метаданных спрайтов

    // Асинхронная загрузка
    mutable std::mutex m_textureMutex;      ///< Мьютекс для безопасного доступа к текстурам
    mutable std::mutex m_fontMutex;         ///< Мьютекс для безопасного доступа к шрифтам
    mutable std::mutex m_soundMutex;        ///< Мьютекс для безопасного доступа к звукам
    mutable std::mutex m_spriteMutex;       ///< Мьютекс для безопасного доступа к метаданным спрайтов
    mutable std::mutex m_futuresMutex;      ///< Мьютекс для безопасного доступа к futures

    std::vector<std::future<void>> m_activeFutures; ///< Активные асинхронные операции
    std::atomic<int> m_activeLoads{0};               ///< Счетчик активных загрузок
};

} // namespace core
