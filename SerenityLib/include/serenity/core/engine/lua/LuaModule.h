#pragma once
#ifndef ENGINE_LUA_MODULE_H
#define ENGINE_LUA_MODULE_H

#include <serenity/core/engine/lua/LuaBinder.h>

namespace Engine::priv {
    class LUAModule final {
        private:
            LUABinder m_Binder;
        public:
            LUAModule();
            ~LUAModule();

            void init();

            const LUABinder& getBinder() const;
    };
};
namespace Engine::priv {
    const Engine::priv::LUABinder& getLUABinder();
}

#endif