#pragma once
#ifndef ENGINE_EVENT_DISPATCHER_H
#define ENGINE_EVENT_DISPATCHER_H

#include <core/engine/events/Engine_EventObject.h>

namespace Engine{
namespace epriv{
    class EventDispatcher final{
        private:
            std::vector<std::vector<EventObserver*>> m_Observers;
        public:
            EventDispatcher();
            ~EventDispatcher();

            void registerObject(EventObserver*, const EventType::Type);
            void unregisterObject(EventObserver*, const EventType::Type);
            void dispatchEvent(const Event& e);
    };
};
};
#endif