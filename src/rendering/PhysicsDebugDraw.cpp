#include "rendering/PhysicsDebugDraw.h"
#include "simulation/PhysicsWorld.h"
#include <cmath>

namespace rendering {

PhysicsDebugDraw::PhysicsDebugDraw() {
    // Инициализируем структуру значениями по умолчанию
    m_debugDraw = b2DefaultDebugDraw();

    // Устанавливаем function pointers
    m_debugDraw.DrawPolygonFcn = drawPolygon;
    m_debugDraw.DrawSolidPolygonFcn = drawSolidPolygon;
    m_debugDraw.DrawCircleFcn = drawCircle;
    m_debugDraw.DrawSolidCircleFcn = drawSolidCircle;
    m_debugDraw.DrawSolidCapsuleFcn = drawSolidCapsule;
    m_debugDraw.DrawSegmentFcn = drawSegment;
    m_debugDraw.DrawTransformFcn = drawTransform;
    m_debugDraw.DrawPointFcn = drawPoint;
    m_debugDraw.DrawStringFcn = drawString;

    // Устанавливаем context для получения this в callbacks
    m_debugDraw.context = this;

    // Включаем отрисовку shapes по умолчанию
    m_debugDraw.drawShapes = true;
    m_debugDraw.drawJoints = false;
    m_debugDraw.drawBounds = false;
    m_debugDraw.drawContacts = false;
    m_debugDraw.drawMass = false;
}

void PhysicsDebugDraw::setDrawShapes(bool enable) {
    m_debugDraw.drawShapes = enable;
}

void PhysicsDebugDraw::setDrawJoints(bool enable) {
    m_debugDraw.drawJoints = enable;
}

void PhysicsDebugDraw::setDrawBounds(bool enable) {
    m_debugDraw.drawBounds = enable;
}

void PhysicsDebugDraw::setDrawContacts(bool enable) {
    m_debugDraw.drawContacts = enable;
}

void PhysicsDebugDraw::setDrawMass(bool enable) {
    m_debugDraw.drawMass = enable;
}

void PhysicsDebugDraw::begin(sf::RenderTarget* target) {
    m_target = target;
    m_lineVertices.clear();
    m_triangleVertices.clear();
}

void PhysicsDebugDraw::end() {
    if (!m_target) {
        return;
    }

    // Сначала рисуем заполненные треугольники (фон)
    if (!m_triangleVertices.empty()) {
        m_target->draw(m_triangleVertices.data(), m_triangleVertices.size(),
                       sf::PrimitiveType::Triangles);
    }

    // Затем рисуем линии (контуры поверх)
    if (!m_lineVertices.empty()) {
        m_target->draw(m_lineVertices.data(), m_lineVertices.size(),
                       sf::PrimitiveType::Lines);
    }

    m_target = nullptr;
}

b2DebugDraw& PhysicsDebugDraw::getDebugDraw() {
    return m_debugDraw;
}

// ===== Static callbacks =====

void PhysicsDebugDraw::drawPolygon(const b2Vec2* vertices, int vertexCount,
                                   b2HexColor color, void* context) {
    auto* self = static_cast<PhysicsDebugDraw*>(context);
    if (!self || vertexCount < 2) {
        return;
    }

    sf::Color sfColor = hexToColor(color);

    // Рисуем контур полигона линиями
    for (int i = 0; i < vertexCount; ++i) {
        sf::Vector2f p1 = self->toPixels(vertices[i]);
        sf::Vector2f p2 = self->toPixels(vertices[(i + 1) % vertexCount]);

        self->m_lineVertices.emplace_back(p1, sfColor);
        self->m_lineVertices.emplace_back(p2, sfColor);
    }
}

void PhysicsDebugDraw::drawSolidPolygon(b2Transform transform,
                                        const b2Vec2* vertices, int vertexCount,
                                        float /*radius*/, b2HexColor color,
                                        void* context) {
    auto* self = static_cast<PhysicsDebugDraw*>(context);
    if (!self || vertexCount < 3) {
        return;
    }

    // Цвет заливки с прозрачностью
    sf::Color fillColor = hexToColor(color, 128);
    // Цвет контура
    sf::Color outlineColor = hexToColor(color);

    // Преобразуем вершины в мировые координаты и в пиксели
    std::vector<sf::Vector2f> pixelVertices;
    pixelVertices.reserve(static_cast<size_t>(vertexCount));

    for (int i = 0; i < vertexCount; ++i) {
        // Применяем трансформацию к локальной вершине
        b2Vec2 worldPoint = b2TransformPoint(transform, vertices[i]);
        pixelVertices.push_back(self->toPixels(worldPoint));
    }

    // Создаём triangle fan для заливки
    // Центр - первая вершина, остальные образуют треугольники
    for (int i = 1; i < vertexCount - 1; ++i) {
        self->m_triangleVertices.emplace_back(pixelVertices[0], fillColor);
        self->m_triangleVertices.emplace_back(pixelVertices[static_cast<size_t>(i)], fillColor);
        self->m_triangleVertices.emplace_back(pixelVertices[static_cast<size_t>(i) + 1], fillColor);
    }

    // Рисуем контур
    for (int i = 0; i < vertexCount; ++i) {
        self->m_lineVertices.emplace_back(pixelVertices[static_cast<size_t>(i)], outlineColor);
        self->m_lineVertices.emplace_back(
            pixelVertices[static_cast<size_t>((i + 1) % vertexCount)], outlineColor);
    }
}

void PhysicsDebugDraw::drawCircle(b2Vec2 center, float radius, b2HexColor color,
                                  void* context) {
    auto* self = static_cast<PhysicsDebugDraw*>(context);
    if (!self) {
        return;
    }

    sf::Vector2f centerPixels = self->toPixels(center);
    float radiusPixels = radius * simulation::PhysicsWorld::PIXELS_PER_METER;
    sf::Color sfColor = hexToColor(color);

    self->addCircleOutline(centerPixels, radiusPixels, sfColor);
}

void PhysicsDebugDraw::drawSolidCircle(b2Transform transform, float radius,
                                       b2HexColor color, void* context) {
    auto* self = static_cast<PhysicsDebugDraw*>(context);
    if (!self) {
        return;
    }

    sf::Vector2f centerPixels = self->toPixels(transform.p);
    float radiusPixels = radius * simulation::PhysicsWorld::PIXELS_PER_METER;

    sf::Color fillColor = hexToColor(color, 128);
    sf::Color outlineColor = hexToColor(color);

    // Заливка круга
    self->addFilledCircle(centerPixels, radiusPixels, fillColor);

    // Контур круга
    self->addCircleOutline(centerPixels, radiusPixels, outlineColor);

    // Рисуем линию от центра по направлению rotation
    // Это показывает ориентацию тела
    b2Vec2 direction = {std::cos(b2Rot_GetAngle(transform.q)),
                        std::sin(b2Rot_GetAngle(transform.q))};
    b2Vec2 endPoint = {transform.p.x + direction.x * radius,
                       transform.p.y + direction.y * radius};

    self->m_lineVertices.emplace_back(centerPixels, outlineColor);
    self->m_lineVertices.emplace_back(self->toPixels(endPoint), outlineColor);
}

void PhysicsDebugDraw::drawSolidCapsule(b2Vec2 p1, b2Vec2 p2, float radius,
                                        b2HexColor color, void* context) {
    auto* self = static_cast<PhysicsDebugDraw*>(context);
    if (!self) {
        return;
    }

    sf::Vector2f p1Pixels = self->toPixels(p1);
    sf::Vector2f p2Pixels = self->toPixels(p2);
    float radiusPixels = radius * simulation::PhysicsWorld::PIXELS_PER_METER;

    sf::Color fillColor = hexToColor(color, 128);
    sf::Color outlineColor = hexToColor(color);

    // Рисуем два полукруга на концах и прямоугольник между ними
    // Упрощённая версия - рисуем два круга и линию между ними
    self->addFilledCircle(p1Pixels, radiusPixels, fillColor);
    self->addFilledCircle(p2Pixels, radiusPixels, fillColor);
    self->addCircleOutline(p1Pixels, radiusPixels, outlineColor);
    self->addCircleOutline(p2Pixels, radiusPixels, outlineColor);

    // Вычисляем перпендикуляр для боковых линий
    sf::Vector2f dir = p2Pixels - p1Pixels;
    float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
    if (len > 0.0001f) {
        sf::Vector2f perp(-dir.y / len * radiusPixels, dir.x / len * radiusPixels);

        // Боковые линии
        self->m_lineVertices.emplace_back(p1Pixels + perp, outlineColor);
        self->m_lineVertices.emplace_back(p2Pixels + perp, outlineColor);

        self->m_lineVertices.emplace_back(p1Pixels - perp, outlineColor);
        self->m_lineVertices.emplace_back(p2Pixels - perp, outlineColor);
    }
}

void PhysicsDebugDraw::drawSegment(b2Vec2 p1, b2Vec2 p2, b2HexColor color,
                                   void* context) {
    auto* self = static_cast<PhysicsDebugDraw*>(context);
    if (!self) {
        return;
    }

    sf::Color sfColor = hexToColor(color);

    self->m_lineVertices.emplace_back(self->toPixels(p1), sfColor);
    self->m_lineVertices.emplace_back(self->toPixels(p2), sfColor);
}

void PhysicsDebugDraw::drawTransform(b2Transform transform, void* context) {
    auto* self = static_cast<PhysicsDebugDraw*>(context);
    if (!self) {
        return;
    }

    // Длина осей в метрах
    constexpr float axisLength = 0.5f;

    sf::Vector2f origin = self->toPixels(transform.p);

    // Ось X (красная)
    b2Vec2 xAxis = {transform.p.x + axisLength * transform.q.c,
                    transform.p.y + axisLength * transform.q.s};
    self->m_lineVertices.emplace_back(origin, sf::Color::Red);
    self->m_lineVertices.emplace_back(self->toPixels(xAxis), sf::Color::Red);

    // Ось Y (зелёная)
    b2Vec2 yAxis = {transform.p.x - axisLength * transform.q.s,
                    transform.p.y + axisLength * transform.q.c};
    self->m_lineVertices.emplace_back(origin, sf::Color::Green);
    self->m_lineVertices.emplace_back(self->toPixels(yAxis), sf::Color::Green);
}

void PhysicsDebugDraw::drawPoint(b2Vec2 p, float size, b2HexColor color,
                                 void* context) {
    auto* self = static_cast<PhysicsDebugDraw*>(context);
    if (!self) {
        return;
    }

    sf::Vector2f pos = self->toPixels(p);
    sf::Color sfColor = hexToColor(color);

    // Рисуем точку как маленький крестик
    float halfSize = size * simulation::PhysicsWorld::PIXELS_PER_METER * 0.5f;
    if (halfSize < 2.0f) {
        halfSize = 2.0f;
    }

    self->m_lineVertices.emplace_back(sf::Vector2f(pos.x - halfSize, pos.y), sfColor);
    self->m_lineVertices.emplace_back(sf::Vector2f(pos.x + halfSize, pos.y), sfColor);

    self->m_lineVertices.emplace_back(sf::Vector2f(pos.x, pos.y - halfSize), sfColor);
    self->m_lineVertices.emplace_back(sf::Vector2f(pos.x, pos.y + halfSize), sfColor);
}

void PhysicsDebugDraw::drawString(b2Vec2 /*p*/, const char* /*s*/,
                                  b2HexColor /*color*/, void* /*context*/) {
    // Отрисовка текста не реализована
    // Для этого нужен доступ к шрифту, что усложняет архитектуру
}

// ===== Вспомогательные методы =====

sf::Color PhysicsDebugDraw::hexToColor(b2HexColor hex, uint8_t alpha) {
    return sf::Color(static_cast<uint8_t>((hex >> 16) & 0xFF),
                     static_cast<uint8_t>((hex >> 8) & 0xFF),
                     static_cast<uint8_t>(hex & 0xFF), alpha);
}

sf::Vector2f PhysicsDebugDraw::toPixels(b2Vec2 meters) const {
    return sf::Vector2f(meters.x * simulation::PhysicsWorld::PIXELS_PER_METER,
                        meters.y * simulation::PhysicsWorld::PIXELS_PER_METER);
}

void PhysicsDebugDraw::addFilledCircle(sf::Vector2f center, float radius,
                                       sf::Color color) {
    // Аппроксимируем круг треугольниками (triangle fan)
    constexpr float PI = 3.14159265358979323846f;
    constexpr float angleStep = 2.0f * PI / CIRCLE_SEGMENTS;

    for (int i = 0; i < CIRCLE_SEGMENTS; ++i) {
        float angle1 = static_cast<float>(i) * angleStep;
        float angle2 = static_cast<float>(i + 1) * angleStep;

        sf::Vector2f p1(center.x + radius * std::cos(angle1),
                        center.y + radius * std::sin(angle1));
        sf::Vector2f p2(center.x + radius * std::cos(angle2),
                        center.y + radius * std::sin(angle2));

        m_triangleVertices.emplace_back(center, color);
        m_triangleVertices.emplace_back(p1, color);
        m_triangleVertices.emplace_back(p2, color);
    }
}

void PhysicsDebugDraw::addCircleOutline(sf::Vector2f center, float radius,
                                        sf::Color color) {
    constexpr float PI = 3.14159265358979323846f;
    constexpr float angleStep = 2.0f * PI / CIRCLE_SEGMENTS;

    for (int i = 0; i < CIRCLE_SEGMENTS; ++i) {
        float angle1 = static_cast<float>(i) * angleStep;
        float angle2 = static_cast<float>(i + 1) * angleStep;

        sf::Vector2f p1(center.x + radius * std::cos(angle1),
                        center.y + radius * std::sin(angle1));
        sf::Vector2f p2(center.x + radius * std::cos(angle2),
                        center.y + radius * std::sin(angle2));

        m_lineVertices.emplace_back(p1, color);
        m_lineVertices.emplace_back(p2, color);
    }
}

}  // namespace rendering
