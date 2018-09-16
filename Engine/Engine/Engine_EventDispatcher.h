#pragma once
#ifndef ENGINE_EVENT_DISPATCHER_H
#define ENGINE_EVENT_DISPATCHER_H

#include <memory>
#include "Engine_EventObject.h"

namespace Engine{
    namespace epriv{
        class EventDispatcher final: private Engine::epriv::noncopyable{
            private:
                class impl; std::unique_ptr<impl> m_i;
            public:
                EventDispatcher(const char* name,uint w,uint h);
                ~EventDispatcher();

                void _update(const float& dt);

                void _registerObject(EventObserver*,EventType::Type);
                void _unregisterObject(EventObserver*,EventType::Type);
                void _dispatchEvent(EventType::Type,const Event& e);
				void _dispatchEvent(const Event& e);
        };
    };
};
#endif