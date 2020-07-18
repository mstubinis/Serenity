#pragma once
#ifndef ENGINE_EVENT_DISPATCHER_H
#define ENGINE_EVENT_DISPATCHER_H

class  Observer;
struct Event;

#include <core/engine/events/EventIncludes.h>

namespace Engine::priv{
    class EventDispatcher final : public Engine::NonCopyable, public Engine::NonMoveable{
        private:
            std::vector<std::vector<Observer*>> m_Observers;

            template <typename T> bool internal_check_for_duplicates(const T* observer, const std::vector<T*> vectorContainer) const {
                for (const auto& o : vectorContainer) {
                    if (o == observer) { return true; }
                }
                return false;
            }
        public:
            EventDispatcher();
            ~EventDispatcher();

            template <class T> void registerObject(Observer&, const T&) = delete;
            template <class T> void unregisterObject(Observer&, const T&) = delete;
            template <class T> bool isObjectRegistered(Observer&, const T&) const = delete;

            void registerObject(Observer&, EventType::Type eventType);
            void unregisterObject(Observer&, EventType::Type eventType);
            bool isObjectRegistered(const Observer&, EventType::Type eventType) const;

            void dispatchEvent(const Event& event);
            void dispatchEvent(EventType::Type eventType);
    };
};
#endif