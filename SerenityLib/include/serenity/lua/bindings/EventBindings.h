#pragma once
#ifndef ENGINE_LUA_BINDINGS_EVENT_H
#define ENGINE_LUA_BINDINGS_EVENT_H

struct lua_State;

namespace Engine::priv::lua::bindings {
    void createBindingsEvents(lua_State*);
}

namespace Engine::lua {

}

#endif