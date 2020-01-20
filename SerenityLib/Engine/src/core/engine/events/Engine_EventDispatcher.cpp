#include <core/engine/events/Engine_EventDispatcher.h>
#include <core/engine/utils/Utils.h>
#include <core/engine/events/Engine_EventObject.h>

using namespace Engine;
using namespace std;

epriv::EventDispatcher::EventDispatcher(){ 
    m_Observers.resize(EventType::_TOTAL);
}
epriv::EventDispatcher::~EventDispatcher(){ 
    for (auto& v : m_Observers) {
        v.clear();
    }
    m_Observers.clear();
}
void epriv::EventDispatcher::registerObject(EventObserver* observerToRegister, const EventType::Type& eventType){
    auto& observers_with_event_type = m_Observers[eventType];
    for (auto& o : observers_with_event_type) {
        if (o == observerToRegister) {
            return; 
        } 
    }
    observers_with_event_type.push_back(observerToRegister);
}
void epriv::EventDispatcher::unregisterObject(EventObserver* obj, const EventType::Type& eventType){
    if (m_Observers.size() <= eventType)
        return;
    auto& observers_with_event_type = m_Observers[eventType];
    removeFromVector(observers_with_event_type, obj);
}
void epriv::EventDispatcher::dispatchEvent(const Event& _event){ 
    const auto& observers_with_event_type = m_Observers[_event.type];
    for (size_t i = 0; i < observers_with_event_type.size(); ++i) {
        observers_with_event_type[i]->onEvent(_event);
    }
}