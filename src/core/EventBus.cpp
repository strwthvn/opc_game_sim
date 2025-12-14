#include "core/EventBus.h"
#include "core/Logger.h"

namespace core {

EventBus& EventBus::getInstance() {
    static EventBus instance;
    return instance;
}

void EventBus::clear() {
    LOG_DEBUG("EventBus: Clearing all event subscriptions (total types: {})", m_signals.size());
    m_signals.clear();
}

} // namespace core
