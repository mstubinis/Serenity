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
        lua_State*   LUA;
        std::string  fileName;
        int          level;
    public:
        LuaScript(const std::string& fileName);
        ~LuaScript();


        void printError(const std::string& variableName, const std::string& reason);
        void clean();

        template<typename T> T get(const std::string& variableName) {
            if (!LUA) {
                printError(variableName, "Script is not loaded");
                return lua_getdefault<T>();
            }
            T result;
            if (lua_gettostack(variableName)) { // variable succesfully on top of stack
                result = lua_get<T>(variableName);
            }else{
                result = lua_getdefault<T>();
            }
            lua_pop(LUA, level + 1); // pop all existing elements from stack
            return result;
        }
        bool lua_gettostack(const std::string& variableName) {
            level = 0;
            std::string var = "";
            for (unsigned int i = 0; i < variableName.size(); i++) {
                if (variableName.at(i) == '.') {
                    if (level == 0) {
                        lua_getglobal(LUA, var.c_str());
                    }else{
                        lua_getfield(LUA, -1, var.c_str());
                    }
                    if (lua_isnil(LUA, -1)) {
                        printError(variableName, var + " is not defined");
                        return false;
                    }else{
                        var = "";
                        level++;
                    }
                }else{
                    var += variableName.at(i);
                }
            }
            if (level == 0) {
                lua_getglobal(LUA, var.c_str());
            }else{
                lua_getfield(LUA, -1, var.c_str());
            }
            if (lua_isnil(LUA, -1)) {
                printError(variableName, var + " is not defined");
                return false;
            }
            return true;
        }
        template<typename T> T lua_get(const std::string& variableName) { return 0; }
        template<typename T> T lua_getdefault() { return 0; }
        template<> inline std::string lua_getdefault<std::string>() { return "null"; }

        template<> inline bool lua_get<bool>(const std::string& variableName) {
            return static_cast<bool>(lua_toboolean(LUA, -1));
        }
        template<> inline float lua_get<float>(const std::string& variableName) {
            if (!lua_isnumber(LUA, -1)) {
                printError(variableName, "Not a number");
            }
            return static_cast<float>(lua_tonumber(LUA, -1));
        }
        template<> inline int lua_get<int>(const std::string& variableName) {
            if (!lua_isnumber(LUA, -1)) {
                printError(variableName, "Not a number");
            }
            return (int)lua_tonumber(LUA, -1);
        }
        template<> inline std::string lua_get<std::string>(const std::string& variableName) {
            std::string s = "null";
            if (lua_isstring(LUA, -1)) {
                s = std::string(lua_tostring(LUA, -1));
            }else{
                printError(variableName, "Not a string");
            }
            return s;
        }


        //arrays
        std::vector<int> getIntVector(const std::string& name);
        std::vector<float> getFloatVector(const std::string& name);
        std::vector<std::string> getStringVector(const std::string& name);

        //table based functions
        std::vector<std::string> getTableKeys(const std::string& name);
};
#endif