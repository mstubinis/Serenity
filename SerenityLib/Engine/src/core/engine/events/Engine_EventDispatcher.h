#pragma once
#ifndef ENGINE_EVENT_DISPATCHER_H
#define ENGINE_EVENT_DISPATCHER_H

struct EventObserver;
struct Event;

#include <core/engine/events/Engine_EventIncludes.h>
#include <vector>

namespace Engine::priv{
    class EventDispatcher final{
        private:
            std::vector<std::vector<EventObserver*>> m_Observers;
            std::vector<std::vector<EventObserver*>> m_ObserversCustom;
        public:
            EventDispatcher();
            ~EventDispatcher();

            EventDispatcher(const EventDispatcher&)                      = delete;
            EventDispatcher& operator=(const EventDispatcher&)           = delete;
            EventDispatcher(EventDispatcher&& other) noexcept            = delete;
            EventDispatcher& operator=(EventDispatcher&& other) noexcept = delete;

            template <class T> void registerObject(EventObserver*, const T&) = delete;
            template <class T> void unregisterObject(EventObserver*, const T&) = delete;

            void registerObject(EventObserver*, const EventType::Type& eventType);
            void unregisterObject(EventObserver*, const EventType::Type& eventType);

            void registerObject(EventObserver*, const unsigned int& eventType);
            void unregisterObject(EventObserver*, const unsigned int& eventType);

            void dispatchEvent(const Event& _event);
            void dispatchEvent(const unsigned int& _eventType);
    };
};
#endif