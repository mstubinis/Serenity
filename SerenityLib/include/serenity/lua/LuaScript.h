#pragma once
#ifndef ENGINE_LUASCRIPT_H
#define ENGINE_LUASCRIPT_H

class LUAState;

#include <serenity/lua/LuaIncludes.h>
#include <serenity/lua/LuaState.h>

class LuaScript final {
    private:
        LUAState&     m_LUAStateRef;
        std::string   m_FileName;

        LuaScript() = delete;
    public:
        LuaScript(const std::string& fileName, bool run = true);

        bool runScript();

        void callFunction(const char* name);
        void clean();

};
#endif