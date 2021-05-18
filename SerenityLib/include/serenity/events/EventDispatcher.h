#pragma once
#ifndef ENGINE_EVENT_DISPATCHER_H
#define ENGINE_EVENT_DISPATCHER_H

class  Observer;
struct Event;

#include <serenity/events/EventIncludes.h>
#include <vector>
#include <array>
#include <utility>
#include <mutex>

namespace Engine::priv {
    class EventDispatcher final {
        using ObserverVector = std::vector<Observer*>;
        private:
            mutable std::mutex                                       m_Mutex;
            std::array<ObserverVector, (size_t)EventType::_TOTAL>    m_Observers;
            std::vector<std::pair<Observer*, size_t>>                m_UnregisteredObservers;

            [[nodiscard]] bool internal_has_duplicate(const Observer&, const ObserverVector&) const noexcept;
            void internal_dispatch_event(const Event&);
        public:
            EventDispatcher() = default;
            EventDispatcher(const EventDispatcher&)                = delete;
            EventDispatcher& operator=(const EventDispatcher&)     = delete;
            EventDispatcher(EventDispatcher&&) noexcept            = delete;
            EventDispatcher& operator=(EventDispatcher&&) noexcept = delete;

            void postUpdate();

            template<class T> void registerObject(Observer&, const T&) noexcept = delete;
            template<class T> void unregisterObject(Observer&, const T&) noexcept = delete;
            template<class T> void unregisterObjectImmediate(Observer&, const T&) noexcept = delete;
            template<class T> [[nodiscard]] bool isObjectRegistered(Observer&, const T&) const noexcept = delete;

            void registerObject(Observer&, EventType) noexcept;
            void unregisterObject(Observer&, EventType) noexcept;
            void unregisterObjectImmediate(Observer&, EventType) noexcept;
            [[nodiscard]] bool isObjectRegistered(const Observer&, EventType) const noexcept;

            void dispatchEvent(const Event&) noexcept;
            void dispatchEvent(EventType) noexcept;
    };
};
#endif