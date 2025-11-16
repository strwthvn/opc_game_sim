#pragma once

#include <SFML/Graphics/Color.hpp>
#include <SFML/Graphics/Rect.hpp>
#include <string>

namespace core {

/**
 * @brief Компонент трансформации
 *
 * Хранит позицию, вращение и масштаб сущности
 */
struct TransformComponent {
    float x = 0.0f;              ///< Позиция X в пикселях
    float y = 0.0f;              ///< Позиция Y в пикселях
    float rotation = 0.0f;       ///< Угол поворота в градусах
    float scaleX = 1.0f;         ///< Масштаб по X
    float scaleY = 1.0f;         ///< Масштаб по Y
};

/**
 * @brief Компонент спрайта
 *
 * Хранит информацию о текстуре и отрисовке
 */
struct SpriteComponent {
    std::string textureName;     ///< Имя текстуры в ResourceManager
    sf::IntRect textureRect;     ///< Прямоугольник текстуры (для спрайт-атласов)
    sf::Color color;             ///< Цвет модуляции (белый = без изменений)
    int layer = 0;               ///< Слой отрисовки (меньше = раньше)
    bool visible = true;         ///< Видимость спрайта

    /**
     * @brief Конструктор по умолчанию
     */
    SpriteComponent()
        : textureName("")
        , textureRect()  // Default constructor
        , color(sf::Color::White)
        , layer(0)
        , visible(true) {
    }

    /**
     * @brief Конструктор с текстурой
     * @param name Имя текстуры
     */
    explicit SpriteComponent(const std::string& name)
        : textureName(name)
        , textureRect()  // Default constructor
        , color(sf::Color::White)
        , layer(0)
        , visible(true) {
    }
};

/**
 * @brief Компонент скорости
 *
 * Хранит линейную и угловую скорость для простого движения
 */
struct VelocityComponent {
    float vx = 0.0f;             ///< Скорость по X (пикселей/секунду)
    float vy = 0.0f;             ///< Скорость по Y (пикселей/секунду)
    float angularVelocity = 0.0f; ///< Угловая скорость (градусов/секунду)
};

/**
 * @brief Тег компонент для обозначения камеры
 *
 * Сущность с этим компонентом будет использоваться как камера
 */
struct CameraComponent {
    float zoom = 1.0f;           ///< Масштаб камеры (1.0 = нормальный)
    bool active = true;          ///< Активна ли камера
};

} // namespace core
