#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/lua/LuaScript.h>
#include <core/engine/lua/LuaModule.h>
#include <core/engine/lua/LuaState.h>

LuaScript::LuaScript(const std::string& fileName, bool run) 
    : m_LUAStateRef{ *Engine::priv::getLUABinder().getState() }
    , m_FileName{ fileName }
{
    if (run) {
        runScript();
    }
}
bool LuaScript::runScript() {
    int res = m_LUAStateRef.runFile(m_FileName);
    if (!res) {
        return false; //error
    }
    return true;
}
void LuaScript::clean() {
    int n = lua_gettop(m_LUAStateRef.getState());
    lua_pop(m_LUAStateRef.getState(), n);
}
void LuaScript::callFunction(const char* funcName) {
    lua_getfield(m_LUAStateRef.getState(), LUA_REGISTRYINDEX, m_FileName.c_str());
    lua_getfield(m_LUAStateRef.getState(), -1, funcName);
    lua_call(m_LUAStateRef.getState(), 0, 0);
}
