#include "core/SpriteMetadata.h"
#include "core/Logger.h"
#include <fstream>
#include <algorithm>

namespace core {

// ========== AnimationFrame ==========

AnimationFrame AnimationFrame::fromJson(const nlohmann::json& json) {
    AnimationFrame frame;

    if (json.contains("x") && json["x"].is_number_integer()) {
        frame.x = json["x"].get<int>();
    }

    if (json.contains("y") && json["y"].is_number_integer()) {
        frame.y = json["y"].get<int>();
    }

    if (json.contains("duration") && json["duration"].is_number_integer()) {
        frame.duration = json["duration"].get<int>();
    }

    return frame;
}

nlohmann::json AnimationFrame::toJson() const {
    return nlohmann::json{
        {"x", x},
        {"y", y},
        {"duration", duration}
    };
}

// ========== AnimationInfo ==========

AnimationInfo AnimationInfo::fromJson(const nlohmann::json& json) {
    AnimationInfo info;

    if (json.contains("name") && json["name"].is_string()) {
        info.name = json["name"].get<std::string>();
    }

    if (json.contains("loop") && json["loop"].is_boolean()) {
        info.loop = json["loop"].get<bool>();
    }

    if (json.contains("frames") && json["frames"].is_array()) {
        for (const auto& frameJson : json["frames"]) {
            info.frames.push_back(AnimationFrame::fromJson(frameJson));
        }
    }

    return info;
}

nlohmann::json AnimationInfo::toJson() const {
    nlohmann::json framesArray = nlohmann::json::array();
    for (const auto& frame : frames) {
        framesArray.push_back(frame.toJson());
    }

    return nlohmann::json{
        {"name", name},
        {"frames", framesArray},
        {"loop", loop}
    };
}

// ========== SpriteSize ==========

SpriteSize SpriteSize::fromJson(const nlohmann::json& json) {
    SpriteSize size;

    if (json.contains("width") && json["width"].is_number_integer()) {
        size.width = json["width"].get<int>();
    }

    if (json.contains("height") && json["height"].is_number_integer()) {
        size.height = json["height"].get<int>();
    }

    return size;
}

nlohmann::json SpriteSize::toJson() const {
    return nlohmann::json{
        {"width", width},
        {"height", height}
    };
}

// ========== SpriteOrigin ==========

SpriteOrigin SpriteOrigin::fromJson(const nlohmann::json& json) {
    SpriteOrigin origin;

    if (json.contains("x") && json["x"].is_number_integer()) {
        origin.x = json["x"].get<int>();
    }

    if (json.contains("y") && json["y"].is_number_integer()) {
        origin.y = json["y"].get<int>();
    }

    return origin;
}

nlohmann::json SpriteOrigin::toJson() const {
    return nlohmann::json{
        {"x", x},
        {"y", y}
    };
}

// ========== SpriteMetadata ==========

std::optional<SpriteMetadata> SpriteMetadata::loadFromFile(const std::string& path) {
    LOG_DEBUG("Loading sprite metadata from: {}", path);

    std::ifstream file(path);
    if (!file.is_open()) {
        LOG_ERROR("Failed to open sprite metadata file: {}", path);
        return std::nullopt;
    }

    nlohmann::json json;
    try {
        file >> json;
    } catch (const nlohmann::json::parse_error& e) {
        LOG_ERROR("Failed to parse sprite metadata JSON: {} (error: {})", path, e.what());
        return std::nullopt;
    }

    return loadFromJson(json);
}

std::optional<SpriteMetadata> SpriteMetadata::loadFromJson(const nlohmann::json& json) {
    if (!json.contains("sprite")) {
        LOG_ERROR("JSON does not contain 'sprite' root object");
        return std::nullopt;
    }

    const auto& spriteJson = json["sprite"];
    SpriteMetadata metadata;

    // Обязательные поля
    if (spriteJson.contains("name") && spriteJson["name"].is_string()) {
        metadata.m_name = spriteJson["name"].get<std::string>();
    } else {
        LOG_WARN("Sprite metadata missing 'name' field");
    }

    if (spriteJson.contains("texture") && spriteJson["texture"].is_string()) {
        metadata.m_texturePath = spriteJson["texture"].get<std::string>();
    } else {
        LOG_WARN("Sprite metadata missing 'texture' field");
    }

    // Опциональные поля
    if (spriteJson.contains("author") && spriteJson["author"].is_string()) {
        metadata.m_author = spriteJson["author"].get<std::string>();
    }

    if (spriteJson.contains("version") && spriteJson["version"].is_string()) {
        metadata.m_version = spriteJson["version"].get<std::string>();
    }

    if (spriteJson.contains("created") && spriteJson["created"].is_string()) {
        metadata.m_created = spriteJson["created"].get<std::string>();
    }

    if (spriteJson.contains("description") && spriteJson["description"].is_string()) {
        metadata.m_description = spriteJson["description"].get<std::string>();
    }

    // Размер
    if (spriteJson.contains("size") && spriteJson["size"].is_object()) {
        metadata.m_size = SpriteSize::fromJson(spriteJson["size"]);
    }

    // Origin
    if (spriteJson.contains("origin") && spriteJson["origin"].is_object()) {
        metadata.m_origin = SpriteOrigin::fromJson(spriteJson["origin"]);
    }

    // Теги
    if (spriteJson.contains("tags") && spriteJson["tags"].is_array()) {
        for (const auto& tag : spriteJson["tags"]) {
            if (tag.is_string()) {
                metadata.m_tags.push_back(tag.get<std::string>());
            }
        }
    }

    // Анимации
    if (spriteJson.contains("animations") && spriteJson["animations"].is_array()) {
        for (const auto& animJson : spriteJson["animations"]) {
            metadata.m_animations.push_back(AnimationInfo::fromJson(animJson));
        }
    }

    LOG_INFO("Loaded sprite metadata: '{}' (texture: {}, {} animations)",
             metadata.m_name, metadata.m_texturePath, metadata.m_animations.size());

    return metadata;
}

bool SpriteMetadata::saveToFile(const std::string& path) const {
    LOG_DEBUG("Saving sprite metadata to: {}", path);

    std::ofstream file(path);
    if (!file.is_open()) {
        LOG_ERROR("Failed to open file for writing: {}", path);
        return false;
    }

    nlohmann::json json = toJson();

    try {
        file << json.dump(2); // Pretty print with 2-space indentation
        LOG_INFO("Saved sprite metadata to: {}", path);
        return true;
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to write sprite metadata: {} (error: {})", path, e.what());
        return false;
    }
}

nlohmann::json SpriteMetadata::toJson() const {
    // Анимации
    nlohmann::json animationsArray = nlohmann::json::array();
    for (const auto& anim : m_animations) {
        animationsArray.push_back(anim.toJson());
    }

    // Теги
    nlohmann::json tagsArray = nlohmann::json::array();
    for (const auto& tag : m_tags) {
        tagsArray.push_back(tag);
    }

    // Основной объект
    nlohmann::json spriteObject = {
        {"name", m_name},
        {"texture", m_texturePath},
        {"size", m_size.toJson()},
        {"origin", m_origin.toJson()},
        {"tags", tagsArray},
        {"animations", animationsArray}
    };

    // Добавляем опциональные поля
    if (!m_author.empty()) {
        spriteObject["author"] = m_author;
    }

    if (!m_version.empty()) {
        spriteObject["version"] = m_version;
    }

    if (!m_created.empty()) {
        spriteObject["created"] = m_created;
    }

    if (!m_description.empty()) {
        spriteObject["description"] = m_description;
    }

    return nlohmann::json{{"sprite", spriteObject}};
}

const AnimationInfo* SpriteMetadata::getAnimation(const std::string& name) const {
    auto it = std::find_if(m_animations.begin(), m_animations.end(),
                          [&name](const AnimationInfo& anim) {
                              return anim.name == name;
                          });

    if (it != m_animations.end()) {
        return &(*it);
    }

    return nullptr;
}

bool SpriteMetadata::hasAnimation(const std::string& name) const {
    return getAnimation(name) != nullptr;
}

bool SpriteMetadata::hasTag(const std::string& tag) const {
    return std::find(m_tags.begin(), m_tags.end(), tag) != m_tags.end();
}

void SpriteMetadata::addTag(const std::string& tag) {
    if (!hasTag(tag)) {
        m_tags.push_back(tag);
    }
}

void SpriteMetadata::addAnimation(const AnimationInfo& animation) {
    // Заменяем существующую анимацию с таким же именем
    auto it = std::find_if(m_animations.begin(), m_animations.end(),
                          [&animation](const AnimationInfo& anim) {
                              return anim.name == animation.name;
                          });

    if (it != m_animations.end()) {
        *it = animation;
        LOG_DEBUG("Replaced animation: {}", animation.name);
    } else {
        m_animations.push_back(animation);
        LOG_DEBUG("Added animation: {}", animation.name);
    }
}

} // namespace core
