#include "core/systems/SystemScheduler.h"
#include "core/Logger.h"
#include <algorithm>

namespace core {

SystemScheduler::SystemScheduler() {
    LOG_DEBUG("SystemScheduler initialized");
}

SystemScheduler::~SystemScheduler() {
    LOG_DEBUG("SystemScheduler destroyed");
}

void SystemScheduler::addSystem(std::unique_ptr<ISystem> system) {
    if (!system) {
        LOG_ERROR("Attempted to add null system to scheduler");
        return;
    }

    const char* name = system->getName();
    int priority = system->getPriority();

    LOG_INFO("Adding system '{}' with priority {}", name, priority);

    m_systems.push_back(std::move(system));
    m_needsSort = true;
}

void SystemScheduler::update(entt::registry& registry, double dt) {
    // Сортируем системы если нужно
    if (m_needsSort) {
        sortSystems();
    }

    // Обновляем все активные системы
    for (auto& system : m_systems) {
        if (system->isActive()) {
            system->update(registry, dt);
        }
    }
}

ISystem* SystemScheduler::getSystem(const char* name) {
    for (auto& sys : m_systems) {
        if (std::strcmp(sys->getName(), name) == 0) {
            return sys.get();
        }
    }
    return nullptr;
}

void SystemScheduler::clear() {
    LOG_DEBUG("Clearing all systems from scheduler");
    m_systems.clear();
    m_needsSort = false;
}

void SystemScheduler::sortSystems() {
    std::stable_sort(m_systems.begin(), m_systems.end(),
                     [](const std::unique_ptr<ISystem>& a, const std::unique_ptr<ISystem>& b) {
                         return a->getPriority() < b->getPriority();
                     });

    m_needsSort = false;

    // Логируем порядок систем
    LOG_DEBUG("Systems sorted by priority:");
    for (const auto& sys : m_systems) {
        LOG_DEBUG("  [{}] {}", sys->getPriority(), sys->getName());
    }
}

} // namespace core
