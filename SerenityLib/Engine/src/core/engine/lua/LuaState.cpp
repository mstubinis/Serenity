#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/lua/LuaState.h>
#include <core/engine/lua/LuaIncludes.h>

using namespace std;

LUAState::LUAState() {
    L = luaL_newstate();
    luaL_openlibs(L);
}
LUAState::~LUAState() {
    if (L) {
        lua_close(L);
    }
}
lua_State* LUAState::getState() const {
    return L;
}
int LUAState::runFile(const string& filename) const {
    int ret = -1;
    auto* filenameC = filename.c_str();
    if (luaL_loadfile(L, filenameC) || lua_pcall(L, 0, 0, 0)) {
        cout << "LUA Error: script did not compile correctly (" << filename << ")\n";
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
