#include <serenity/core/engine/events/Observer.h>
#include <serenity/core/engine/system/Engine.h>
#include <serenity/core/engine/events/EventDispatcher.h>

void Observer::registerEvent(EventType type) noexcept {
    Engine::priv::Core::m_Engine->m_EventModule.m_EventDispatcher.registerObject(*this, type);
}
void Observer::unregisterEvent(EventType type) noexcept {
    Engine::priv::Core::m_Engine->m_EventModule.m_EventDispatcher.unregisterObject(*this, type);
}
bool Observer::isRegistered(EventType type) const noexcept {
    return Engine::priv::Core::m_Engine->m_EventModule.m_EventDispatcher.isObjectRegistered(*this, type);
}
