#include <serenity/lua/LuaModule.h>
#include <serenity/system/Macros.h>
#include <serenity/ecs/entity/Entity.h>

namespace {
    Engine::priv::LUAModule* LUA_MODULE = nullptr;

    std::vector<std::pair<luabridge::LuaRef, Entity>>   LUA_SCRIPT_ON_UPDATE_FUNCTIONS_ENTITY;

    void cleanup_on_update_function(uint32_t scriptID) {
        if (LUA_SCRIPT_ON_UPDATE_FUNCTIONS_ENTITY.size() > scriptID) {
            LUA_SCRIPT_ON_UPDATE_FUNCTIONS_ENTITY[scriptID].first  = luabridge::LuaRef(&Engine::lua::getGlobalState());
            LUA_SCRIPT_ON_UPDATE_FUNCTIONS_ENTITY[scriptID].second = Entity{};
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
    for (auto& [LUAUpdateFunction, entity] : LUA_SCRIPT_ON_UPDATE_FUNCTIONS_ENTITY) {
        if (!LUAUpdateFunction.isNil()) {
            ASSERT(LUAUpdateFunction.isFunction(), "");
            try {
                LUAUpdateFunction(dt, entity);
            }
            catch (...) {
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
void Engine::lua::addOnInitFunction(luabridge::LuaRef initFunction) {
    lua_State* L         = &Engine::lua::getGlobalState();
    auto scriptEntityRef = luabridge::getGlobal(L, ENGINE_LUA_CURRENT_SCRIPT_TOKEN_ENTITY);
    Entity entity        = scriptEntityRef.cast<Entity>();
    initFunction(entity);
}
void Engine::lua::addOnUpdateFunction(luabridge::LuaRef updateFunction) {
    lua_State* L         = &Engine::lua::getGlobalState();
    auto scriptIDRef     = luabridge::getGlobal(L, ENGINE_LUA_CURRENT_SCRIPT_TOKEN_ID);
    auto scriptEntityRef = luabridge::getGlobal(L, ENGINE_LUA_CURRENT_SCRIPT_TOKEN_ENTITY);
    uint32_t scriptID    = scriptIDRef.cast<uint32_t>();
    Entity entity        = scriptEntityRef.cast<Entity>();
    LUA_SCRIPT_ON_UPDATE_FUNCTIONS_ENTITY.resize(scriptID + 1, std::make_pair(luabridge::LuaRef{ L }, Entity{}));
    LUA_SCRIPT_ON_UPDATE_FUNCTIONS_ENTITY[scriptID].first  = updateFunction;
    LUA_SCRIPT_ON_UPDATE_FUNCTIONS_ENTITY[scriptID].second = entity;
}
