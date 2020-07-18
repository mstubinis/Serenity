#include "core/engine/utils/PrecompiledHeader.h"
#include <core/engine/lua/LuaScript.h>
#include <core/engine/lua/LuaModule.h>
#include <core/engine/lua/LuaState.h>

//include these to manually scan the file line by line for whatever reason
//#include <boost/filesystem.hpp>
//#include <boost/iostreams/device/mapped_file.hpp>
//#include <boost/iostreams/stream.hpp>
//#include <boost/lexical_cast.hpp>
//#include <boost/algorithm/string.hpp>

using namespace std;

LuaScript::LuaScript(const string& fileName, bool run) : m_LUAStateRef(*Engine::priv::getLUABinder().getState()){
    m_FileName  = fileName;
    if (run) {
        runScript();
    }
}
LuaScript::~LuaScript() {
    m_FileName = "";
}
bool LuaScript::runScript() {
    const int res = m_LUAStateRef.runFile(m_FileName);
    if (!res) {
        //error
        return false;
    }
    return true;
}
void LuaScript::clean() {
    int n = lua_gettop(m_LUAStateRef.getState());
    lua_pop(m_LUAStateRef.getState(), n);
}
void LuaScript::callFunction(const string& funcName) {
    lua_getfield(m_LUAStateRef.getState(), LUA_REGISTRYINDEX, m_FileName.c_str());
    lua_getfield(m_LUAStateRef.getState(), -1, funcName.c_str());
    lua_call(m_LUAStateRef.getState(), 0, 0);
}
