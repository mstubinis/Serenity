#include "core/engine/Engine.h"
#include "core/engine/Engine_EventObject.h"
#include "core/engine/Engine_EventDispatcher.h"

using namespace Engine;
using namespace std;

EventObserver::EventObserver(){}
EventObserver::~EventObserver(){}
void EventObserver::registerEvent(const EventType::Type& type) {
    epriv::Core::m_Engine->m_EventDispatcher._registerObject(this, type);
}
void EventObserver::unregisterEvent(const EventType::Type& type){
    epriv::Core::m_Engine->m_EventDispatcher._unregisterObject(this, type);
}
void EventObserver::onEvent(const Event& e){}