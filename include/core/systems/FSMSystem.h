#pragma once

#include "core/systems/ISystem.h"
#include <entt/entt.hpp>

namespace core {

/**
 * @brief Система управления конечными автоматами (FSM)
 *
 * Обновляет EntityStateComponent, отслеживая время в состоянии.
 * Вызывает коллбеки при переходах между состояниями.
 *
 * Приоритет: 150 (после CollisionSystem, до OverlaySystem)
 */
class FSMSystem : public ISystem {
public:
    /**
     * @brief Конструктор
     */
    FSMSystem();

    /**
     * @brief Деструктор
     */
    ~FSMSystem() override;

    /**
     * @brief Обновление системы FSM
     *
     * Обновляет timeInState для всех сущностей с EntityStateComponent.
     *
     * @param registry EnTT registry с сущностями
     * @param dt Время с последнего обновления (секунды)
     */
    void update(entt::registry& registry, double dt) override;

    /**
     * @brief Получить приоритет системы
     * @return Приоритет (150 - после CollisionSystem, до OverlaySystem)
     */
    int getPriority() const override { return 150; }

    /**
     * @brief Получить имя системы
     * @return Имя системы
     */
    const char* getName() const override { return "FSMSystem"; }
};

} // namespace core
