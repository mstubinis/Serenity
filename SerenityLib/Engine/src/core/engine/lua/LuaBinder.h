#pragma once
#ifndef ENGINE_LUA_BINDER_H
#define ENGINE_LUA_BINDER_H

class LUAState;

namespace Engine::priv {
    class LUABinder {
        private:
            LUAState*   m_LUA_STATE = nullptr;
        public:
            LUABinder();
            virtual ~LUABinder();

            LUAState* getState() const;
    };
}

#endif