#include <serenity/events/EventDispatcher.h>
#include <serenity/utils/Utils.h>
#include <serenity/events/Observer.h>
#include <serenity/events/Event.h>

bool Engine::priv::EventDispatcher::internal_has_duplicate(const Observer& observer, const ObserverVector& vectorContainer) const noexcept {
    return std::any_of(std::cbegin(vectorContainer), std::cend(vectorContainer), [&observer](const auto& o) {
        return o == &observer;
    });
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
    const auto& observers_with_event = m_Observers[(size_t)eventType];
    return internal_has_duplicate(observer, observers_with_event);
}
void Engine::priv::EventDispatcher::internal_dispatch_event(const Event& e) {
    if ((size_t)e.type >= m_Observers.size()) {
        return;
    }
    const auto& observers_with_event = m_Observers[(size_t)e.type];
    for (auto& observerPtr : observers_with_event) {
        observerPtr->onEvent(e);
    }
}
void Engine::priv::EventDispatcher::dispatchEvent(const Event& e) noexcept {
    internal_dispatch_event(e);
}
void Engine::priv::EventDispatcher::dispatchEvent(EventType eventType) noexcept {
    Event e{ eventType };
    internal_dispatch_event(e);
}
void Engine::priv::EventDispatcher::postUpdate() {
    if (m_UnregisteredObservers.size() > 0) {
        std::lock_guard lock{ m_Mutex };
        for (const auto& [observer, idx] : m_UnregisteredObservers) {
            std::erase_if(m_Observers[idx], [observer](auto observerItr) {
                return observerItr == observer;
            });
        }
        m_UnregisteredObservers.clear();
    }
}