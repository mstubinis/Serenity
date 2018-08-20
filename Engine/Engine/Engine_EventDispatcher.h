#pragma once
#ifndef ENGINE_EVENT_DISPATCHER_H
#define ENGINE_EVENT_DISPATCHER_H

#include <memory>
#include "Engine_EventObject.h"

namespace Engine{
    namespace epriv{
        class EventDispatcher final{
            private:
                class impl; std::unique_ptr<impl> m_i;
                EventDispatcher(const EventDispatcher&); // non construction-copyable
                EventDispatcher& operator=(const EventDispatcher&); // non copyable
            public:
                EventDispatcher(const char* name,uint w,uint h);
                ~EventDispatcher();

                void _update(const float& dt);
                void _init(const char* name,uint w,uint h);

                void _registerObject(EventObserver*,EventType::Type);
                void _unregisterObject(EventObserver*,EventType::Type);
                void _dispatchEvent(EventType::Type,const Event& e);
        };
    };
};
#endif