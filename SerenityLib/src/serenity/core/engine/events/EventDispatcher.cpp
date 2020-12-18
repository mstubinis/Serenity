#include <serenity/core/engine/events/EventDispatcher.h>
#include <serenity/core/engine/utils/Utils.h>
#include <serenity/core/engine/events/Observer.h>
#include <serenity/core/engine/events/Event.h>

Engine::priv::EventDispatcher::EventDispatcher() { 
    m_Observers.resize((size_t)EventType::_TOTAL);
}
void Engine::priv::EventDispatcher::registerObject(Observer& observer, EventType eventType) noexcept {
    std::lock_guard lock(m_Mutex);
    auto& observers_with_event_type = m_Observers[(size_t)eventType];
    if (internal_has_duplicate(observer, observers_with_event_type)) {
        return;
    }
    observers_with_event_type.emplace_back(&observer);
}
void Engine::priv::EventDispatcher::unregisterObject(Observer& observer, EventType eventType) noexcept {
    //ASSERT(m_Observers.size() > (size_t)eventType, __FUNCTION__ << "(): eventType does not fit in m_Observers!");
    if (m_Observers.size() > (size_t)eventType) {
        std::lock_guard lock(m_Mutex);
        m_UnregisteredObservers.emplace_back(&observer, (size_t)eventType);
    }
}
bool Engine::priv::EventDispatcher::isObjectRegistered(const Observer& observer, EventType eventType) const noexcept {
    std::lock_guard lock(m_Mutex);
    const auto& observers_with_event_type = m_Observers[(size_t)eventType];
    bool result = internal_has_duplicate(observer, observers_with_event_type);
    return result;
}
void Engine::priv::EventDispatcher::dispatchEvent(const Event& e) noexcept {
    const auto& observers_with_event_type = m_Observers[(size_t)e.type];
    std::for_each(observers_with_event_type.begin(), observers_with_event_type.end(), [&e](auto& observer) {
        observer->onEvent(e);
    });
}
void Engine::priv::EventDispatcher::dispatchEvent(EventType eventType) noexcept {
    Event e{ eventType };
    const auto& observers_with_event_type = m_Observers[(size_t)e.type];
    std::for_each(observers_with_event_type.begin(), observers_with_event_type.end(), [&e](auto& observer) {
        observer->onEvent(e);
    });
}
void Engine::priv::EventDispatcher::postUpdate() {
    if (m_UnregisteredObservers.size() > 0) {
        std::lock_guard lock(m_Mutex);
        std::for_each(std::cbegin(m_UnregisteredObservers), std::cend(m_UnregisteredObservers), [this](const auto& data) {
            std::erase_if(m_Observers[data.second], [&data](auto observerItr) {
                return observerItr == data.first;
            });
        });
        m_UnregisteredObservers.clear();
    }
}