#include <serenity/lua/LuaScript.h>
#include <serenity/lua/LuaModule.h>
#include <serenity/lua/LuaState.h>
#include <serenity/system/Engine.h>

namespace {
    std::atomic<uint32_t> ID_COUNTER = 0;
}

LuaScript::LuaScript()
    : m_L{ Engine::priv::getLUABinder().getState() }
    , m_ID{ ID_COUNTER++ }
{
}
LuaScript::LuaScript(LUAState& state, const std::string& fileName, bool run)
    : m_FileName{ fileName }
    , m_L{ &state }
    , m_ID { ID_COUNTER++ }
    , m_Executed { run }
{
    if (run) {
        runScript();
    }
}
LuaScript::LuaScript(const std::string& fileName, bool run) 
    : LuaScript{ *Engine::priv::getLUABinder().getState(), fileName, run }
{}
LuaScript::LuaScript(LuaScript&& other) noexcept {
    m_FileName = std::move(other.m_FileName);
    m_L        = std::exchange(other.m_L, nullptr);
    m_ID       = std::exchange(other.m_ID, std::numeric_limits<uint32_t>().max());
    m_Executed = std::exchange(other.m_Executed, false);
}
LuaScript& LuaScript::operator=(LuaScript&& other) noexcept {
    if (this != &other) {
        m_FileName = std::move(other.m_FileName);
        m_L        = std::exchange(other.m_L, nullptr);
        m_ID       = std::exchange(other.m_ID, std::numeric_limits<uint32_t>().max());
        m_Executed = std::exchange(other.m_Executed, false);
    }
    return *this;
}

LuaScript::~LuaScript() {
    clean();
}
bool LuaScript::runScript() noexcept {
    assert(m_FileName.size() > 0);
    assert(m_L != nullptr);
    assert(m_ID != std::numeric_limits<uint32_t>().max());
    assert(m_Executed == false);

    int res = m_L->runFile(m_FileName, m_ID);
    m_Executed = static_cast<bool>(res);
    return res;
}
void LuaScript::clean() noexcept {
    assert(m_L != nullptr);
    assert(m_ID != std::numeric_limits<uint32_t>().max());

    if (m_Executed) {
        int n = lua_gettop(*m_L);
        lua_pop(*m_L, n);
        Engine::priv::Core::m_Engine->m_LUAModule.cleanupScript(m_ID);
        Engine::priv::Core::m_Engine->m_EventModule.m_EventDispatcher.cleanupScript(m_ID);
        lua_gc(*m_L, LUA_GCCOLLECT, 0);
    }
}
void LuaScript::callFunction(const char* funcName) noexcept {
    assert(m_L != nullptr);
    assert(m_ID != std::numeric_limits<uint32_t>().max());
    assert(m_Executed == true);

    lua_getfield(*m_L, LUA_REGISTRYINDEX, m_FileName.c_str());
    lua_getfield(*m_L, -1, funcName);
    lua_call(*m_L, 0, 0);
}
