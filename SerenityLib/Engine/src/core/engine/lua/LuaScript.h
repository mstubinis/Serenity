#pragma once
#ifndef ENGINE_LUASCRIPT_H
#define ENGINE_LUASCRIPT_H

class LUAState;

#include <core/engine/lua/LuaIncludes.h>
#include <core/engine/lua/LuaState.h>

class LuaScript final {
    private:
        LUAState&      m_LUAStateRef;
        std::string    m_FileName  = "";
        LuaScript() = delete;
    public:
        LuaScript(const std::string& fileName, bool run = true);
        ~LuaScript();

        bool runScript();

        void callFunction(const std::string& name);
        void clean();

};
#endif