#pragma once
#ifndef ENGINE_LUASCRIPT_H
#define ENGINE_LUASCRIPT_H

class LUAState;

#include <serenity/lua/LuaIncludes.h>
#include <serenity/lua/LuaState.h>

class LuaScript final {
    private:
        LUAState&     m_L;
        std::string   m_FileName;
        size_t        m_ID        = std::numeric_limits<size_t>().max();

        LuaScript() = delete;
    public:
        LuaScript(LUAState&, const std::string& fileName, bool run = true);
        LuaScript(const std::string& fileName, bool run = true);
        ~LuaScript();

        bool runScript() noexcept;

        void callFunction(const char* name) noexcept;
        void clean() noexcept;

};
#endif