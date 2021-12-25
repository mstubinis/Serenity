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

        [[nodiscard]] inline lua_State* getState() const noexcept { return L; }

        [[nodiscard]] inline operator lua_State*() { return L; }

        int runFile(const std::string& filename, uint32_t scriptID, void* entity = nullptr) const noexcept;
        int runCodeContent(std::string& data, uint32_t scriptID, void* entity = nullptr) const noexcept;
};

#endif