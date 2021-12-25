#pragma once
#ifndef ENGINE_LUA_BINDINGS_SOUND_H
#define ENGINE_LUA_BINDINGS_SOUND_H

struct lua_State;

namespace Engine::priv::lua::bindings {
    void createBindingsSounds(lua_State*);
}

namespace Engine::lua {

}

#endif