#pragma once

#include <SFML/Graphics/Rect.hpp>
#include <SFML/System/Time.hpp>
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace core {

// Forward declaration
class SpriteMetadata;

/**
 * @brief Один кадр анимации (для AnimationData)
 *
 * Хранит информацию о конкретном кадре: позицию в текстуре и длительность.
 * Отличается от AnimationFrame в SpriteMetadata тем, что использует sf::IntRect и sf::Time.
 */
struct AnimationFrameData {
    sf::IntRect textureRect;    ///< Прямоугольник в текстуре
    sf::Time duration;          ///< Длительность кадра

    AnimationFrameData() = default;

    AnimationFrameData(const sf::IntRect& rect, const sf::Time& dur)
        : textureRect(rect), duration(dur) {}
};

/**
 * @brief Определение анимации
 *
 * Содержит все кадры для одной именованной анимации (например, "running", "idle").
 */
struct AnimationDefinition {
    std::string name;                       ///< Имя анимации
    std::vector<AnimationFrameData> frames; ///< Кадры анимации
    bool loop = true;                       ///< Зацикливать анимацию

    AnimationDefinition() = default;

    explicit AnimationDefinition(const std::string& animName)
        : name(animName), loop(true) {}

    /**
     * @brief Получает общую длительность анимации
     * @return Суммарная длительность всех кадров
     */
    sf::Time getTotalDuration() const {
        sf::Time total = sf::Time::Zero;
        for (const auto& frame : frames) {
            total += frame.duration;
        }
        return total;
    }

    /**
     * @brief Проверяет, пустая ли анимация
     */
    bool isEmpty() const {
        return frames.empty();
    }

    /**
     * @brief Возвращает количество кадров
     */
    size_t getFrameCount() const {
        return frames.size();
    }
};

/**
 * @brief Менеджер анимаций для сущности
 *
 * Хранит все доступные анимации для спрайта и позволяет переключаться между ними.
 * Создается из SpriteMetadata или вручную.
 */
class AnimationData {
public:
    /**
     * @brief Конструктор по умолчанию
     */
    AnimationData() = default;

    /**
     * @brief Создает AnimationData из SpriteMetadata
     *
     * Конвертирует все анимации из метаданных спрайта в формат AnimationData.
     *
     * @param metadata Метаданные спрайта
     * @return Указатель на созданный AnimationData
     */
    static std::shared_ptr<AnimationData> createFromMetadata(const SpriteMetadata& metadata);

    /**
     * @brief Добавляет определение анимации
     * @param definition Определение анимации
     */
    void addAnimation(const AnimationDefinition& definition);

    /**
     * @brief Добавляет определение анимации по имени
     * @param name Имя анимации
     * @param loop Зацикливать анимацию
     * @return Ссылка на созданное определение
     */
    AnimationDefinition& addAnimation(const std::string& name, bool loop = true);

    /**
     * @brief Получает определение анимации по имени
     * @param name Имя анимации
     * @return Указатель на определение или nullptr если не найдено
     */
    const AnimationDefinition* getAnimation(const std::string& name) const;

    /**
     * @brief Проверяет наличие анимации
     */
    bool hasAnimation(const std::string& name) const;

    /**
     * @brief Возвращает имена всех анимаций
     */
    std::vector<std::string> getAnimationNames() const;

    /**
     * @brief Возвращает количество анимаций
     */
    size_t getAnimationCount() const {
        return m_animations.size();
    }

    /**
     * @brief Очищает все анимации
     */
    void clear() {
        m_animations.clear();
    }

private:
    std::unordered_map<std::string, AnimationDefinition> m_animations; ///< Карта анимаций по имени
};

} // namespace core
