#pragma once
#ifndef ENGINE_LUA_BINDER_H
#define ENGINE_LUA_BINDER_H

#include <core/engine/lua/LuaState.h>

namespace Engine::priv {
    class LUABinder {
        private:
            std::unique_ptr<LUAState>   m_LUA_STATE;
        public:
            LUABinder();
            virtual ~LUABinder() = default;

            inline CONSTEXPR LUAState* getState() const noexcept { return m_LUA_STATE.get(); }

    };
}

#endif