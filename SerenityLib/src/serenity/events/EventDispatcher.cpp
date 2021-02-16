#include <serenity/events/EventDispatcher.h>
#include <serenity/utils/Utils.h>
#include <serenity/events/Observer.h>
#include <serenity/events/Event.h>

Engine::priv::EventDispatcher::EventDispatcher() { 
    m_Observers.resize((size_t)EventType::_TOTAL);
}
void Engine::priv::EventDispatcher::registerObject(Observer& observer, EventType eventType) noexcept {
    std::lock_guard lock{ m_Mutex };
    auto& observers_with_event_type = m_Observers[(size_t)eventType];
    if (internal_has_duplicate(observer, observers_with_event_type)) {
        return;
    }
    observers_with_event_type.emplace_back(&observer);
}
void Engine::priv::EventDispatcher::unregisterObject(Observer& observer, EventType eventType) noexcept {
    //ASSERT(m_Observers.size() > (size_t)eventType, __FUNCTION__ << "(): eventType does not fit in m_Observers!");
    if (m_Observers.size() > (size_t)eventType) {
        std::lock_guard lock{ m_Mutex };
        m_UnregisteredObservers.emplace_back(&observer, (size_t)eventType);
    }
}
bool Engine::priv::EventDispatcher::isObjectRegistered(const Observer& observer, EventType eventType) const noexcept {
    std::lock_guard lock{ m_Mutex };
    const auto& observers_with_event_type = m_Observers[(size_t)eventType];
    bool result = internal_has_duplicate(observer, observers_with_event_type);
    return result;
}
void Engine::priv::EventDispatcher::dispatchEvent(const Event& e) noexcept {
    if ((size_t)e.type >= m_Observers.size()) {
        return;
    }
    const auto& observers_with_event_type = m_Observers[(size_t)e.type];
    std::for_each(std::begin(observers_with_event_type), std::end(observers_with_event_type), [&e](auto& observer) {
        observer->onEvent(e);
    });
}
void Engine::priv::EventDispatcher::dispatchEvent(EventType eventType) noexcept {
    if ((size_t)eventType >= m_Observers.size()) {
        return;
    }
    Event e{ eventType };
    const auto& observers_with_event_type = m_Observers[(size_t)e.type];
    std::for_each(std::begin(observers_with_event_type), std::end(observers_with_event_type), [&e](auto& observer) {
        observer->onEvent(e);
    });
}
void Engine::priv::EventDispatcher::postUpdate() {
    if (m_UnregisteredObservers.size() > 0) {
        std::lock_guard lock{ m_Mutex };
        std::for_each(std::cbegin(m_UnregisteredObservers), std::cend(m_UnregisteredObservers), [this](const auto& data) {
            std::erase_if(m_Observers[data.second], [&data](auto observerItr) {
                return observerItr == data.first;
            });
        });
        m_UnregisteredObservers.clear();
    }
}