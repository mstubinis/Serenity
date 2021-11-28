#pragma once
#ifndef ENGINE_LUASCRIPT_H
#define ENGINE_LUASCRIPT_H

class LUAState;

#include <serenity/lua/LuaIncludes.h>
#include <serenity/lua/LuaState.h>

class LuaScript final {
    private:
        std::string   m_FileName;
        LUAState*     m_L           = nullptr;
        uint32_t      m_ID          = std::numeric_limits<uint32_t>().max();
        bool          m_Executed    = false;
    public:
        LuaScript();
        LuaScript(LUAState&, const std::string& fileName, bool run = true);
        LuaScript(const std::string& fileName, bool run = true);

        LuaScript(const LuaScript&)                 = delete;
        LuaScript& operator=(const LuaScript&)      = delete;
        LuaScript(LuaScript&&) noexcept;
        LuaScript& operator=(LuaScript&&) noexcept;
        ~LuaScript();

        bool runScript() noexcept;

        void callFunction(const char* name) noexcept;
        void clean() noexcept;

};
#endif