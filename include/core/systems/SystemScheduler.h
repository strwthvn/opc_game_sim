#pragma once

#include "core/systems/ISystem.h"
#include <entt/entt.hpp>
#include <memory>
#include <vector>
#include <algorithm>

namespace core {

/**
 * @brief Планировщик систем для управления порядком выполнения
 *
 * Автоматически управляет порядком обновления систем на основе их приоритетов.
 * Упрощает добавление новых систем и гарантирует корректный порядок выполнения.
 *
 * Использование:
 * @code
 * SystemScheduler scheduler;
 * scheduler.addSystem(std::make_unique<UpdateSystem>());
 * scheduler.addSystem(std::make_unique<TilePositionSystem>());
 * scheduler.update(registry, dt);
 * @endcode
 */
class SystemScheduler {
public:
    /**
     * @brief Конструктор
     */
    SystemScheduler();

    /**
     * @brief Деструктор
     */
    ~SystemScheduler();

    /**
     * @brief Добавить систему в планировщик
     *
     * Система автоматически сортируется по приоритету.
     * Системы с меньшим приоритетом выполняются раньше.
     *
     * @param system Уникальный указатель на систему
     */
    void addSystem(std::unique_ptr<ISystem> system);

    /**
     * @brief Обновить все активные системы
     *
     * Системы вызываются в порядке приоритета (меньше = раньше).
     * Неактивные системы пропускаются.
     *
     * @param registry EnTT registry с сущностями
     * @param dt Время с последнего обновления (секунды)
     */
    void update(entt::registry& registry, double dt);

    /**
     * @brief Получить систему по имени
     *
     * @param name Имя системы
     * @return Указатель на систему или nullptr если не найдена
     */
    ISystem* getSystem(const char* name);

    /**
     * @brief Получить систему по типу
     *
     * @tparam T Тип системы
     * @return Указатель на систему или nullptr если не найдена
     */
    template<typename T>
    T* getSystem() {
        for (auto& sys : m_systems) {
            if (auto* casted = dynamic_cast<T*>(sys.get())) {
                return casted;
            }
        }
        return nullptr;
    }

    /**
     * @brief Очистить все системы
     */
    void clear();

    /**
     * @brief Получить количество зарегистрированных систем
     * @return Количество систем
     */
    size_t getSystemCount() const { return m_systems.size(); }

private:
    std::vector<std::unique_ptr<ISystem>> m_systems;  ///< Список систем
    bool m_needsSort = false;  ///< Флаг необходимости пересортировки

    /**
     * @brief Сортировать системы по приоритету
     */
    void sortSystems();
};

} // namespace core
