#include "core/State.h"
#include "core/StateManager.h"
#include "core/InputManager.h"
#include <SFML/System/Vector2.hpp>

namespace core {

State::State(StateManager* stateManager)
    : m_stateManager(stateManager) {
}

InputManager* State::getInputManager() const {
    if (m_stateManager) {
        return m_stateManager->getInputManager();
    }
    return nullptr;
}

sf::Vector2u State::getWindowSize() const {
    if (m_stateManager) {
        return m_stateManager->getWindowSize();
    }
    // Fallback размер если StateManager не установлен
    return sf::Vector2u(1280, 720);
}

ResourceManager* State::getResourceManager() const {
    if (m_stateManager) {
        return m_stateManager->getResourceManager();
    }
    return nullptr;
}

} // namespace core
