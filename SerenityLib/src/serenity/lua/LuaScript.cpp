#include <serenity/lua/LuaScript.h>
#include <serenity/lua/LuaModule.h>
#include <serenity/lua/LuaState.h>

LuaScript::LuaScript(const std::string& fileName, bool run) 
    : m_LUAStateRef{ *Engine::priv::getLUABinder().getState() }
    , m_FileName{ fileName }
{
    if (run) {
        runScript();
    }
}
LuaScript::~LuaScript() {
    clean();
}
bool LuaScript::runScript() noexcept {
    int res;
    res = m_LUAStateRef.runFile(m_FileName);
    return res;
}
void LuaScript::clean() noexcept {
    int n = lua_gettop(m_LUAStateRef.getState());
    lua_pop(m_LUAStateRef.getState(), n);
}
void LuaScript::callFunction(const char* funcName) noexcept {
    lua_getfield(m_LUAStateRef.getState(), LUA_REGISTRYINDEX, m_FileName.c_str());
    lua_getfield(m_LUAStateRef.getState(), -1, funcName);
    lua_call(m_LUAStateRef.getState(), 0, 0);
}
