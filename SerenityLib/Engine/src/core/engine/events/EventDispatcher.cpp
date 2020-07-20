#include "core/engine/utils/PrecompiledHeader.h"
#include <core/engine/events/EventDispatcher.h>
#include <core/engine/utils/Utils.h>
#include <core/engine/events/Observer.h>
#include <core/engine/events/Event.h>

using namespace Engine;
using namespace std;

priv::EventDispatcher::EventDispatcher(){ 
    m_Observers.resize((unsigned int)EventType::_TOTAL);
}
priv::EventDispatcher::~EventDispatcher(){ 
    /*
    for (auto& v : m_Observers) {
        v.clear();
    }
    m_Observers.clear();
    */
}
void priv::EventDispatcher::registerObject(Observer& observer, const EventType eventType){
    auto& observers_with_event_type = m_Observers[(unsigned int)eventType];
    if (internal_check_for_duplicates(&observer, observers_with_event_type)) {
        return;
    }
    observers_with_event_type.push_back(&observer);
}
void priv::EventDispatcher::unregisterObject(Observer& observer, const EventType eventType){
    if (m_Observers.size() <= (unsigned int)eventType) {
        return;
    }
    auto& observers_with_event_type = m_Observers[(unsigned int)eventType];
    removeFromVector(observers_with_event_type, &observer);
}
bool priv::EventDispatcher::isObjectRegistered(const Observer& observer, const EventType eventType) const {
    const auto& observers_with_event_type = m_Observers[(unsigned int)eventType];
    return internal_check_for_duplicates(&observer, observers_with_event_type);
}
void priv::EventDispatcher::dispatchEvent(const Event& event) {
    const auto& observers_with_event_type = m_Observers[(unsigned int)event.type];
    for (size_t i = 0; i < observers_with_event_type.size(); ++i) {
        observers_with_event_type[i]->onEvent(event);
    }
}
void priv::EventDispatcher::dispatchEvent(EventType eventType) {
    Event event{ eventType };
    const auto& observers_with_event_type = m_Observers[(unsigned int)event.type];
    for (size_t i = 0; i < observers_with_event_type.size(); ++i) {
        observers_with_event_type[i]->onEvent(event);
    }
}