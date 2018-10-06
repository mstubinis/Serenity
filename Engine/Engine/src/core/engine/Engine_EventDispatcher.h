#pragma once
#ifndef ENGINE_EVENT_DISPATCHER_H
#define ENGINE_EVENT_DISPATCHER_H

#include <vector>
#include "core/engine/Engine_EventObject.h"

namespace Engine{
    namespace epriv{
        class EventDispatcher final{
            private:
                std::vector<std::vector<EventObserver*>> m_Observers;
            public:
                EventDispatcher(const char* name,uint w,uint h);
                ~EventDispatcher();

                void _registerObject(EventObserver*,EventType::Type);
                void _unregisterObject(EventObserver*,EventType::Type);
                void _dispatchEvent(EventType::Type,const Event& e);
                void _dispatchEvent(const Event& e);
        };
    };
};
#endif