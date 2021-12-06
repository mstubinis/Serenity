#include <serenity/lua/LuaState.h>
#include <serenity/lua/LuaIncludes.h>
#include <serenity/system/Macros.h>
#include <serenity/ecs/entity/Entity.h>

LUAState::LUAState() 
    : L{ luaL_newstate() }
{
    luaL_openlibs(L);
}
LUAState::~LUAState() {
    if (L) {
        lua_close(L);
    }
}
int LUAState::runFile(const std::string& filename, uint32_t scriptID, void* entity) const noexcept {
    int ret = -1;
    auto* filenameC = filename.c_str();
    luabridge::setGlobal(L, scriptID, ENGINE_LUA_CURRENT_SCRIPT_TOKEN_ID);
    luabridge::setGlobal(L, entity ? *static_cast<Entity*>(entity) : Entity{}, ENGINE_LUA_CURRENT_SCRIPT_TOKEN_ENTITY);
    bool failedToLoaded = luaL_dofile(L, filenameC);
    luabridge::setGlobal(L, -1, ENGINE_LUA_CURRENT_SCRIPT_TOKEN_ID);
    luabridge::setGlobal(L, -1, ENGINE_LUA_CURRENT_SCRIPT_TOKEN_ENTITY);
    if (failedToLoaded) {
        ENGINE_PRODUCTION_LOG("LUA Error: script did not compile correctly (" << filename << ")")
        return 0;
    }
    ret = lua_getfield(L, LUA_REGISTRYINDEX, filenameC);
    if (ret) { //already loaded once
        ret = lua_getglobal(L, "_G"); //Get the global table
        lua_setfield(L, -2, "__index");
        ret = lua_setmetatable(L, -2);
        ret = lua_getfield(L, LUA_REGISTRYINDEX, filenameC); 
        lua_setupvalue(L, 1, 1);
        ret = lua_pcall(L, 0, LUA_MULTRET, 0);
        return 0;
    }
    lua_newtable(L);
    lua_newtable(L);
    ret = lua_getglobal(L, "_G");
    lua_setfield(L, -2, "__index");
    ret = lua_setmetatable(L, -2);
    lua_setfield(L, LUA_REGISTRYINDEX, filenameC);
    ret = lua_getfield(L, LUA_REGISTRYINDEX, filenameC);
    lua_pop(L, 1);
    ret = lua_pcall(L, 0, LUA_MULTRET, 0);
    return 1;
}
int LUAState::runCodeContent(const std::string& inData, uint32_t scriptID, void* entity) const noexcept {
    int ret = -1;
    auto* data = inData.c_str();
    luabridge::setGlobal(L, scriptID, ENGINE_LUA_CURRENT_SCRIPT_TOKEN_ID);
    luabridge::setGlobal(L, entity ? *static_cast<Entity*>(entity) : Entity{}, ENGINE_LUA_CURRENT_SCRIPT_TOKEN_ENTITY);
    std::string scriptName = std::string{ "Script: " } + std::to_string(scriptID);
    bool failedToLoaded = luaL_loadbuffer(L, data, inData.size(), scriptName.c_str() ) || lua_pcall(L, 0, 0, 0);
    luabridge::setGlobal(L, -1, ENGINE_LUA_CURRENT_SCRIPT_TOKEN_ID);
    luabridge::setGlobal(L, -1, ENGINE_LUA_CURRENT_SCRIPT_TOKEN_ENTITY);
    if (failedToLoaded) {
        ENGINE_PRODUCTION_LOG("LUA Error: script did not compile correctly (" << scriptName << ")")
        return 0;
    }
    ret = lua_getfield(L, LUA_REGISTRYINDEX, scriptName.c_str());
    if (ret) { //already loaded once
        ret = lua_getglobal(L, "_G"); //Get the global table
        lua_setfield(L, -2, "__index");
        ret = lua_setmetatable(L, -2);
        ret = lua_getfield(L, LUA_REGISTRYINDEX, scriptName.c_str());
        lua_setupvalue(L, 1, 1);
        ret = lua_pcall(L, 0, LUA_MULTRET, 0);
        return 0;
    }
    lua_newtable(L);
    lua_newtable(L);
    ret = lua_getglobal(L, "_G");
    lua_setfield(L, -2, "__index");
    ret = lua_setmetatable(L, -2);
    lua_setfield(L, LUA_REGISTRYINDEX, scriptName.c_str());
    ret = lua_getfield(L, LUA_REGISTRYINDEX, scriptName.c_str());
    lua_pop(L, 1);
    ret = lua_pcall(L, 0, LUA_MULTRET, 0);
    return 1;
}