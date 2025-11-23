#pragma once

#include <entt/entt.hpp>

namespace core {

/**
 * @brief Базовый интерфейс для всех ECS систем
 *
 * Унифицирует API систем и позволяет управлять ими через SystemScheduler.
 * Все системы должны наследоваться от этого интерфейса.
 */
class ISystem {
public:
    virtual ~ISystem() = default;

    /**
     * @brief Обновление системы
     *
     * Вызывается каждый кадр системным планировщиком.
     *
     * @param registry EnTT registry с сущностями
     * @param dt Время с последнего обновления (секунды)
     */
    virtual void update(entt::registry& registry, double dt) = 0;

    /**
     * @brief Получить приоритет выполнения системы
     *
     * Системы с меньшим приоритетом выполняются раньше.
     * Позволяет контролировать порядок выполнения систем.
     *
     * Рекомендуемые значения:
     * - 0-99: Игровая логика (UpdateSystem, LifetimeSystem)
     * - 100-199: Физика и симуляция
     * - 200-299: Синхронизация позиций (TilePositionSystem)
     * - 300-399: Анимация и визуальные эффекты (AnimationSystem)
     * - 400-499: UI и оверлеи (OverlaySystem)
     *
     * @return Приоритет системы (меньше = раньше)
     */
    virtual int getPriority() const { return 0; }

    /**
     * @brief Получить имя системы для отладки
     * @return Имя системы
     */
    virtual const char* getName() const = 0;

    /**
     * @brief Проверить, активна ли система
     *
     * Неактивные системы пропускаются при обновлении.
     *
     * @return true если система активна
     */
    virtual bool isActive() const { return true; }

    /**
     * @brief Установить активность системы
     * @param active Новое состояние активности
     */
    virtual void setActive(bool active) {
        // По умолчанию ничего не делаем (переопределяется в подклассах при необходимости)
    }
};

} // namespace core
