#include "core/ResourceManager.h"
#include "core/Logger.h"
#include <stdexcept>
#include <thread>
#include <chrono>

namespace core {

ResourceManager::ResourceManager() {
    LOG_DEBUG("ResourceManager initialized");
}

const sf::Font& ResourceManager::getFont(const std::string& name) {
    LOG_DEBUG("ResourceManager::getFont called for: {}", name);

    // Проверяем кеш
    auto it = m_fonts.find(name);
    if (it != m_fonts.end()) {
        LOG_DEBUG("Font '{}' found in cache", name);
        return it->second;
    }

    LOG_DEBUG("Font '{}' not in cache, attempting to load...", name);

    // Не найден в кеше - пытаемся загрузить
    if (name == "default") {
        // Загружаем системный шрифт
        LOG_DEBUG("Loading default system font...");
        std::string fontPath = getSystemFontPath();
        if (!fontPath.empty()) {
            LOG_DEBUG("System font path found: {}, loading...", fontPath);
            if (loadFont(name, fontPath)) {
                LOG_INFO("Loaded default system font: {}", fontPath);
                return m_fonts.at(name);
            } else {
                LOG_ERROR("Failed to load font from path: {}", fontPath);
            }
        } else {
            LOG_ERROR("No system font path found");
        }
    } else {
        LOG_WARN("Font name '{}' is not 'default' and no path specified", name);
    }

    // Не удалось загрузить
    LOG_ERROR("Failed to load font: {}", name);
    throw std::runtime_error("Failed to load font: " + name);
}

const sf::Texture& ResourceManager::getTexture(const std::string& path) {
    // Проверяем кеш (thread-safe)
    {
        std::lock_guard<std::mutex> lock(m_textureMutex);
        auto it = m_textures.find(path);
        if (it != m_textures.end()) {
            return it->second;
        }
    }

    // Не найден в кеше - пытаемся загрузить
    if (loadTexture(path)) {
        std::lock_guard<std::mutex> lock(m_textureMutex);
        return m_textures.at(path);
    }

    // Не удалось загрузить
    LOG_ERROR("Failed to load texture: {}", path);
    throw std::runtime_error("Failed to load texture: " + path);
}

bool ResourceManager::loadFont(const std::string& name, const std::string& path) {
    sf::Font font;
    if (!font.openFromFile(path)) {
        LOG_WARN("Failed to load font from: {}", path);
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(m_fontMutex);
        m_fonts[name] = std::move(font);
    }
    LOG_DEBUG("Font loaded: {} from {}", name, path);
    return true;
}

bool ResourceManager::loadTexture(const std::string& name, const std::string& path) {
    sf::Texture texture;
    if (!texture.loadFromFile(path)) {
        LOG_WARN("Failed to load texture from: {}", path);
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(m_textureMutex);
        m_textures[name] = std::move(texture);
    }
    LOG_DEBUG("Texture loaded: {} from {}", name, path);
    return true;
}

bool ResourceManager::loadTexture(const std::string& path) {
    return loadTexture(path, path);
}

bool ResourceManager::loadTextureFromImage(const std::string& name, const sf::Image& image) {
    sf::Texture texture;
    if (!texture.loadFromImage(image)) {
        LOG_WARN("Failed to load texture from image: {}", name);
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(m_textureMutex);
        m_textures[name] = std::move(texture);
    }
    LOG_DEBUG("Texture loaded from image: {}", name);
    return true;
}

size_t ResourceManager::preloadTextures(const std::vector<std::string>& paths,
                                        std::function<void(float)> progressCallback) {
    if (paths.empty()) {
        return 0;
    }

    size_t loaded = 0;
    size_t total = paths.size();

    LOG_INFO("Preloading {} textures...", total);

    for (size_t i = 0; i < total; ++i) {
        const auto& path = paths[i];

        // Пропускаем уже загруженные
        if (hasTexture(path)) {
            ++loaded;
        } else if (loadTexture(path)) {
            ++loaded;
        }

        // Уведомляем о прогрессе
        if (progressCallback) {
            float progress = static_cast<float>(i + 1) / static_cast<float>(total);
            progressCallback(progress);
        }
    }

    LOG_INFO("Preloaded {}/{} textures", loaded, total);
    return loaded;
}

size_t ResourceManager::preloadFonts(const std::vector<std::pair<std::string, std::string>>& fontConfigs,
                                     std::function<void(float)> progressCallback) {
    if (fontConfigs.empty()) {
        return 0;
    }

    size_t loaded = 0;
    size_t total = fontConfigs.size();

    LOG_INFO("Preloading {} fonts...", total);

    for (size_t i = 0; i < total; ++i) {
        const auto& [name, path] = fontConfigs[i];

        // Пропускаем уже загруженные
        if (hasFont(name)) {
            ++loaded;
        } else if (loadFont(name, path)) {
            ++loaded;
        }

        // Уведомляем о прогрессе
        if (progressCallback) {
            float progress = static_cast<float>(i + 1) / static_cast<float>(total);
            progressCallback(progress);
        }
    }

    LOG_INFO("Preloaded {}/{} fonts", loaded, total);
    return loaded;
}

bool ResourceManager::hasFont(const std::string& name) const {
    return m_fonts.find(name) != m_fonts.end();
}

bool ResourceManager::hasTexture(const std::string& name) const {
    return m_textures.find(name) != m_textures.end();
}

bool ResourceManager::unloadTexture(const std::string& name) {
    std::lock_guard<std::mutex> lock(m_textureMutex);
    auto it = m_textures.find(name);
    if (it != m_textures.end()) {
        m_textures.erase(it);
        LOG_DEBUG("Texture unloaded: {}", name);
        return true;
    }
    LOG_WARN("Cannot unload texture '{}': not found", name);
    return false;
}

bool ResourceManager::unloadFont(const std::string& name) {
    std::lock_guard<std::mutex> lock(m_fontMutex);
    auto it = m_fonts.find(name);
    if (it != m_fonts.end()) {
        m_fonts.erase(it);
        LOG_DEBUG("Font unloaded: {}", name);
        return true;
    }
    LOG_WARN("Cannot unload font '{}': not found", name);
    return false;
}

const sf::SoundBuffer& ResourceManager::getSound(const std::string& name) {
    // Проверяем кеш
    auto it = m_soundBuffers.find(name);
    if (it != m_soundBuffers.end()) {
        return it->second;
    }

    // Не найден в кеше
    LOG_ERROR("Failed to get sound: {}", name);
    throw std::runtime_error("Failed to get sound: " + name);
}

bool ResourceManager::loadSound(const std::string& name, const std::string& path) {
    sf::SoundBuffer buffer;
    if (!buffer.loadFromFile(path)) {
        LOG_WARN("Failed to load sound from: {}", path);
        return false;
    }

    {
        std::lock_guard<std::mutex> lock(m_soundMutex);
        m_soundBuffers[name] = std::move(buffer);
    }
    LOG_DEBUG("Sound loaded: {} from {}", name, path);
    return true;
}

size_t ResourceManager::preloadSounds(const std::vector<std::pair<std::string, std::string>>& soundConfigs,
                                      std::function<void(float)> progressCallback) {
    if (soundConfigs.empty()) {
        return 0;
    }

    size_t loaded = 0;
    size_t total = soundConfigs.size();

    LOG_INFO("Preloading {} sounds...", total);

    for (size_t i = 0; i < total; ++i) {
        const auto& [name, path] = soundConfigs[i];

        // Пропускаем уже загруженные
        if (hasSound(name)) {
            ++loaded;
        } else if (loadSound(name, path)) {
            ++loaded;
        }

        // Уведомляем о прогрессе
        if (progressCallback) {
            float progress = static_cast<float>(i + 1) / static_cast<float>(total);
            progressCallback(progress);
        }
    }

    LOG_INFO("Preloaded {}/{} sounds", loaded, total);
    return loaded;
}

bool ResourceManager::hasSound(const std::string& name) const {
    return m_soundBuffers.find(name) != m_soundBuffers.end();
}

bool ResourceManager::unloadSound(const std::string& name) {
    std::lock_guard<std::mutex> lock(m_soundMutex);
    auto it = m_soundBuffers.find(name);
    if (it != m_soundBuffers.end()) {
        m_soundBuffers.erase(it);
        LOG_DEBUG("Sound unloaded: {}", name);
        return true;
    }
    LOG_WARN("Cannot unload sound '{}': not found", name);
    return false;
}

void ResourceManager::clear() {
    std::lock_guard<std::mutex> textureLock(m_textureMutex);
    std::lock_guard<std::mutex> fontLock(m_fontMutex);
    std::lock_guard<std::mutex> soundLock(m_soundMutex);

    LOG_DEBUG("Clearing all resources: {} fonts, {} textures, {} sounds",
              m_fonts.size(), m_textures.size(), m_soundBuffers.size());
    m_fonts.clear();
    m_textures.clear();
    m_soundBuffers.clear();
}

std::string ResourceManager::getSystemFontPath() const {
    LOG_DEBUG("Searching for system font...");

    // Пытаемся найти системный шрифт в зависимости от платформы
    const std::vector<std::string> fontPaths = {
#ifdef __linux__
        "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
        "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
        "/usr/share/fonts/truetype/ubuntu/Ubuntu-R.ttf",
        "/usr/share/fonts/TTF/DejaVuSans.ttf",
#elif _WIN32
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/calibri.ttf",
        "C:/Windows/Fonts/segoeui.ttf",
#elif __APPLE__
        "/System/Library/Fonts/Helvetica.ttc",
        "/System/Library/Fonts/SFNSDisplay.ttf",
        "/Library/Fonts/Arial.ttf",
#endif
    };

    // Проверяем каждый путь
    for (const auto& path : fontPaths) {
        LOG_DEBUG("Trying font path: {}", path);
        sf::Font testFont;
        if (testFont.openFromFile(path)) {
            LOG_INFO("Successfully found system font: {}", path);
            return path;
        } else {
            LOG_DEBUG("Failed to load font from: {}", path);
        }
    }

    // Не найден ни один системный шрифт
    LOG_ERROR("No system font found! Tried {} paths", fontPaths.size());
    return "";
}

// ========== Асинхронная загрузка текстур ==========

std::future<bool> ResourceManager::loadTextureAsync(const std::string& name, const std::string& path) {
    LOG_DEBUG("Starting async texture load: {} from {}", name, path);

    m_activeLoads++;

    return std::async(std::launch::async, [this, name, path]() -> bool {
        // Загружаем текстуру из файла в отдельном потоке
        sf::Texture texture;
        if (!texture.loadFromFile(path)) {
            LOG_WARN("Failed to load texture from: {}", path);
            m_activeLoads--;
            return false;
        }

        // Помещаем загруженную текстуру в кеш (thread-safe)
        {
            std::lock_guard<std::mutex> lock(m_textureMutex);
            m_textures[name] = std::move(texture);
        }

        LOG_DEBUG("Async texture loaded: {} from {}", name, path);
        m_activeLoads--;
        return true;
    });
}

std::future<bool> ResourceManager::loadTextureAsync(const std::string& path) {
    return loadTextureAsync(path, path);
}

// ========== Асинхронная загрузка шрифтов ==========

std::future<bool> ResourceManager::loadFontAsync(const std::string& name, const std::string& path) {
    LOG_DEBUG("Starting async font load: {} from {}", name, path);

    m_activeLoads++;

    return std::async(std::launch::async, [this, name, path]() -> bool {
        // Загружаем шрифт из файла в отдельном потоке
        sf::Font font;
        if (!font.openFromFile(path)) {
            LOG_WARN("Failed to load font from: {}", path);
            m_activeLoads--;
            return false;
        }

        // Помещаем загруженный шрифт в кеш (thread-safe)
        {
            std::lock_guard<std::mutex> lock(m_fontMutex);
            m_fonts[name] = std::move(font);
        }

        LOG_DEBUG("Async font loaded: {} from {}", name, path);
        m_activeLoads--;
        return true;
    });
}

// ========== Асинхронная загрузка звуков ==========

std::future<bool> ResourceManager::loadSoundAsync(const std::string& name, const std::string& path) {
    LOG_DEBUG("Starting async sound load: {} from {}", name, path);

    m_activeLoads++;

    return std::async(std::launch::async, [this, name, path]() -> bool {
        // Загружаем звук из файла в отдельном потоке
        sf::SoundBuffer buffer;
        if (!buffer.loadFromFile(path)) {
            LOG_WARN("Failed to load sound from: {}", path);
            m_activeLoads--;
            return false;
        }

        // Помещаем загруженный звук в кеш (thread-safe)
        {
            std::lock_guard<std::mutex> lock(m_soundMutex);
            m_soundBuffers[name] = std::move(buffer);
        }

        LOG_DEBUG("Async sound loaded: {} from {}", name, path);
        m_activeLoads--;
        return true;
    });
}

// ========== Проверка готовности ресурсов ==========

bool ResourceManager::isTextureReady(const std::string& name) const {
    std::lock_guard<std::mutex> lock(m_textureMutex);
    return m_textures.find(name) != m_textures.end();
}

bool ResourceManager::isFontReady(const std::string& name) const {
    std::lock_guard<std::mutex> lock(m_fontMutex);
    return m_fonts.find(name) != m_fonts.end();
}

bool ResourceManager::isSoundReady(const std::string& name) const {
    std::lock_guard<std::mutex> lock(m_soundMutex);
    return m_soundBuffers.find(name) != m_soundBuffers.end();
}

// ========== Ожидание загрузки ==========

bool ResourceManager::waitForTexture(const std::string& name, int timeoutMs) {
    if (isTextureReady(name)) {
        return true;
    }

    LOG_DEBUG("Waiting for texture: {}", name);

    auto startTime = std::chrono::steady_clock::now();

    while (!isTextureReady(name)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        if (timeoutMs > 0) {
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - startTime).count();

            if (elapsed >= timeoutMs) {
                LOG_WARN("Timeout waiting for texture: {}", name);
                return false;
            }
        }
    }

    LOG_DEBUG("Texture ready: {}", name);
    return true;
}

bool ResourceManager::waitForFont(const std::string& name, int timeoutMs) {
    if (isFontReady(name)) {
        return true;
    }

    LOG_DEBUG("Waiting for font: {}", name);

    auto startTime = std::chrono::steady_clock::now();

    while (!isFontReady(name)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        if (timeoutMs > 0) {
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - startTime).count();

            if (elapsed >= timeoutMs) {
                LOG_WARN("Timeout waiting for font: {}", name);
                return false;
            }
        }
    }

    LOG_DEBUG("Font ready: {}", name);
    return true;
}

bool ResourceManager::waitForSound(const std::string& name, int timeoutMs) {
    if (isSoundReady(name)) {
        return true;
    }

    LOG_DEBUG("Waiting for sound: {}", name);

    auto startTime = std::chrono::steady_clock::now();

    while (!isSoundReady(name)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        if (timeoutMs > 0) {
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - startTime).count();

            if (elapsed >= timeoutMs) {
                LOG_WARN("Timeout waiting for sound: {}", name);
                return false;
            }
        }
    }

    LOG_DEBUG("Sound ready: {}", name);
    return true;
}

// ========== Асинхронная предзагрузка ==========

std::future<size_t> ResourceManager::preloadTexturesAsync(
    const std::vector<std::string>& paths,
    std::function<void(float)> progressCallback,
    std::function<void(size_t loaded, size_t total)> completionCallback) {

    if (paths.empty()) {
        return std::async(std::launch::deferred, []() { return size_t(0); });
    }

    LOG_INFO("Starting async preload of {} textures...", paths.size());

    return std::async(std::launch::async, [this, paths, progressCallback, completionCallback]() -> size_t {
        size_t loaded = 0;
        size_t total = paths.size();

        std::vector<std::future<bool>> futures;
        futures.reserve(total);

        // Запускаем загрузку всех текстур
        for (const auto& path : paths) {
            // Пропускаем уже загруженные
            if (isTextureReady(path)) {
                ++loaded;
                if (progressCallback) {
                    progressCallback(static_cast<float>(loaded) / static_cast<float>(total));
                }
            } else {
                futures.push_back(loadTextureAsync(path));
            }
        }

        // Ожидаем завершения всех загрузок
        for (size_t i = 0; i < futures.size(); ++i) {
            if (futures[i].get()) {
                ++loaded;
            }

            if (progressCallback) {
                float progress = static_cast<float>(loaded) / static_cast<float>(total);
                progressCallback(progress);
            }
        }

        LOG_INFO("Async preload completed: {}/{} textures", loaded, total);

        if (completionCallback) {
            completionCallback(loaded, total);
        }

        return loaded;
    });
}

std::future<size_t> ResourceManager::preloadFontsAsync(
    const std::vector<std::pair<std::string, std::string>>& fontConfigs,
    std::function<void(float)> progressCallback,
    std::function<void(size_t loaded, size_t total)> completionCallback) {

    if (fontConfigs.empty()) {
        return std::async(std::launch::deferred, []() { return size_t(0); });
    }

    LOG_INFO("Starting async preload of {} fonts...", fontConfigs.size());

    return std::async(std::launch::async, [this, fontConfigs, progressCallback, completionCallback]() -> size_t {
        size_t loaded = 0;
        size_t total = fontConfigs.size();

        std::vector<std::future<bool>> futures;
        futures.reserve(total);

        // Запускаем загрузку всех шрифтов
        for (const auto& [name, path] : fontConfigs) {
            // Пропускаем уже загруженные
            if (isFontReady(name)) {
                ++loaded;
                if (progressCallback) {
                    progressCallback(static_cast<float>(loaded) / static_cast<float>(total));
                }
            } else {
                futures.push_back(loadFontAsync(name, path));
            }
        }

        // Ожидаем завершения всех загрузок
        for (size_t i = 0; i < futures.size(); ++i) {
            if (futures[i].get()) {
                ++loaded;
            }

            if (progressCallback) {
                float progress = static_cast<float>(loaded) / static_cast<float>(total);
                progressCallback(progress);
            }
        }

        LOG_INFO("Async preload completed: {}/{} fonts", loaded, total);

        if (completionCallback) {
            completionCallback(loaded, total);
        }

        return loaded;
    });
}

std::future<size_t> ResourceManager::preloadSoundsAsync(
    const std::vector<std::pair<std::string, std::string>>& soundConfigs,
    std::function<void(float)> progressCallback,
    std::function<void(size_t loaded, size_t total)> completionCallback) {

    if (soundConfigs.empty()) {
        return std::async(std::launch::deferred, []() { return size_t(0); });
    }

    LOG_INFO("Starting async preload of {} sounds...", soundConfigs.size());

    return std::async(std::launch::async, [this, soundConfigs, progressCallback, completionCallback]() -> size_t {
        size_t loaded = 0;
        size_t total = soundConfigs.size();

        std::vector<std::future<bool>> futures;
        futures.reserve(total);

        // Запускаем загрузку всех звуков
        for (const auto& [name, path] : soundConfigs) {
            // Пропускаем уже загруженные
            if (isSoundReady(name)) {
                ++loaded;
                if (progressCallback) {
                    progressCallback(static_cast<float>(loaded) / static_cast<float>(total));
                }
            } else {
                futures.push_back(loadSoundAsync(name, path));
            }
        }

        // Ожидаем завершения всех загрузок
        for (size_t i = 0; i < futures.size(); ++i) {
            if (futures[i].get()) {
                ++loaded;
            }

            if (progressCallback) {
                float progress = static_cast<float>(loaded) / static_cast<float>(total);
                progressCallback(progress);
            }
        }

        LOG_INFO("Async preload completed: {}/{} sounds", loaded, total);

        if (completionCallback) {
            completionCallback(loaded, total);
        }

        return loaded;
    });
}

// ========== Управление загрузками ==========

void ResourceManager::waitForAllLoads() {
    LOG_DEBUG("Waiting for all active loads to complete...");

    while (m_activeLoads > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    LOG_DEBUG("All loads completed");
}

bool ResourceManager::hasActiveLoads() const {
    return m_activeLoads > 0;
}

} // namespace core
