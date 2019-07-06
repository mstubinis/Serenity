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
void epriv::EventDispatcher::registerObject(EventObserver* obj, const EventType::Type type){
    auto& v = m_Observers[type];
    for (auto& o : v) { 
        if (o == obj) { 
            return; 
        } 
    }
    v.push_back(obj);
}
void epriv::EventDispatcher::unregisterObject(EventObserver* obj, const EventType::Type type){
    auto& v = m_Observers[type];
    removeFromVector(v, obj);
}
void epriv::EventDispatcher::dispatchEvent(const Event& e){ 
    const auto& v = m_Observers[e.type];
    for (auto& obj : v) {
        obj->onEvent(e);
    }
}