#pragma once
#ifndef ENGINE_LUA_BINDER_H
#define ENGINE_LUA_BINDER_H

#include <serenity/core/engine/lua/LuaState.h>
#include <memory>

namespace Engine::priv {
    class LUABinder {
        private:
            std::unique_ptr<LUAState>   m_LUA_STATE;
        public:
            LUABinder();
            virtual ~LUABinder() = default;

            inline LUAState* getState() const noexcept { return m_LUA_STATE.get(); }

    };
}

#endif