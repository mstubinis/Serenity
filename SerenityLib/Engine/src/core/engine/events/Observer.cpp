#include "core/engine/utils/PrecompiledHeader.h"
#include <core/engine/events/Observer.h>
#include <core/engine/system/Engine.h>
#include <core/engine/events/EventDispatcher.h>

using namespace Engine;

Observer::Observer() {

}
Observer::~Observer() {

}
void Observer::registerEvent(EventType type) {
    priv::Core::m_Engine->m_EventModule.m_EventDispatcher.registerObject(*this, type);
}
void Observer::unregisterEvent(EventType type) {
    priv::Core::m_Engine->m_EventModule.m_EventDispatcher.unregisterObject(*this, type);
}
bool Observer::isRegistered(EventType type) const {
    return priv::Core::m_Engine->m_EventModule.m_EventDispatcher.isObjectRegistered(*this, type);
}

void Observer::onEvent(const Event& event) {

}