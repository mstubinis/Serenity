#pragma once
#ifndef ENGINE_EVENT_DISPATCHER_H
#define ENGINE_EVENT_DISPATCHER_H

class  Observer;
struct Event;

#include <core/engine/events/EventIncludes.h>

namespace Engine::priv{
    class EventDispatcher final {
        private:
            mutable std::mutex                          m_Mutex;
            std::vector<std::vector<Observer*>>         m_Observers;
            std::vector<std::pair<Observer*, size_t>>   m_UnregisteredObservers;

            bool internal_has_duplicate(const Observer& observer, const std::vector<Observer*>& vectorContainer) const noexcept {
                return std::any_of(vectorContainer.cbegin(), vectorContainer.cend(), [&observer](const auto& o) {
                    return o == &observer;
                });
            }
        public:
            EventDispatcher();
            EventDispatcher(const EventDispatcher&)                = delete;
            EventDispatcher& operator=(const EventDispatcher&)     = delete;
            EventDispatcher(EventDispatcher&&) noexcept            = delete;
            EventDispatcher& operator=(EventDispatcher&&) noexcept = delete;

            void postUpdate();

            template<class T> void registerObject(Observer&, const T&) noexcept = delete;
            template<class T> void unregisterObject(Observer&, const T&) noexcept = delete;
            template<class T> bool isObjectRegistered(Observer&, const T&) const noexcept = delete;

            void registerObject(Observer&, EventType eventType) noexcept;
            void unregisterObject(Observer&, EventType eventType) noexcept;
            bool isObjectRegistered(const Observer&, EventType eventType) const noexcept;

            void dispatchEvent(const Event&) noexcept;
            void dispatchEvent(EventType eventType) noexcept;
    };
};
#endif