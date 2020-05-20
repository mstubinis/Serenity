#pragma once
#ifndef ENGINE_EVENT_DISPATCHER_H
#define ENGINE_EVENT_DISPATCHER_H

class  EventObserver;
struct Event;

#include <core/engine/events/Engine_EventIncludes.h>
#include <vector>

namespace Engine::priv{
    class EventDispatcher final : public Engine::NonCopyable, public Engine::NonMoveable{
        private:
            std::vector<std::vector<EventObserver*>> m_Observers;
            std::vector<std::vector<EventObserver*>> m_ObserversCustom;

            template <typename T> bool internal_check_for_duplicates(const T* observer, const std::vector<T*> vectorContainer) const {
                for (const auto& o : vectorContainer) {
                    if (o == observer) {
                        return true;
                    }
                }
                return false;
            }
        public:
            EventDispatcher();
            ~EventDispatcher();

            template <class T> void registerObject(EventObserver&, const T&) = delete;
            template <class T> void unregisterObject(EventObserver&, const T&) = delete;
            template <class T> bool isObjectRegistered(EventObserver&, const T&) const = delete;

            void registerObject(EventObserver&, const EventType::Type eventType);
            void unregisterObject(EventObserver&, const EventType::Type eventType);
            bool isObjectRegistered(const EventObserver&, const EventType::Type eventType) const;

            void registerObject(EventObserver&, const unsigned int eventType);
            void unregisterObject(EventObserver&, const unsigned int eventType);
            bool isObjectRegistered(const EventObserver&, const unsigned int eventType) const;

            void dispatchEvent(const Event& event_);
            void dispatchEvent(const unsigned int eventType);
    };
};
#endif