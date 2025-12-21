#include "core/AnimationData.h"
#include "core/SpriteMetadata.h"
#include "core/Logger.h"

namespace core {

std::shared_ptr<AnimationData> AnimationData::createFromMetadata(const SpriteMetadata& metadata) {
    auto animData = std::make_shared<AnimationData>();

    const auto& animations = metadata.getAnimations();
    const auto& size = metadata.getSize();

    LOG_DEBUG("Creating AnimationData from metadata '{}' with {} animations",
              metadata.getName(), animations.size());

    for (const auto& animInfo : animations) {
        AnimationDefinition definition(animInfo.name);
        definition.loop = animInfo.loop;

        // Конвертируем кадры из метаданных
        for (const auto& frameInfo : animInfo.frames) {
            // Создаем прямоугольник текстуры (SFML 3 API)
            sf::IntRect textureRect(
                sf::Vector2i(frameInfo.x, frameInfo.y),
                sf::Vector2i(size.width, size.height)
            );

            // Конвертируем длительность из миллисекунд в sf::Time
            sf::Time duration = sf::milliseconds(frameInfo.duration);

            // Добавляем кадр
            AnimationFrameData frame(textureRect, duration);
            definition.frames.push_back(frame);
        }

        LOG_DEBUG("  Animation '{}': {} frames, loop={}, duration={}ms",
                  animInfo.name,
                  definition.frames.size(),
                  definition.loop,
                  definition.getTotalDuration().asMilliseconds());

        animData->addAnimation(definition);
    }

    LOG_INFO("Created AnimationData from '{}' with {} animations",
             metadata.getName(), animData->getAnimationCount());

    return animData;
}

void AnimationData::addAnimation(const AnimationDefinition& definition) {
    if (definition.name.empty()) {
        LOG_WARN("Trying to add animation with empty name");
        return;
    }

    if (definition.frames.empty()) {
        LOG_WARN("Trying to add empty animation '{}'", definition.name);
        return;
    }

    m_animations[definition.name] = definition;
    LOG_DEBUG("Added animation '{}' with {} frames", definition.name, definition.frames.size());
}

AnimationDefinition& AnimationData::addAnimation(const std::string& name, bool loop) {
    AnimationDefinition definition(name);
    definition.loop = loop;
    m_animations[name] = definition;
    return m_animations[name];
}

const AnimationDefinition* AnimationData::getAnimation(const std::string& name) const {
    auto it = m_animations.find(name);
    if (it != m_animations.end()) {
        return &it->second;
    }
    return nullptr;
}

bool AnimationData::hasAnimation(const std::string& name) const {
    return m_animations.find(name) != m_animations.end();
}

std::vector<std::string> AnimationData::getAnimationNames() const {
    std::vector<std::string> names;
    names.reserve(m_animations.size());

    for (const auto& [name, definition] : m_animations) {
        names.push_back(name);
    }

    return names;
}

} // namespace core
