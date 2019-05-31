#include <core/engine/lua/LuaScript.h>
#include <core/engine/Engine_Utils.h>

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
    level = 0;
    fileName = _fileName;
    LUA = luaL_newstate();
    if (luaL_loadfile(LUA, fileName.c_str()) || lua_pcall(LUA, 0, 0, 0)) {
        cout << "LUA Error: script not loaded (" << fileName << ")" << endl;
    }
    luaL_openlibs(LUA);


    //scan the file line by line for anything?
    //boost::iostreams::stream<boost::iostreams::mapped_file_source> str(fileName);
    //for (string line; getline(str, line, '\n');) {
    //    line.erase(std::remove(line.begin(), line.end(), '\r'), line.end()); //remove \r from the line
    //}
}
LuaScript::~LuaScript() {
    if (LUA) {
        fileName = "";
        level = 0;
        lua_close(LUA);
    }

}
void LuaScript::clean() {
    int n = lua_gettop(LUA);
    lua_pop(LUA, n);
}
void LuaScript::printError(const string& variableName, const string& reason) {
    cout << "LUA Error: can't get variable name: [" << variableName << "]. " << reason << endl;
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

*/


vector<int> LuaScript::getIntVector(const string& name) {
    vector<int> v;
    lua_getglobal(LUA, name.c_str());
    if (lua_isnil(LUA, -1)) {
        return v;
    }
    lua_pushnil(LUA);
    while (lua_next(LUA, -2)) {
        v.push_back((int)lua_tonumber(LUA, -1));
        lua_pop(LUA, 1);
    }
    clean();
    return v;
}
vector<float> LuaScript::getFloatVector(const string& name) {
    vector<float> v;
    lua_getglobal(LUA, name.c_str());
    if (lua_isnil(LUA, -1)) {
        return v;
    }
    lua_pushnil(LUA);
    while (lua_next(LUA, -2)) {
        v.push_back((float)lua_tonumber(LUA, -1));
        lua_pop(LUA, 1);
    }
    clean();
    return v;
}
vector<string> LuaScript::getStringVector(const string& name) {
    vector<string> v;
    lua_getglobal(LUA, name.c_str());
    if (lua_isnil(LUA, -1)) {
        return v;
    }
    lua_pushnil(LUA);
    while (lua_next(LUA, -2)) {
        v.push_back((string)lua_tostring(LUA, -1));
        lua_pop(LUA, 1);
    }
    clean();
    return v;
}
vector<string> LuaScript::getTableKeys(const string& name) {
    luaL_loadstring(LUA, getKeysCode.c_str()); // execute code
    lua_pcall(LUA, 0, 0, 0);
    lua_getglobal(LUA, "getKeys");      // get function
    lua_pushstring(LUA, name.c_str());
    lua_pcall(LUA, 1, 1, 0);            // execute function

    string test = lua_tostring(LUA, -1);
    vector<string> strings;
    string temp = "";
    for (unsigned int i = 0; i < test.size(); i++) {
        if (test.at(i) != ',') {
            temp += test[i];
        }else{
            strings.push_back(temp);
            temp = "";
        }
    }
    clean();
    return strings;
}