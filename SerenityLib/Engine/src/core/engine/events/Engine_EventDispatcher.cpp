#include <core/engine/events/Engine_EventDispatcher.h>

using namespace Engine;
using namespace std;

epriv::EventDispatcher::EventDispatcher(){ 
    m_Observers.resize(EventType::_TOTAL);
}
epriv::EventDispatcher::~EventDispatcher(){ 
    for (auto& v : m_Observers) {
        vector_clear(v);
    }
    vector_clear(m_Observers);
}
void epriv::EventDispatcher::registerObject(EventObserver* obj, const EventType::Type& eventType){
    auto& v = m_Observers[eventType];
    for (auto& o : v) { 
        if (o == obj) { 
            return; 
        } 
    }
    v.push_back(obj);
}
void epriv::EventDispatcher::unregisterObject(EventObserver* obj, const EventType::Type& eventType){
    auto& v = m_Observers[eventType];
    removeFromVector(v, obj);
}
void epriv::EventDispatcher::dispatchEvent(const Event& _event){ 
    const auto& v = m_Observers[_event.type];
    for (auto& obj : v) {
        obj->onEvent(_event);
    }
}