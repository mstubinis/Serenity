#pragma once
#ifndef ENGINE_EVENTS_OBSERVER_H
#define ENGINE_EVENTS_OBSERVER_H

struct Event;

#include <core/engine/events/EventIncludes.h>

/*
Inherit from this class to expose your class to events and event dispatching, specifically the following functions:
    void registerEvent(EventType::Type type)    -  register this object as an observer to the parameterized event type
    void unregisterEvent(EventType::Type type)  -  unregister this object as an observer to the parameterized event type
    virtual void onEvent(const Event& e)        -  execute this function when the event occurs
*/
class Observer {
    private:

    public:
        Observer();
        virtual ~Observer();

        void registerEvent(EventType::Type type);
        void unregisterEvent(EventType::Type type);
        bool isRegistered(EventType::Type type) const;

        void registerEvent(unsigned int type);
        void unregisterEvent(unsigned int type);
        bool isRegistered(unsigned int type) const;

        virtual void onEvent(const Event& e);
};

#endif