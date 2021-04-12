#include <serenity/events/Observer.h>
#include <serenity/system/Engine.h>
#include <serenity/events/EventDispatcher.h>

void Observer::registerEvent(EventType type) noexcept {
    Engine::priv::Core::m_Engine->m_EventModule.m_EventDispatcher.registerObject(*this, type);
}
void Observer::unregisterEvent(EventType type) noexcept {
    Engine::priv::Core::m_Engine->m_EventModule.m_EventDispatcher.unregisterObject(*this, type);
}
void Observer::unregisterEventImmediate(EventType type) noexcept {
    Engine::priv::Core::m_Engine->m_EventModule.m_EventDispatcher.unregisterObjectImmediate(*this, type);
}
bool Observer::isRegistered(EventType type) const noexcept {
    return Engine::priv::Core::m_Engine->m_EventModule.m_EventDispatcher.isObjectRegistered(*this, type);
}
