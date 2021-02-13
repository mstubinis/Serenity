#pragma once
#ifndef ENGINE_LUA_MODULE_H
#define ENGINE_LUA_MODULE_H

#include <serenity/lua/LuaBinder.h>

namespace Engine::priv {
    class LUAModule final {
        private:
            LUABinder m_Binder;
        public:
            LUAModule();
            ~LUAModule();

            void init();

            [[nodiscard]] const LUABinder& getBinder() const;
    };
};
namespace Engine::priv {
    [[nodiscard]] const Engine::priv::LUABinder& getLUABinder();
}

#endif