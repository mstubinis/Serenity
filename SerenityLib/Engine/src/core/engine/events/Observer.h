#pragma once
#ifndef ENGINE_EVENTS_OBSERVER_H
#define ENGINE_EVENTS_OBSERVER_H

struct Event;

#include <core/engine/events/EventIncludes.h>

/*
Inherit from this class to expose your class to events and event dispatching, specifically the following functions:
    void registerEvent(EventType type)    -  register this object as an observer to the parameterized event type
    void unregisterEvent(EventType type)  -  unregister this object as an observer to the parameterized event type
    virtual void onEvent(const Event& e)  -  execute this function when the event occurs
*/
class Observer {
    public:
        Observer() = default;
        virtual ~Observer() = default;

        void registerEvent(EventType type) noexcept;
        void unregisterEvent(EventType type) noexcept;
        bool isRegistered(EventType type) const noexcept;
        
        virtual void onEvent(const Event& e) {}
};

#endif