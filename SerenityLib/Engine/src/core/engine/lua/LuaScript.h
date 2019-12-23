#pragma once
#ifndef ENGINE_LUASCRIPT_H_INCLUDE_GUARD
#define ENGINE_LUASCRIPT_H_INCLUDE_GUARD

#include <string>
#include <vector>
#include <iostream>

#ifdef __cplusplus
#include <lua.hpp>
#else
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#endif

class LuaScript final {
    private:
        lua_State*     LUA_STATE;
        std::string    fileName;
        int            level;
    public:
        LuaScript(const std::string& fileName);
        ~LuaScript();


        void printError(const std::string& variableName, const std::string& reason);
        void clean();

        template<typename T> T get(const std::string& variableName) {
            if (!LUA_STATE) {
                LuaScript::printError(variableName, "Script is not loaded");
                return lua_getdefault<T>();
            }
            T result;
            if (lua_gettostack(variableName)) { // variable succesfully on top of stack
                result = LuaScript::lua_get<T>(variableName);
            }else{
                result = LuaScript::lua_getdefault<T>();
            }
            lua_pop(LUA_STATE, level + 1); // pop all existing elements from stack
            return result;
        }
        const bool lua_gettostack(const std::string& variableName);
        template<typename T> T lua_get(const std::string& variableName) { 
            return 0; 
        }
        template<typename T> T lua_getdefault() { 
            return 0; 
        }

        //bools
        template<> inline bool lua_get<bool>(const std::string& variableName) {
            return static_cast<bool>(lua_toboolean(LUA_STATE, -1));
        }

        //strings
        template<> inline std::string lua_get<std::string>(const std::string& variableName) {
            std::string s = "null";
            if (lua_isstring(LUA_STATE, -1)) {
                s = std::string(lua_tostring(LUA_STATE, -1));
            }
            else {
                LuaScript::printError(variableName, "Not a string");
            }
            return s;
        }
        template<> inline std::string lua_getdefault<std::string>() {
            return "null";
        }

        //floats
        template<> inline float lua_get<float>(const std::string& variableName) {
            if (!lua_isnumber(LUA_STATE, -1)) {
                LuaScript::printError(variableName, "Not a number");
            }
            return static_cast<float>(lua_tonumber(LUA_STATE, -1));
        }
        //ints
        template<> inline int lua_get<int>(const std::string& variableName) {
            if (!lua_isnumber(LUA_STATE, -1)) {
                LuaScript::printError(variableName, "Not a number");
            }
            return (int)lua_tonumber(LUA_STATE, -1);
        }




        //arrays
        std::vector<int>           getIntVector(const std::string& name);
        std::vector<float>         getFloatVector(const std::string& name);
        std::vector<std::string>   getStringVector(const std::string& name);

        //table based functions
        std::vector<std::string>   getTableKeys(const std::string& name);
};
#endif