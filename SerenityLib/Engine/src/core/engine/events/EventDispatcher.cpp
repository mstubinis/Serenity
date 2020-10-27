#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/events/EventDispatcher.h>
#include <core/engine/utils/Utils.h>
#include <core/engine/events/Observer.h>
#include <core/engine/events/Event.h>

Engine::priv::EventDispatcher::EventDispatcher() { 
    m_Observers.resize((size_t)EventType::_TOTAL);
}
void Engine::priv::EventDispatcher::registerObject(Observer& observer, EventType eventType) noexcept {
    std::lock_guard lock(m_Mutex);
    auto& observers_with_event_type = m_Observers[(size_t)eventType];
    if (internal_check_for_duplicates(observer, observers_with_event_type)) {
        return;
    }
    observers_with_event_type.emplace_back(&observer);
}
void Engine::priv::EventDispatcher::unregisterObject(Observer& observer, EventType eventType) noexcept {
    ASSERT(m_Observers.size() > (size_t)eventType, __FUNCTION__ << "(): eventType does not fit in m_Observers!");
    std::lock_guard lock(m_Mutex);
    m_UnregisteredObservers.emplace_back(&observer, (size_t)eventType);
}
bool Engine::priv::EventDispatcher::isObjectRegistered(const Observer& observer, EventType eventType) const noexcept {
    std::lock_guard lock(m_Mutex);
    const auto& observers_with_event_type = m_Observers[(size_t)eventType];
    bool result = internal_check_for_duplicates(observer, observers_with_event_type);
    return result;
}
void Engine::priv::EventDispatcher::dispatchEvent(const Event& e) noexcept {
    const auto& observers_with_event_type = m_Observers[(size_t)e.type];
    for (auto& observer : observers_with_event_type) {
        observer->onEvent(e);
    }
}
void Engine::priv::EventDispatcher::dispatchEvent(EventType eventType) noexcept {
    Event e{ eventType };
    const auto& observers_with_event_type = m_Observers[(size_t)e.type];
    for (auto& observer : observers_with_event_type) {
        observer->onEvent(e);
    }
}
void Engine::priv::EventDispatcher::onPostUpdate() {
    if (m_UnregisteredObservers.size() > 0) {
        std::lock_guard lock(m_Mutex);
        for (auto& [observer, index] : m_UnregisteredObservers) {
            auto& observers_with_event_type = m_Observers[index];
            std::erase_if(observers_with_event_type, [&observer](auto observerItr) {
                return observerItr == observer;
            });
        }
        m_UnregisteredObservers.clear();
    }
}