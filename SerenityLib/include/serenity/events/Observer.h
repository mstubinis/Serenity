#pragma once
#ifndef ENGINE_EVENTS_OBSERVER_H
#define ENGINE_EVENTS_OBSERVER_H

struct Event;

#include <serenity/events/EventIncludes.h>

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

        void registerEvent(EventType) noexcept;
        void unregisterEvent(EventType) noexcept;
        void unregisterEventImmediate(EventType) noexcept;
        [[nodiscard]] bool isRegistered(EventType) const noexcept;
        
        virtual void onEvent(const Event&) {}
};

#endif