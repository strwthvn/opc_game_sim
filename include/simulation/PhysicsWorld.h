#pragma once

#include <box2d/box2d.h>

/**
 * @file PhysicsWorld.h
 * @brief Обёртка для Box2D физического мира
 *
 * PhysicsWorld управляет Box2D симуляцией и предоставляет API для
 * интеграции с ECS системами.
 */

namespace simulation {

/**
 * @brief Обёртка для Box2D физического мира (версия 3.x)
 *
 * Управляет созданием и обновлением Box2D мира, предоставляет
 * методы для настройки гравитации и выполнения шагов симуляции.
 *
 * Класс использует RAII для автоматического управления временем жизни Box2D мира.
 *
 * @note Box2D 3.x использует ID-based API вместо указателей.
 * @note Физический мир работает в метрах, а не в пикселях.
 *       Используйте константу PIXELS_PER_METER для конвертации.
 */
class PhysicsWorld {
public:
    /**
     * @brief Масштаб для конвертации между пикселями и метрами
     *
     * Box2D работает в метрах, а рендеринг в пикселях.
     * 32 пикселя = 1 метр (соответствует размеру тайла).
     */
    static constexpr float PIXELS_PER_METER = 32.0f;

    /**
     * @brief Конструктор с указанием гравитации
     *
     * Создаёт новый Box2D мир с заданным вектором гравитации.
     *
     * @param gravity Вектор гравитации в м/с² (по умолчанию: 0, 9.8 - земная гравитация вниз)
     *
     * @code
     * // Создать мир с земной гравитацией
     * PhysicsWorld world(b2Vec2{0.0f, 9.8f});
     *
     * // Создать мир без гравитации (космос)
     * PhysicsWorld world(b2Vec2{0.0f, 0.0f});
     * @endcode
     */
    explicit PhysicsWorld(const b2Vec2& gravity = b2Vec2{0.0f, 9.8f});

    /**
     * @brief Деструктор
     *
     * Автоматически уничтожает Box2D мир и все связанные ресурсы.
     */
    ~PhysicsWorld();

    // Запретить копирование (Box2D мир не должен копироваться)
    PhysicsWorld(const PhysicsWorld&) = delete;
    PhysicsWorld& operator=(const PhysicsWorld&) = delete;

    // Запретить перемещение (b2WorldId не должен дублироваться)
    PhysicsWorld(PhysicsWorld&&) = delete;
    PhysicsWorld& operator=(PhysicsWorld&&) = delete;

    /**
     * @brief Выполнить шаг физической симуляции
     *
     * Обновляет состояние всех физических тел на заданный промежуток времени.
     * Вызывает b2World_Step() с заданным количеством sub-steps.
     *
     * @param deltaTime Время шага в секундах (обычно 1/60 или фиксированный timestep)
     *
     * @note Рекомендуется использовать фиксированный timestep (например, 1/60 = 0.0167с)
     *       для стабильности симуляции. Box2D 3.x использует sub-stepping для точности.
     *
     * @code
     * // Пример использования в game loop
     * const float FIXED_TIMESTEP = 1.0f / 60.0f;
     * physicsWorld.step(FIXED_TIMESTEP);
     * @endcode
     */
    void step(float deltaTime);

    /**
     * @brief Установить вектор гравитации
     *
     * Изменяет глобальную гравитацию для всех динамических тел в мире.
     * Изменение влияет только на новые вычисления, не меняет текущие скорости.
     *
     * @param gravity Новый вектор гравитации в м/с²
     *
     * @code
     * // Изменить направление гравитации (платформер с перевёрнутой гравитацией)
     * world.setGravity(b2Vec2{0.0f, -9.8f});
     *
     * // Отключить гравитацию (космическая станция)
     * world.setGravity(b2Vec2{0.0f, 0.0f});
     *
     * // Боковая гравитация
     * world.setGravity(b2Vec2{9.8f, 0.0f});
     * @endcode
     */
    void setGravity(const b2Vec2& gravity);

    /**
     * @brief Получить текущий вектор гравитации
     *
     * @return Вектор гравитации в м/с²
     */
    b2Vec2 getGravity() const;

    /**
     * @brief Получить ID Box2D мира
     *
     * Предоставляет доступ к b2WorldId для создания тел,
     * джоинтов и выполнения запросов (raycasts, AABB queries).
     *
     * @return ID Box2D мира (всегда валидный)
     *
     * @code
     * // Создание тела через ID мира
     * b2BodyDef bodyDef = b2DefaultBodyDef();
     * bodyDef.type = b2_dynamicBody;
     * bodyDef.position = b2Vec2{1.0f, 5.0f};
     * b2BodyId bodyId = b2CreateBody(world.getWorldId(), &bodyDef);
     * @endcode
     */
    b2WorldId getWorldId() const;

    /**
     * @brief Конвертировать пиксели в метры
     *
     * @param pixels Значение в пикселях
     * @return Значение в метрах
     */
    static float pixelsToMeters(float pixels) {
        return pixels / PIXELS_PER_METER;
    }

    /**
     * @brief Конвертировать метры в пиксели
     *
     * @param meters Значение в метрах
     * @return Значение в пикселях
     */
    static float metersToPixels(float meters) {
        return meters * PIXELS_PER_METER;
    }

    /**
     * @brief Конвертировать вектор из пикселей в метры
     *
     * @param pixels Вектор в пикселях
     * @return Вектор в метрах
     */
    static b2Vec2 pixelsToMeters(const b2Vec2& pixels) {
        return b2Vec2(pixels.x / PIXELS_PER_METER, pixels.y / PIXELS_PER_METER);
    }

    /**
     * @brief Конвертировать вектор из метров в пиксели
     *
     * @param meters Вектор в метрах
     * @return Вектор в пикселях
     */
    static b2Vec2 metersToPixels(const b2Vec2& meters) {
        return b2Vec2(meters.x * PIXELS_PER_METER, meters.y * PIXELS_PER_METER);
    }

    /**
     * @brief Проверить валидность Box2D мира
     *
     * @return true если мир был создан и не разрушен
     */
    bool isValid() const;

private:
    b2WorldId m_worldId;   ///< ID Box2D физического мира

    /**
     * @brief Количество sub-steps на один шаг симуляции
     *
     * Большее значение = более точная симуляция, но медленнее.
     * Box2D 3.x использует sub-stepping вместо итераций velocity/position.
     * Рекомендуемое значение: 4.
     */
    static constexpr int SUB_STEP_COUNT = 4;
};

} // namespace simulation
