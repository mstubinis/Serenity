#include <serenity/events/EventDispatcher.h>
#include <serenity/utils/Utils.h>
#include <serenity/events/Observer.h>
#include <serenity/events/Event.h>
#include <serenity/lua/LuaModule.h>
#include <serenity/system/Engine.h>

namespace {
    [[nodiscard]] bool internal_has_duplicate(const Observer& observer, const std::vector<Observer*>& vectorContainer) noexcept {
        return std::any_of(std::cbegin(vectorContainer), std::cend(vectorContainer), [&observer](const auto& o) {
            return o == &observer;
        });
    }
    void internal_dispatch_script_event(const Event& e, std::vector<luabridge::LuaRef>& functions) {
        for (auto& func : functions) {
            if (!func.isNil()) {
                func(e);
            }
        }
    }
    void internal_dispatch_event(const Event& e, const std::array<std::vector<Observer*>, EventType::_TOTAL>& observers) {
        if (e.type < observers.size()) {
            for (auto& observerPtr : observers[e.type]) {
                observerPtr->onEvent(e);
            }
        }
    }
}


void Engine::priv::EventDispatcher::registerObject(Observer& observer, EventType eventType) noexcept {
    std::lock_guard lock{ m_Mutex };
    auto& observers_with_event_type = m_Observers[eventType];
    if (internal_has_duplicate(observer, observers_with_event_type)) {
        return;
    }
    observers_with_event_type.emplace_back(&observer);
}
void Engine::priv::EventDispatcher::unregisterObject(Observer& observer, EventType eventType) noexcept {
    //ASSERT(m_Observers.size() > eventType, __FUNCTION__ << "(): eventType does not fit in m_Observers!");
    if (m_Observers.size() > eventType) {
        std::lock_guard lock{ m_Mutex };
        m_UnregisteredObservers.emplace_back(&observer, eventType);
    }
}
void Engine::priv::EventDispatcher::unregisterObjectImmediate(Observer& observer, EventType eventType) noexcept {
    //ASSERT(m_Observers.size() > eventType, __FUNCTION__ << "(): eventType does not fit in m_Observers!");
    if (m_Observers.size() > eventType) {
        std::lock_guard lock{ m_Mutex };
        auto& observers_with_event = m_Observers[eventType];
        auto it = observers_with_event.begin();
        while (it != observers_with_event.end()) {
            if (&observer == *it) {
                it = observers_with_event.erase(it);
            }else{
                ++it;
            }
        }
    }
}
bool Engine::priv::EventDispatcher::isObjectRegistered(const Observer& observer, EventType eventType) const noexcept {
    std::lock_guard lock{ m_Mutex };
    return internal_has_duplicate(observer, m_Observers[eventType]);
}
void Engine::priv::EventDispatcher::dispatchEvent(const Event& e) noexcept {
    internal_dispatch_event(e, m_Observers);
    internal_dispatch_script_event(e, m_ScriptFunctions);
}
void Engine::priv::EventDispatcher::dispatchEvent(EventType eventType) noexcept {
    Event e = Event{ eventType };
    internal_dispatch_event(e, m_Observers);
    internal_dispatch_script_event(e, m_ScriptFunctions);
}
void Engine::priv::EventDispatcher::postUpdate() {
    if (m_UnregisteredObservers.size() > 0) {
        std::lock_guard lock{ m_Mutex };
        for (const auto& [observer, idx] : m_UnregisteredObservers) {
            auto& observers_with_event = m_Observers[idx];
            auto it = observers_with_event.begin();
            while (it != observers_with_event.end()) {
                if (observer == *it) {
                    it = observers_with_event.erase(it);
                } else {
                    ++it;
                }
            }
        }
        m_UnregisteredObservers.clear();
    }
}
void Engine::priv::EventDispatcher::addScriptOnEventFunction(lua_State* L, size_t scriptID, luabridge::LuaRef eventFunction) {
    m_ScriptFunctions.resize(scriptID + 1, luabridge::LuaRef{ L });
    m_ScriptFunctions[scriptID] = eventFunction;
}
void Engine::priv::EventDispatcher::cleanupScript(size_t scriptID) {
    if (m_ScriptFunctions.size() > scriptID) {
        m_ScriptFunctions[scriptID] = luabridge::LuaRef(Engine::priv::getLUABinder().getState()->getState());
    }
}



void Engine::lua::addOnEventFunction(luabridge::LuaRef eventFunction) {
    lua_State* L     = Engine::priv::getLUABinder().getState()->getState();
    auto scriptIDRef = luabridge::getGlobal(L, ENGINE_LUA_CURRENT_SCRIPT_TOKEN);
    size_t scriptID  = scriptIDRef.cast<size_t>();
    auto& dispatcher = Engine::priv::Core::m_Engine->m_EventModule.m_EventDispatcher;
    dispatcher.addScriptOnEventFunction(L, scriptID, eventFunction);
}