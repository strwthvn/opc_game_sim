#pragma once

/**
 * @file PhysicsDebugDraw.h
 * @brief Отладочная визуализация физики Box2D 3.x
 *
 * Реализует debug draw для Box2D 3.x с использованием SFML 3.
 * Box2D 3.x использует callback-based API через структуру b2DebugDraw
 * с function pointers.
 */

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Vertex.hpp>
#include <box2d/box2d.h>
#include <vector>

namespace rendering {

/**
 * @brief Класс для отладочной визуализации физических тел Box2D
 *
 * Реализует отрисовку физических примитивов (полигоны, круги, сегменты)
 * через SFML. Использует batching для эффективного рендеринга.
 *
 * @note Box2D 3.x использует callback-based подход с function pointers,
 *       а не виртуальные методы как в Box2D 2.x.
 *
 * @code
 * PhysicsDebugDraw debugDraw;
 * debugDraw.begin(&window);
 * b2World_Draw(worldId, &debugDraw.getDebugDraw());
 * debugDraw.end();
 * @endcode
 */
class PhysicsDebugDraw {
public:
    /**
     * @brief Конструктор
     *
     * Инициализирует b2DebugDraw структуру с function pointers
     * и включает отрисовку shapes по умолчанию.
     */
    PhysicsDebugDraw();

    /**
     * @brief Включить/выключить отрисовку shapes (коллайдеров)
     * @param enable true для включения
     */
    void setDrawShapes(bool enable);

    /**
     * @brief Включить/выключить отрисовку joints (соединений)
     * @param enable true для включения
     */
    void setDrawJoints(bool enable);

    /**
     * @brief Включить/выключить отрисовку AABB bounds
     * @param enable true для включения
     */
    void setDrawBounds(bool enable);

    /**
     * @brief Включить/выключить отрисовку контактных точек
     * @param enable true для включения
     */
    void setDrawContacts(bool enable);

    /**
     * @brief Включить/выключить отрисовку центра масс
     * @param enable true для включения
     */
    void setDrawMass(bool enable);

    /**
     * @brief Начать сбор примитивов для отрисовки
     *
     * Вызывать перед b2World_Draw(). Очищает внутренние буферы
     * и сохраняет указатель на render target.
     *
     * @param target Указатель на sf::RenderTarget (обычно sf::RenderWindow)
     */
    void begin(sf::RenderTarget* target);

    /**
     * @brief Отрисовать все собранные примитивы
     *
     * Вызывать после b2World_Draw(). Рендерит все накопленные
     * вершины в target и очищает буферы.
     */
    void end();

    /**
     * @brief Получить структуру b2DebugDraw для передачи в b2World_Draw
     * @return Ссылка на внутреннюю структуру b2DebugDraw
     */
    b2DebugDraw& getDebugDraw();

private:
    // ===== Static callbacks для Box2D 3.x =====

    /**
     * @brief Отрисовка контура полигона (линиями)
     */
    static void drawPolygon(const b2Vec2* vertices, int vertexCount,
                            b2HexColor color, void* context);

    /**
     * @brief Отрисовка заполненного полигона с контуром
     */
    static void drawSolidPolygon(b2Transform transform, const b2Vec2* vertices,
                                 int vertexCount, float radius, b2HexColor color,
                                 void* context);

    /**
     * @brief Отрисовка контура круга
     */
    static void drawCircle(b2Vec2 center, float radius, b2HexColor color,
                           void* context);

    /**
     * @brief Отрисовка заполненного круга с контуром
     */
    static void drawSolidCircle(b2Transform transform, float radius,
                                b2HexColor color, void* context);

    /**
     * @brief Отрисовка заполненной капсулы
     */
    static void drawSolidCapsule(b2Vec2 p1, b2Vec2 p2, float radius,
                                 b2HexColor color, void* context);

    /**
     * @brief Отрисовка отрезка (линии)
     */
    static void drawSegment(b2Vec2 p1, b2Vec2 p2, b2HexColor color,
                            void* context);

    /**
     * @brief Отрисовка трансформации (осей координат)
     */
    static void drawTransform(b2Transform transform, void* context);

    /**
     * @brief Отрисовка точки
     */
    static void drawPoint(b2Vec2 p, float size, b2HexColor color, void* context);

    /**
     * @brief Отрисовка текста (не реализовано)
     */
    static void drawString(b2Vec2 p, const char* s, b2HexColor color,
                           void* context);

    // ===== Вспомогательные методы =====

    /**
     * @brief Конвертировать b2HexColor в sf::Color
     * @param hex Цвет в формате 0xRRGGBB
     * @param alpha Альфа-канал (0-255)
     * @return sf::Color
     */
    static sf::Color hexToColor(b2HexColor hex, uint8_t alpha = 255);

    /**
     * @brief Конвертировать координаты из метров в пиксели
     * @param meters Позиция в метрах (Box2D)
     * @return Позиция в пикселях (SFML)
     */
    sf::Vector2f toPixels(b2Vec2 meters) const;

    /**
     * @brief Добавить заполненный круг в буфер треугольников
     * @param center Центр в пикселях
     * @param radius Радиус в пикселях
     * @param color Цвет заливки
     */
    void addFilledCircle(sf::Vector2f center, float radius, sf::Color color);

    /**
     * @brief Добавить контур круга в буфер линий
     * @param center Центр в пикселях
     * @param radius Радиус в пикселях
     * @param color Цвет контура
     */
    void addCircleOutline(sf::Vector2f center, float radius, sf::Color color);

    // ===== Данные =====

    std::vector<sf::Vertex> m_lineVertices;      ///< Буфер для линий
    std::vector<sf::Vertex> m_triangleVertices;  ///< Буфер для треугольников

    sf::RenderTarget* m_target = nullptr;  ///< Текущий render target
    b2DebugDraw m_debugDraw{};             ///< Структура для Box2D

    static constexpr int CIRCLE_SEGMENTS = 24;  ///< Количество сегментов для аппроксимации круга
};

}  // namespace rendering
