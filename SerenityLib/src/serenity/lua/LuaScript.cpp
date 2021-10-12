#include <serenity/lua/LuaScript.h>
#include <serenity/lua/LuaModule.h>
#include <serenity/lua/LuaState.h>
#include <serenity/system/Engine.h>

namespace {
    std::atomic<size_t> ID_COUNTER = 0;
}

LuaScript::LuaScript(LUAState& state, const std::string& fileName, bool run)
    : m_L{ state }
    , m_FileName{ fileName }
    , m_ID { ID_COUNTER++ }
{
    if (run) {
        runScript();
    }
}
LuaScript::LuaScript(const std::string& fileName, bool run) 
    : LuaScript{ *Engine::priv::getLUABinder().getState(), fileName, run }
{}
LuaScript::~LuaScript() {
    clean();
}
bool LuaScript::runScript() noexcept {
    int res;
    res = m_L.runFile(m_FileName, m_ID);
    return res;
}
void LuaScript::clean() noexcept {
    int n = lua_gettop(m_L);
    lua_pop(m_L, n);

    Engine::priv::Core::m_Engine->m_LUAModule.cleanupScript(m_ID);
    Engine::priv::Core::m_Engine->m_EventModule.m_EventDispatcher.cleanupScript(m_ID);

    lua_gc(m_L, LUA_GCCOLLECT, 0);
}
void LuaScript::callFunction(const char* funcName) noexcept {
    lua_getfield(m_L, LUA_REGISTRYINDEX, m_FileName.c_str());
    lua_getfield(m_L, -1, funcName);
    lua_call(m_L, 0, 0);
}
