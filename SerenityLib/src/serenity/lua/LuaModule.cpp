#include <serenity/lua/LuaModule.h>
#include <serenity/system/Macros.h>
#include <serenity/events/Event.h>
#include <serenity/utils/Utils.h>

namespace {
    Engine::priv::LUAModule*                               LUA_MODULE = nullptr;
    std::array<std::vector<uint32_t>, EventType::_TOTAL>   LUA_SCRIPT_OBSERVERS; //[eventType] => list of script IDs registered to this event
    std::vector<std::pair<luabridge::LuaRef, Entity>>      LUA_SCRIPT_ON_EVENT_FUNCTIONS;
    std::vector<std::pair<luabridge::LuaRef, Entity>>      LUA_SCRIPT_ON_UPDATE_FUNCTIONS;

    void cleanup_on_update_function(uint32_t scriptID) {
        if (LUA_SCRIPT_ON_UPDATE_FUNCTIONS.size() > scriptID) {
            LUA_SCRIPT_ON_UPDATE_FUNCTIONS[scriptID] = { luabridge::LuaRef(&Engine::lua::getGlobalState()), Entity{} };
        }
    }
    void cleanup_on_event_function(uint32_t scriptID) {
        if (LUA_SCRIPT_ON_EVENT_FUNCTIONS.size() > scriptID) {
            LUA_SCRIPT_ON_EVENT_FUNCTIONS[scriptID] = { luabridge::LuaRef(&Engine::lua::getGlobalState()), Entity{} };
        }
        for (auto& scriptIDs : LUA_SCRIPT_OBSERVERS) {
            size_t idx = Engine::binary_search(scriptIDs, scriptID);
            if (idx != std::numeric_limits<size_t>().max()) {
                auto lastIdx = scriptIDs.size() - 1;
                if (idx != lastIdx) {
                    scriptIDs[idx] = std::move(scriptIDs[lastIdx]);
                }
                if (scriptIDs.size() > 1) {
                    scriptIDs.pop_back();
                } else {
                    scriptIDs.clear();
                }
                Engine::insertion_sort(scriptIDs);
            }
        }
    }
}

#pragma region LUAScriptInstance

void Engine::priv::LUAScriptInstance::registerEvent(uint32_t eventID) {
    lua_State* L      = &Engine::lua::getGlobalState();

    assert(eventID >= 0 && eventID < m_ScriptObservers.size());
    //binary search to see if scriptID is already in the container
    const size_t idx = Engine::binary_search(LUA_SCRIPT_OBSERVERS[eventID], m_ScriptID);
    if (idx == std::numeric_limits<size_t>().max()) {
        LUA_SCRIPT_OBSERVERS[eventID].push_back(m_ScriptID);
        Engine::insertion_sort(LUA_SCRIPT_OBSERVERS[eventID]);
    }
}
void Engine::priv::LUAScriptInstance::unregisterEvent(uint32_t eventID) {
    lua_State* L = &Engine::lua::getGlobalState();

    assert(eventID >= 0 && eventID < m_ScriptObservers.size());
    //binary search to see if scriptID is already in the container
    const size_t idx = Engine::binary_search(LUA_SCRIPT_OBSERVERS[eventID], m_ScriptID);
    if (idx != std::numeric_limits<size_t>().max()) {
        auto lastIdx = LUA_SCRIPT_OBSERVERS[eventID].size() - 1;
        if (idx != lastIdx) {
            LUA_SCRIPT_OBSERVERS[eventID][idx] = std::move(LUA_SCRIPT_OBSERVERS[eventID][lastIdx]);
        }
        if (LUA_SCRIPT_OBSERVERS[eventID].size() > 1) {
            LUA_SCRIPT_OBSERVERS[eventID].pop_back();
        } else {
            LUA_SCRIPT_OBSERVERS[eventID].clear();
        }
        Engine::insertion_sort(LUA_SCRIPT_OBSERVERS[eventID]);
    }
}
void Engine::priv::LUAScriptInstance::addOnInitFunction(luabridge::LuaRef initFunction) {
    lua_State* L         = &Engine::lua::getGlobalState();
    auto scriptEntityRef = luabridge::getGlobal(L, ENGINE_LUA_CURRENT_SCRIPT_TOKEN_ENTITY);
    Entity entity        = scriptEntityRef.cast<Entity>();
    initFunction(entity);
}
void Engine::priv::LUAScriptInstance::addOnUpdateFunction(luabridge::LuaRef updateFunction) {
    lua_State* L = &Engine::lua::getGlobalState();
    LUA_SCRIPT_ON_UPDATE_FUNCTIONS.resize(m_ScriptID + 1, std::make_pair(luabridge::LuaRef{ L }, Entity{}));
    LUA_SCRIPT_ON_UPDATE_FUNCTIONS[m_ScriptID] = { updateFunction, m_Entity };
}
void Engine::priv::LUAScriptInstance::addOnEventFunction(luabridge::LuaRef eventFunction) {
    lua_State* L         = &Engine::lua::getGlobalState();
    LUA_SCRIPT_ON_EVENT_FUNCTIONS.resize(m_ScriptID + 1, std::make_pair(luabridge::LuaRef{ L }, Entity{}));
    LUA_SCRIPT_ON_EVENT_FUNCTIONS[m_ScriptID] = { eventFunction, m_Entity };
}

#pragma endregion


#pragma region LUAModule

Engine::priv::LUAModule::LUAModule() {
    LUA_MODULE = this;
}
Engine::priv::LUAModule::~LUAModule() {

}
void Engine::priv::LUAModule::init() {

}
void Engine::priv::LUAModule::cleanupScript(uint32_t scriptID) {
    cleanup_on_update_function(scriptID);
    cleanup_on_event_function(scriptID);
}
void Engine::priv::LUAModule::update(const float dt) {
    for (size_t scriptID = 0; scriptID < LUA_SCRIPT_ON_UPDATE_FUNCTIONS.size(); ++scriptID) {
        auto& luaUpdateFunc = LUA_SCRIPT_ON_UPDATE_FUNCTIONS[scriptID].first;
        Entity entity       = LUA_SCRIPT_ON_UPDATE_FUNCTIONS[scriptID].second;
        if (!luaUpdateFunc.isNil()) {
            ASSERT(luaUpdateFunc.isFunction(), "");
            try {
                luaUpdateFunc(dt, entity);
            } catch (const luabridge::LuaException& exception) {
                ENGINE_PRODUCTION_LOG(__FUNCTION__ << "(): " << exception.what())
            }
        }
    }
}
void Engine::priv::LUAModule::onEvent(const Event& inEvent) {
    if (inEvent.type < LUA_SCRIPT_OBSERVERS.size()) {
        for (const uint32_t scriptID : LUA_SCRIPT_OBSERVERS[inEvent.type]) {
            auto& [function, entity] = LUA_SCRIPT_ON_EVENT_FUNCTIONS[scriptID];
            if (!function.isNil()) {
                try {
                    function(inEvent, entity);
                } catch (const luabridge::LuaException& exception) {
                    ENGINE_PRODUCTION_LOG(__FUNCTION__ << "(): " << exception.what())
                }
            }
        }
    }
}
const Engine::priv::LUABinder& Engine::priv::LUAModule::getBinder() const {
    return m_Binder;
}



const Engine::priv::LUABinder& Engine::priv::getLUABinder() {
    return LUA_MODULE->getBinder();
}
lua_State& Engine::lua::getGlobalState() noexcept {
    return *Engine::priv::getLUABinder().getState()->getState();
}
Engine::priv::LUAScriptInstance Engine::lua::registerScript() {
    //register this script id, entity, and create a lua binded class that containes
    lua_State* L         = &Engine::lua::getGlobalState();
    auto scriptIDRef     = luabridge::getGlobal(L, ENGINE_LUA_CURRENT_SCRIPT_TOKEN_ID);
    auto scriptEntityRef = luabridge::getGlobal(L, ENGINE_LUA_CURRENT_SCRIPT_TOKEN_ENTITY);
    uint32_t scriptID    = scriptIDRef.cast<uint32_t>();
    Entity entity        = scriptEntityRef.cast<Entity>();

    return Engine::priv::LUAScriptInstance{ scriptID, entity };
}

#pragma endregion