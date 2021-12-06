#pragma once
#ifndef ENGINE_LUA_INCLUDES_H
#define ENGINE_LUA_INCLUDES_H

#ifdef __cplusplus
    #include <lua.h>
    #include <lualib.h>
    #include <lauxlib.h>
#else
    extern "C" {
        #include "lua.h"
        #include "lauxlib.h"
        #include "lualib.h"
    }
#endif
#include <LuaBridge/LuaBridge.h>
#include <LuaBridge/List.h>
#include <LuaBridge/Map.h>
#include <LuaBridge/UnorderedMap.h>
#include <LuaBridge/Vector.h>

constexpr auto ENGINE_LUA_CURRENT_SCRIPT_TOKEN_ID     = "ScrNum";
constexpr auto ENGINE_LUA_CURRENT_SCRIPT_TOKEN_ENTITY = "ScrEnt";

#endif