#include "core/StateManager.h"
#include "core/Logger.h"
#include "core/Window.h"
#include <SFML/Graphics/RenderWindow.hpp>

namespace core {

StateManager::StateManager()
    : m_isProcessingChanges(false)
    , m_inputManager(nullptr)
    , m_resourceManager(nullptr)
    , m_window(nullptr) {
    LOG_DEBUG("StateManager created");
}

void StateManager::pushState(std::unique_ptr<State> state) {
    if (!state) {
        LOG_ERROR("Attempted to push null state");
        return;
    }

    LOG_INFO("Pushing state: {}", state->getName());
    m_pendingChanges.push_back({Action::Push, std::move(state)});
}

void StateManager::popState() {
    LOG_INFO("Requesting pop state");
    m_pendingChanges.push_back({Action::Pop, nullptr});
}

void StateManager::changeState(std::unique_ptr<State> state) {
    if (!state) {
        LOG_ERROR("Attempted to change to null state");
        return;
    }

    LOG_INFO("Changing state to: {}", state->getName());
    m_pendingChanges.push_back({Action::Change, std::move(state)});
}

void StateManager::clearStates() {
    LOG_INFO("Clearing all states");
    m_pendingChanges.push_back({Action::Clear, nullptr});
}

void StateManager::handleEvent(const sf::Event& event) {
    // Обрабатываем события с вершины стека вниз
    // Если состояние возвращает true, событие считается обработанным
    for (auto it = m_states.rbegin(); it != m_states.rend(); ++it) {
        if ((*it)->handleEvent(event)) {
            break; // Событие обработано, не передаем дальше
        }
    }
}

void StateManager::update(double dt) {
    // Обновляем состояния с вершины стека вниз
    // Останавливаемся на первом состоянии, которое не разрешает обновление нижних
    for (auto it = m_states.rbegin(); it != m_states.rend(); ++it) {
        (*it)->update(dt);

        if (!(*it)->updateBelow()) {
            break; // Не обновляем состояния ниже
        }
    }

    // Применяем отложенные изменения после обновления
    applyPendingChanges();
}

void StateManager::render(sf::RenderWindow& window) {
    // Находим самое нижнее состояние, которое нужно отрисовать
    auto it = m_states.rbegin();
    while (it != m_states.rend()) {
        if (!(*it)->renderBelow()) {
            break;
        }
        ++it;
    }

    // Отрисовываем состояния снизу вверх
    if (it != m_states.rend()) {
        auto forwardIt = it.base();
        for (auto renderIt = forwardIt; renderIt != m_states.end(); ++renderIt) {
            (*renderIt)->render(window);
        }
    }
}

State* StateManager::getCurrentState() const {
    if (m_states.empty()) {
        return nullptr;
    }
    return m_states.back().get();
}

bool StateManager::isEmpty() const {
    return m_states.empty();
}

size_t StateManager::getStateCount() const {
    return m_states.size();
}

void StateManager::applyPendingChanges() {
    if (m_isProcessingChanges || m_pendingChanges.empty()) {
        return;
    }

    m_isProcessingChanges = true;

    for (auto& change : m_pendingChanges) {
        switch (change.action) {
            case Action::Push: {
                if (change.state) {
                    LOG_DEBUG("Applying push: {}", change.state->getName());
                    change.state->onEnter();
                    m_states.push_back(std::move(change.state));
                }
                break;
            }

            case Action::Pop: {
                if (!m_states.empty()) {
                    auto& topState = m_states.back();
                    LOG_DEBUG("Applying pop: {}", topState->getName());
                    topState->onExit();
                    m_states.pop_back();
                } else {
                    LOG_WARN("Attempted to pop from empty state stack");
                }
                break;
            }

            case Action::Change: {
                if (!m_states.empty()) {
                    auto& topState = m_states.back();
                    LOG_DEBUG("Applying change: {} -> {}",
                             topState->getName(),
                             change.state->getName());
                    topState->onExit();
                    m_states.pop_back();
                }
                if (change.state) {
                    change.state->onEnter();
                    m_states.push_back(std::move(change.state));
                }
                break;
            }

            case Action::Clear: {
                LOG_DEBUG("Applying clear, {} states", m_states.size());
                while (!m_states.empty()) {
                    m_states.back()->onExit();
                    m_states.pop_back();
                }
                break;
            }
        }
    }

    m_pendingChanges.clear();
    m_isProcessingChanges = false;
}

void StateManager::setInputManager(InputManager* inputManager) {
    m_inputManager = inputManager;
    LOG_DEBUG("InputManager set in StateManager");
}

InputManager* StateManager::getInputManager() const {
    return m_inputManager;
}

void StateManager::setWindow(Window* window) {
    m_window = window;
    LOG_DEBUG("Window set in StateManager");
}

sf::Vector2u StateManager::getWindowSize() const {
    if (m_window) {
        return m_window->getSize();
    }
    // Возвращаем размер по умолчанию если окно не установлено
    LOG_WARN("Window not set in StateManager, returning default size");
    return sf::Vector2u(1280, 720);
}

void StateManager::setResourceManager(ResourceManager* resourceManager) {
    m_resourceManager = resourceManager;
    LOG_DEBUG("ResourceManager set in StateManager");
}

ResourceManager* StateManager::getResourceManager() const {
    return m_resourceManager;
}

} // namespace core
