#pragma once
#ifndef ENGINE_LUA_MODULE_H
#define ENGINE_LUA_MODULE_H

#include <serenity/lua/LuaBinder.h>
#include <serenity/lua/LuaIncludes.h>
#include <serenity/ecs/entity/Entity.h>

struct Event;

namespace Engine::priv {
    class LUAScriptInstance {
        private:
            uint32_t  m_ScriptID = std::numeric_limits<uint32_t>().max();
            Entity    m_Entity;
        public:
            LUAScriptInstance() = default;
            LUAScriptInstance(uint32_t scriptID, Entity entity)
                : m_ScriptID{ scriptID }
                , m_Entity{ entity }
            {}

            void addOnInitFunction(luabridge::LuaRef initFunction);
            void addOnUpdateFunction(luabridge::LuaRef updateFunction);
            void addOnEventFunction(luabridge::LuaRef eventFunction);
            void registerEvent(uint32_t eventID);
            void unregisterEvent(uint32_t eventID);
    };
    class LUAModule final {
        private:
            LUABinder m_Binder;
        public:
            LUAModule();
            ~LUAModule();

            void cleanupScript(uint32_t scriptID);

            void init();
            void update(const float dt);
            void onEvent(const Event&);

            [[nodiscard]] const LUABinder& getBinder() const;
    };
};
namespace Engine::priv {
    [[nodiscard]] const Engine::priv::LUABinder& getLUABinder();
}

namespace Engine::lua {
    lua_State& getGlobalState() noexcept;

    Engine::priv::LUAScriptInstance registerScript();
}

#endif