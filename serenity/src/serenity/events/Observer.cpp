#include <serenity/events/Observer.h>
#include <serenity/system/Engine.h>
#include <serenity/events/EventDispatcher.h>

void Observer::registerEvent(EventType type) noexcept {
    registerEvent(type, Engine::priv::Core::m_Engine->m_EventModule.m_EventDispatcher);
}
void Observer::unregisterEvent(EventType type) noexcept {
    unregisterEvent(type, Engine::priv::Core::m_Engine->m_EventModule.m_EventDispatcher);
}
void Observer::unregisterEventImmediate(EventType type) noexcept {
    unregisterEventImmediate(type, Engine::priv::Core::m_Engine->m_EventModule.m_EventDispatcher);
}
bool Observer::isRegistered(EventType type) const noexcept {
    return isRegistered(type, Engine::priv::Core::m_Engine->m_EventModule.m_EventDispatcher);
}



void Observer::registerEvent(EventType type, Engine::priv::EventDispatcher& dispatcher) noexcept {
    dispatcher.registerObject(*this, type);
}
void Observer::unregisterEvent(EventType type, Engine::priv::EventDispatcher& dispatcher) noexcept {
    dispatcher.unregisterObject(*this, type);
}
void Observer::unregisterEventImmediate(EventType type, Engine::priv::EventDispatcher& dispatcher) noexcept {
    dispatcher.unregisterObjectImmediate(*this, type);
}
bool Observer::isRegistered(EventType type, Engine::priv::EventDispatcher& dispatcher) const noexcept {
    return dispatcher.isObjectRegistered(*this, type);
}
