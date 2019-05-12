#pragma once
#ifndef ENGINE_EVENT_DISPATCHER_H
#define ENGINE_EVENT_DISPATCHER_H

#include <vector>
#include <core/engine/events/Engine_EventObject.h>

namespace Engine{
    namespace epriv{
        class EventDispatcher final{
            private:
                std::vector<std::vector<EventObserver*>> m_Observers;
            public:
                EventDispatcher();
                ~EventDispatcher();

                void registerObject(EventObserver*,EventType::Type);
                void unregisterObject(EventObserver*,EventType::Type);
                void dispatchEvent(EventType::Type,const Event& e);
                void dispatchEvent(const Event& e);
        };
    };
};
#endif