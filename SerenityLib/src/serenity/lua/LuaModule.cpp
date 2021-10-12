#include <serenity/lua/LuaModule.h>
#include <serenity/system/Macros.h>

namespace {
    Engine::priv::LUAModule* LUA_MODULE = nullptr;

    std::vector<luabridge::LuaRef> LUA_SCRIPT_ON_UPDATE_FUNCTIONS;

    void cleanup_on_update_function(uint32_t scriptID) {
        if (LUA_SCRIPT_ON_UPDATE_FUNCTIONS.size() > scriptID) {
            LUA_SCRIPT_ON_UPDATE_FUNCTIONS[scriptID] = luabridge::LuaRef(Engine::priv::getLUABinder().getState()->getState());
        }
    }
}

Engine::priv::LUAModule::LUAModule() {
    LUA_MODULE = this;
}
Engine::priv::LUAModule::~LUAModule() {

}

void Engine::priv::LUAModule::init() {

}
void Engine::priv::LUAModule::cleanupScript(uint32_t scriptID) {
    cleanup_on_update_function(scriptID);
}
void Engine::priv::LUAModule::update(const float dt) {
    for (auto& itr : LUA_SCRIPT_ON_UPDATE_FUNCTIONS) {
        if (!itr.isNil()) {
            ASSERT(itr.isFunction(), "");
            itr(dt);
        }
    }
}
const Engine::priv::LUABinder& Engine::priv::LUAModule::getBinder() const {
    return m_Binder;
}





const Engine::priv::LUABinder& Engine::priv::getLUABinder() {
    return LUA_MODULE->getBinder();
}
void Engine::lua::addOnUpdateFunction(luabridge::LuaRef updateFunction) {
    lua_State* L       = Engine::priv::getLUABinder().getState()->getState();
    auto scriptIDRef   = luabridge::getGlobal(L, ENGINE_LUA_CURRENT_SCRIPT_TOKEN);
    uint32_t scriptID  = scriptIDRef.cast<uint32_t>();
    LUA_SCRIPT_ON_UPDATE_FUNCTIONS.resize(scriptID + 1, luabridge::LuaRef{L});
    LUA_SCRIPT_ON_UPDATE_FUNCTIONS[scriptID] = updateFunction;
}
