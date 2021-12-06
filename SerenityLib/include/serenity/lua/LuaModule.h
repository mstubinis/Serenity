#pragma once
#ifndef ENGINE_LUA_MODULE_H
#define ENGINE_LUA_MODULE_H

#include <serenity/lua/LuaBinder.h>
#include <serenity/lua/LuaIncludes.h>

namespace Engine::priv {
    class LUAModule final {
        private:
            LUABinder m_Binder;
        public:
            LUAModule();
            ~LUAModule();

            void cleanupScript(uint32_t scriptID);

            void init();
            void update(const float dt);

            [[nodiscard]] const LUABinder& getBinder() const;
    };
};
namespace Engine::priv {
    [[nodiscard]] const Engine::priv::LUABinder& getLUABinder();
}

namespace Engine::lua {
    lua_State& getGlobalState() noexcept;

    void addOnInitFunction(luabridge::LuaRef initFunction);
    void addOnUpdateFunction(luabridge::LuaRef updateFunction);
    void addOnEventFunction(luabridge::LuaRef eventFunction);
}

#endif