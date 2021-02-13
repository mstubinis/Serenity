#include <serenity/lua/LuaState.h>
#include <serenity/lua/LuaIncludes.h>
#include <serenity/system/Macros.h>

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
int LUAState::runFile(const std::string& filename) const {
    int ret = -1;
    auto* filenameC = filename.c_str();
    if (luaL_loadfile(L, filenameC) || lua_pcall(L, 0, 0, 0)) {
        ENGINE_PRODUCTION_LOG("LUA Error: script did not compile correctly (" << filename)
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
    lua_setupvalue(L, 1, 1);
    ret = lua_pcall(L, 0, LUA_MULTRET, 0);
    return 1;
}
