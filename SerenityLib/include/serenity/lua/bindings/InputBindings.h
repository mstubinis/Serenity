#pragma once
#ifndef ENGINE_LUA_BINDINGS_INPUT_H
#define ENGINE_LUA_BINDINGS_INPUT_H

struct lua_State;

namespace Engine::priv::lua::bindings {
    void createBindingsInput(lua_State*);
}

namespace Engine::lua {

}

#endif