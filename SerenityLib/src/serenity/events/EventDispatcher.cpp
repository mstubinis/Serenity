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
    void internal_dispatch_script_event(const Event& e, std::vector<luabridge::LuaRef>& functions, const std::array<std::vector<uint32_t>, EventType::_TOTAL>& observers) {
        if (e.type < observers.size()) {
            for (uint32_t scriptID : observers[e.type]) {
                if (!functions[scriptID].isNil()) {
                    functions[scriptID](e);
                }
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


Engine::priv::EventDispatcher::~EventDispatcher() {
    m_UnregisteredObservers.clear();
}
void Engine::priv::EventDispatcher::registerObject(Observer& observer, EventType eventType) noexcept {
    ASSERT(eventType >= 0 && eventType < m_Observers.size(), "");
    std::lock_guard lock{ m_Mutex };
    auto& observers_with_event_type = m_Observers[eventType];
    if (internal_has_duplicate(observer, observers_with_event_type)) {
        return;
    }
    observers_with_event_type.emplace_back(&observer);
}
void Engine::priv::EventDispatcher::unregisterObject(Observer& observer, EventType eventType) noexcept {
    if (m_Observers.size() > eventType) {
        std::lock_guard lock{ m_Mutex };
        m_UnregisteredObservers.emplace_back(&observer, eventType);
    }
}
void Engine::priv::EventDispatcher::unregisterObjectImmediate(Observer& observer, EventType eventType) noexcept {
    if (m_Observers.size() > eventType) {
        std::lock_guard lock{ m_Mutex };
        auto& observers_with_event = m_Observers[eventType];
        Engine::swap_and_pop_single(observers_with_event, [](auto& item, Observer* inObserver) {
            return inObserver == item;
        }, &observer);
    }
}
bool Engine::priv::EventDispatcher::isObjectRegistered(const Observer& observer, EventType eventType) const noexcept {
    std::lock_guard lock{ m_Mutex };
    return internal_has_duplicate(observer, m_Observers[eventType]);
}
void Engine::priv::EventDispatcher::dispatchEvent(const Event& e) noexcept {
    internal_dispatch_event(e, m_Observers);
    internal_dispatch_script_event(e, m_ScriptFunctions, m_ScriptObservers);
}
void Engine::priv::EventDispatcher::dispatchEvent(EventType eventType) noexcept {
    Event e = Event{ eventType };
    internal_dispatch_event(e, m_Observers);
    internal_dispatch_script_event(e, m_ScriptFunctions, m_ScriptObservers);
}
void Engine::priv::EventDispatcher::postUpdate() {
    if (m_UnregisteredObservers.size() > 0) {
        std::lock_guard lock{ m_Mutex };
        for (const auto& [observer, idx] : m_UnregisteredObservers) {
            Engine::swap_and_pop(m_Observers[idx], [](auto& item, Observer* observer) {
                return observer == item;
            }, observer);
        }
        m_UnregisteredObservers.clear();
    }
}
void Engine::priv::EventDispatcher::addScriptOnEventFunction(lua_State* L, uint32_t scriptID, luabridge::LuaRef eventFunction) {
    m_ScriptFunctions.resize(scriptID + 1, luabridge::LuaRef{ L });
    m_ScriptFunctions[scriptID] = eventFunction;
}
void Engine::priv::EventDispatcher::cleanupScript(uint32_t scriptID) {
    if (m_ScriptFunctions.size() > scriptID) {
        m_ScriptFunctions[scriptID] = luabridge::LuaRef(&Engine::lua::getGlobalState());
    }
    for (auto& scriptIDs : m_ScriptObservers) {
        size_t idx = Engine::binary_search(scriptIDs, scriptID);
        if (idx != std::numeric_limits<size_t>().max()) {
            auto lastIdx = scriptIDs.size() - 1;
            if (idx != lastIdx) {
                scriptIDs[idx] = std::move(scriptIDs[lastIdx]);
            }
            scriptIDs.pop_back();
            Engine::insertion_sort(scriptIDs);
        }
    }
}
void Engine::priv::EventDispatcher::registerScriptEvent(uint32_t scriptID, uint32_t eventID) {
    ASSERT(eventID >= 0 && eventID < m_ScriptObservers.size(), "");
    //binary search to see if scriptID is already in the container
    size_t idx = Engine::binary_search(m_ScriptObservers[eventID], scriptID);
    if (idx == std::numeric_limits<size_t>().max()) {
        m_ScriptObservers[eventID].push_back(scriptID);
        Engine::insertion_sort(m_ScriptObservers[eventID]);
    }
}



void Engine::lua::addOnEventFunction(luabridge::LuaRef eventFunction) {
    lua_State* L       = &Engine::lua::getGlobalState();
    auto scriptIDRef   = luabridge::getGlobal(L, ENGINE_LUA_CURRENT_SCRIPT_TOKEN);
    uint32_t scriptID  = scriptIDRef.cast<uint32_t>();
    auto& dispatcher   = Engine::priv::Core::m_Engine->m_EventModule.m_EventDispatcher;
    dispatcher.addScriptOnEventFunction(L, scriptID, eventFunction);
}
void Engine::lua::registerEvent(uint32_t eventID) {
    lua_State* L      = &Engine::lua::getGlobalState();
    auto scriptIDRef  = luabridge::getGlobal(L, ENGINE_LUA_CURRENT_SCRIPT_TOKEN);
    uint32_t scriptID = scriptIDRef.cast<uint32_t>();
    auto& dispatcher  = Engine::priv::Core::m_Engine->m_EventModule.m_EventDispatcher;
    dispatcher.registerScriptEvent(scriptID, eventID);
}