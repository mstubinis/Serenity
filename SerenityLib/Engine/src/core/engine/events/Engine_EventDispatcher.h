#pragma once
#ifndef ENGINE_EVENT_DISPATCHER_H
#define ENGINE_EVENT_DISPATCHER_H

#include <core/engine/events/Engine_EventIncludes.h>
#include <vector>

struct EventObserver;
struct Event;
namespace Engine{
    namespace epriv{
        class EventDispatcher final{
            private:
                std::vector<std::vector<EventObserver*>> m_Observers;
            public:
                EventDispatcher();
                ~EventDispatcher();

                void registerObject(EventObserver*, const EventType::Type& eventType);
                void unregisterObject(EventObserver*, const EventType::Type& eventType);
                void dispatchEvent(const Event& _event);
        };
    };
};
#endif