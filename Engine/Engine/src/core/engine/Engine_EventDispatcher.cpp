#include "core/engine/Engine_EventDispatcher.h"
#include <vector>

using namespace Engine;
using namespace std;

epriv::EventDispatcher::EventDispatcher(const char* name,uint w,uint h){ 
    m_Observers.resize(EventType::_TOTAL);
}
epriv::EventDispatcher::~EventDispatcher(){ 
    for (auto& v : m_Observers) {
        vector_clear(v);
    }
    vector_clear(m_Observers);
}
void epriv::EventDispatcher::_registerObject(EventObserver* obj,EventType::Type type){ 
    auto& v = m_Observers[type];
    for (auto& o : v) { if (o == obj) { return; } }
    v.push_back(obj);
}
void epriv::EventDispatcher::_unregisterObject(EventObserver* obj,EventType::Type type){ 
    auto& v = m_Observers[type];
    removeFromVector(v, obj);
}
void epriv::EventDispatcher::_dispatchEvent(EventType::Type type,const Event& e){ 
    auto& v = m_Observers[type];
    for (auto& obj : v) {
        obj->onEvent(e);
    }
}
void epriv::EventDispatcher::_dispatchEvent(const Event& e){ 
    auto& v = m_Observers[e.type];
    for (auto& obj : v) {
        obj->onEvent(e);
    }
}