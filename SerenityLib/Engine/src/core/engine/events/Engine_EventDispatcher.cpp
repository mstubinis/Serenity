#include <core/engine/events/Engine_EventDispatcher.h>
#include <core/engine/utils/Utils.h>
#include <core/engine/events/Engine_EventObject.h>

using namespace Engine;
using namespace std;

priv::EventDispatcher::EventDispatcher(){ 
    m_Observers.resize(static_cast<size_t>(EventType::_TOTAL - CONST_EVENT_TYPE_ENGINE_OFFSET));
}
priv::EventDispatcher::~EventDispatcher(){ 
    for (auto& v : m_Observers) {
        v.clear();
    }
    m_Observers.clear();
}
void priv::EventDispatcher::registerObject(EventObserver* observer, const EventType::Type& eventType){
    auto& observers_with_event_type = m_Observers[eventType - CONST_EVENT_TYPE_ENGINE_OFFSET];
    for (auto& o : observers_with_event_type) {
        if (o == observer) { return; } 
    }
    observers_with_event_type.push_back(observer);
}
void priv::EventDispatcher::unregisterObject(EventObserver* observer, const EventType::Type& eventType){
    if (m_Observers.size() <= eventType - CONST_EVENT_TYPE_ENGINE_OFFSET)
        return;
    auto& observers_with_event_type = m_Observers[eventType - CONST_EVENT_TYPE_ENGINE_OFFSET];
    removeFromVector(observers_with_event_type, observer);
}
void priv::EventDispatcher::dispatchEvent(const Event& _event) {
    const auto& observers_with_event_type = m_Observers[_event.type - CONST_EVENT_TYPE_ENGINE_OFFSET];
    for (size_t i = 0; i < observers_with_event_type.size(); ++i) {
        observers_with_event_type[i]->onEvent(_event);
    }
}



void priv::EventDispatcher::registerObject(EventObserver* observer, const unsigned int& eventType) {
    if (m_ObserversCustom.size() <= eventType)
        m_ObserversCustom.resize(static_cast<size_t>(eventType + 1U));

    auto& observers_with_event_type = m_ObserversCustom[eventType];
    for (auto& o : observers_with_event_type) {
        if (o == observer) { return; }
    }
    observers_with_event_type.push_back(observer);
}
void priv::EventDispatcher::unregisterObject(EventObserver* observer, const unsigned int& eventType) {
    if (m_ObserversCustom.size() <= eventType)
        return;
    removeFromVector(m_ObserversCustom[eventType], observer);
}
void priv::EventDispatcher::dispatchEvent(const unsigned int& eventType) {
    if (m_ObserversCustom.size() <= eventType)
        return;
    const auto& observers_with_event_type = m_ObserversCustom[eventType];
    for (size_t i = 0; i < observers_with_event_type.size(); ++i) {
        Event e(static_cast<EventType::Type>(eventType));
        observers_with_event_type[i]->onEvent(e);
    }
}