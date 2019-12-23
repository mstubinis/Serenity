#include <core/engine/lua/LuaScript.h>
#include <core/engine/utils/Utils.h>

//include these to manually scan the file line by line for whatever reason
//#include <boost/filesystem.hpp>
//#include <boost/iostreams/device/mapped_file.hpp>
//#include <boost/iostreams/stream.hpp>
//#include <boost/lexical_cast.hpp>
//#include <boost/algorithm/string.hpp>

using namespace std;

const string& getKeysCode =
"function getKeys(name) \n"
"    local s = \"\"\n"
"    local k \n"
"    local v \n"
"    for k, v in pairs(_G[name]) do \n"
"        s = s..k..\",\" \n"
"    end \n"
"    return s \n"
"end";

LuaScript::LuaScript(const string& _fileName) {
    level      = 0;
    fileName   = _fileName;
    LUA_STATE  = luaL_newstate();
    if (luaL_loadfile(LUA_STATE, fileName.c_str()) || lua_pcall(LUA_STATE, 0, 0, 0)) {
        cout << "LUA Error: script not loaded (" << fileName << ")" << endl;
    }
    luaL_openlibs(LUA_STATE);
}
LuaScript::~LuaScript() {
    if (LUA_STATE) {
        fileName  = "";
        level     = 0;
        lua_close(LUA_STATE);
    }

}
void LuaScript::clean() {
    int n = lua_gettop(LUA_STATE);
    lua_pop(LUA_STATE, n);
}
void LuaScript::printError(const string& variableName, const string& reason) {
    std::cout << "LUA Error: can't get variable name: [" << variableName << "]. " << reason << std::endl;
}


/*
example of binding a lua function to the c++ equivalent

function sum(x, y)
    return x + y
end


------------------
     LUA STACK
------------------
         y                - 0
------------------ 
         x                - 1
------------------
        sum               - 2
------------------


int sum(int x, int y) {
    lua_State* LUA = luaL_newstate();
    if (luaL_loadfile(LUA, "sum.lua") || lua_pcall(LUA, 0, 0, 0)) {
        std::cout<<"Error: failed to load sum.lua"<<std::endl;
        return 0;
    }
    lua_getglobal(LUA, "sum");
    lua_pushnumber(LUA, x);
    lua_pushnumber(LUA, y);

    lua_pcall(LUA, 2, 1, 0);

    int result = (int)lua_tonumber(LUA, -1);
    lua_pop(LUA, 1);
    return result;
}

Any function registered with Lua must have this same prototype, defined as lua_CFunction in lua.h:
typedef int (*lua_CFunction) (lua_State *L);


*/


const bool LuaScript::lua_gettostack(const string& variableName) {
    level        = 0;
    string var   = "";
    for (size_t i = 0; i < variableName.size(); i++) {
        if (variableName.at(i) == '.') {
            if (level == 0) {
                lua_getglobal(LUA_STATE, var.c_str());
            }else{
                lua_getfield(LUA_STATE, -1, var.c_str());
            }
            if (lua_isnil(LUA_STATE, -1)) {
                LuaScript::printError(variableName, var + " is not defined");
                return false;
            }else{
                var = "";
                level++;
            }
        }else{
            var += variableName.at(i);
        }
    }
    if (level == 0) {
        lua_getglobal(LUA_STATE, var.c_str());
    }else{
        lua_getfield(LUA_STATE, -1, var.c_str());
    }
    if (lua_isnil(LUA_STATE, -1)) {
        LuaScript::printError(variableName, var + " is not defined");
        return false;
    }
    return true;
}


vector<int> LuaScript::getIntVector(const string& name) {
    vector<int> v;
    lua_getglobal(LUA_STATE, name.c_str());
    if (lua_isnil(LUA_STATE, -1)) {
        return v;
    }
    lua_pushnil(LUA_STATE);
    while (lua_next(LUA_STATE, -2)) {
        const auto int_val = static_cast<int>(lua_tonumber(LUA_STATE, -1));
        v.push_back(int_val);
        lua_pop(LUA_STATE, 1);
    }
    LuaScript::clean();
    return v;
}
vector<float> LuaScript::getFloatVector(const string& name) {
    vector<float> v;
    lua_getglobal(LUA_STATE, name.c_str());
    if (lua_isnil(LUA_STATE, -1)) {
        return v;
    }
    lua_pushnil(LUA_STATE);
    while (lua_next(LUA_STATE, -2)) {
        const auto float_val = static_cast<float>(lua_tonumber(LUA_STATE, -1));
        v.push_back(float_val);
        lua_pop(LUA_STATE, 1);
    }
    LuaScript::clean();
    return v;
}
vector<string> LuaScript::getStringVector(const string& name) {
    vector<string> v;
    lua_getglobal(LUA_STATE, name.c_str());
    if (lua_isnil(LUA_STATE, -1)) {
        return v;
    }
    lua_pushnil(LUA_STATE);
    while (lua_next(LUA_STATE, -2)) {
        const auto string_val = static_cast<string>(lua_tostring(LUA_STATE, -1));
        v.push_back(string_val);
        lua_pop(LUA_STATE, 1);
    }
    LuaScript::clean();
    return v;
}
vector<string> LuaScript::getTableKeys(const string& name) {
    vector<string> strings;

    luaL_loadstring(LUA_STATE, getKeysCode.c_str()); // execute code
    lua_pcall(LUA_STATE, 0, 0, 0);
    lua_getglobal(LUA_STATE, "getKeys");      // get function
    lua_pushstring(LUA_STATE, name.c_str());
    lua_pcall(LUA_STATE, 1, 1, 0);            // execute function

    string test = lua_tostring(LUA_STATE, -1);
    string temp = "";
    for (unsigned int i = 0; i < test.size(); i++) {
        if (test.at(i) != ',') {
            temp += test[i];
        }else{
            strings.push_back(temp);
            temp = "";
        }
    }
    LuaScript::clean();
    return strings;
}