#pragma once
#ifndef ENGINE_LUA_STATE_H
#define ENGINE_LUA_STATE_H

struct lua_State;

#include <string>

class LUAState {
    private:
        lua_State* L = nullptr;
    public:
        LUAState();
        virtual ~LUAState();

        inline lua_State* getState() const noexcept { return L; }

        int runFile(const std::string& filename) const;
};

#endif