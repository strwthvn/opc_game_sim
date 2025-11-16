#include "core/ResourceManager.h"
#include "core/Logger.h"
#include <stdexcept>

namespace core {

ResourceManager::ResourceManager() {
    LOG_DEBUG("ResourceManager initialized");
}

const sf::Font& ResourceManager::getFont(const std::string& name) {
    // Проверяем кеш
    auto it = m_fonts.find(name);
    if (it != m_fonts.end()) {
        return it->second;
    }

    // Не найден в кеше - пытаемся загрузить
    if (name == "default") {
        // Загружаем системный шрифт
        std::string fontPath = getSystemFontPath();
        if (!fontPath.empty()) {
            if (loadFont(name, fontPath)) {
                LOG_INFO("Loaded default system font: {}", fontPath);
                return m_fonts.at(name);
            }
        }
    }

    // Не удалось загрузить
    LOG_ERROR("Failed to load font: {}", name);
    throw std::runtime_error("Failed to load font: " + name);
}

const sf::Texture& ResourceManager::getTexture(const std::string& path) {
    // Проверяем кеш
    auto it = m_textures.find(path);
    if (it != m_textures.end()) {
        return it->second;
    }

    // Не найден в кеше - пытаемся загрузить
    if (loadTexture(path, path)) {
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

    m_fonts[name] = std::move(font);
    LOG_DEBUG("Font loaded: {} from {}", name, path);
    return true;
}

bool ResourceManager::loadTexture(const std::string& name, const std::string& path) {
    sf::Texture texture;
    if (!texture.loadFromFile(path)) {
        LOG_WARN("Failed to load texture from: {}", path);
        return false;
    }

    m_textures[name] = std::move(texture);
    LOG_DEBUG("Texture loaded: {} from {}", name, path);
    return true;
}

bool ResourceManager::hasFont(const std::string& name) const {
    return m_fonts.find(name) != m_fonts.end();
}

bool ResourceManager::hasTexture(const std::string& name) const {
    return m_textures.find(name) != m_textures.end();
}

void ResourceManager::clear() {
    LOG_DEBUG("Clearing all resources: {} fonts, {} textures",
              m_fonts.size(), m_textures.size());
    m_fonts.clear();
    m_textures.clear();
}

std::string ResourceManager::getSystemFontPath() const {
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
        sf::Font testFont;
        if (testFont.openFromFile(path)) {
            return path;
        }
    }

    // Не найден ни один системный шрифт
    LOG_WARN("No system font found");
    return "";
}

} // namespace core
