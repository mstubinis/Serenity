#pragma once
#ifndef ENGINE_EVENT_DISPATCHER_H
#define ENGINE_EVENT_DISPATCHER_H

class  Observer;
struct Event;

#include <serenity/events/EventIncludes.h>
#include <array>
#include <utility>
#include <mutex>
#include <serenity/lua/LuaIncludes.h>

namespace Engine::priv {
    class EventDispatcher final {
        private:
            std::array<std::vector<Observer*>, EventType::_TOTAL>           m_Observers;

            std::array<std::vector<uint32_t>, EventType::_TOTAL>            m_ScriptObservers;
            std::vector<luabridge::LuaRef>                                  m_ScriptFunctions;

            mutable std::mutex                                              m_Mutex;
            std::vector<std::pair<Observer*, size_t>>                       m_UnregisteredObservers;

        public:
            EventDispatcher() = default;
            EventDispatcher(const EventDispatcher&)                = delete;
            EventDispatcher& operator=(const EventDispatcher&)     = delete;
            EventDispatcher(EventDispatcher&&) noexcept            = delete;
            EventDispatcher& operator=(EventDispatcher&&) noexcept = delete;

            void postUpdate();
            void addScriptOnEventFunction(lua_State*, uint32_t scriptID, luabridge::LuaRef eventFunction);
            void registerScriptEvent(uint32_t scriptID, uint32_t eventID);
            void cleanupScript(uint32_t scriptID);

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

namespace Engine::lua {
    void addOnEventFunction(luabridge::LuaRef eventFunction);
    void registerEvent(uint32_t eventID);
}

#endif