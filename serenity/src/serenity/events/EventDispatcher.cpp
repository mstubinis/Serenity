#include <serenity/events/EventDispatcher.h>
#include <serenity/utils/Utils.h>
#include <serenity/events/Observer.h>
#include <serenity/events/Event.h>
#include <serenity/lua/LuaModule.h>
#include <serenity/system/Engine.h>

namespace {
    [[nodiscard]] bool internal_has_duplicate(const Observer& observer, const std::vector<Observer*>& vectorContainer) noexcept {
        return std::any_of(std::cbegin(vectorContainer), std::cend(vectorContainer), [&observer](const auto& observer_ptr) {
            return observer_ptr == std::addressof(observer);
        });
    }
    void internal_dispatch_event(const Event& e, const std::array<std::vector<Observer*>, EventType::_TOTAL>& observers) {
        if (e.type < observers.size()) {
            for (auto& observerPtr : observers[e.type]) {
                observerPtr->onEvent(e);
            }
        }
    }
}


Engine::priv::EventDispatcher::~EventDispatcher() {
    m_UnregisteredObservers.clear();
}
void Engine::priv::EventDispatcher::registerObject(Observer& observer, EventType eventType) noexcept {
    ASSERT(eventType >= 0 && eventType < m_Observers.size(), "");
    std::scoped_lock lock{ m_Mutex };
    auto& observers_with_event_type = m_Observers[eventType];
    if (internal_has_duplicate(observer, observers_with_event_type)) {
        return;
    }
    observers_with_event_type.push_back(std::addressof(observer));
}
void Engine::priv::EventDispatcher::unregisterObject(Observer& observer, EventType eventType) noexcept {
    if (m_Observers.size() > eventType) {
        std::scoped_lock lock{ m_Mutex };
        m_UnregisteredObservers.emplace_back(std::addressof(observer), eventType);
    }
}
void Engine::priv::EventDispatcher::unregisterObjectImmediate(Observer& observer, EventType eventType) noexcept {
    if (m_Observers.size() > eventType) {
        std::scoped_lock lock{ m_Mutex };
        auto& observers_with_event = m_Observers[eventType];
        Engine::swap_and_pop_single(observers_with_event, [](const auto& item, Observer* inObserver) {
            return inObserver == item;
        }, std::addressof(observer));
    }
}
bool Engine::priv::EventDispatcher::isObjectRegistered(const Observer& observer, EventType eventType) const noexcept {
    std::scoped_lock lock{ m_Mutex };
    return internal_has_duplicate(observer, m_Observers[eventType]);
}
void Engine::priv::EventDispatcher::dispatchEvent(const Event& e) noexcept {
    internal_dispatch_event(e, m_Observers);
    Engine::priv::Core::m_Engine->m_LUAModule.onEvent(e);
}
void Engine::priv::EventDispatcher::dispatchEvent(EventType eventType) noexcept {
    dispatchEvent(Event{ eventType });
}
void Engine::priv::EventDispatcher::postUpdate() {
    if (m_UnregisteredObservers.size() > 0) {
        std::scoped_lock lock{ m_Mutex };
        for (const auto& [observer_ptr, idx] : m_UnregisteredObservers) {
            Engine::swap_and_pop(m_Observers[idx], [](const auto& item, Observer* observer) {
                return observer == item;
            }, observer_ptr);
        }
        m_UnregisteredObservers.clear();
    }
}