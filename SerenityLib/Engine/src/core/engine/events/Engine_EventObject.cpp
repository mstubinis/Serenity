#include <core/engine/system/Engine.h>
#include <core/engine/events/Engine_EventObject.h>
#include <core/engine/events/Engine_EventDispatcher.h>

using namespace Engine;
using namespace std;

void EventObserver::registerEvent(const EventType::Type& type) {
    epriv::Core::m_Engine->m_EventManager.m_EventDispatcher.registerObject(this, type);
}
void EventObserver::unregisterEvent(const EventType::Type& type){
    epriv::Core::m_Engine->m_EventManager.m_EventDispatcher.unregisterObject(this, type);
}