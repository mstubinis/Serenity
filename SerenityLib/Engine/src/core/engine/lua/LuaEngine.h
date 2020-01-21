#pragma once
#ifndef ENGINE_LUAENGINE_H_INCLUDE_GUARD
#define ENGINE_LUAENGINE_H_INCLUDE_GUARD

namespace Engine {
    namespace priv {
        class LUAEngine final {
            private:

            public:
                LUAEngine();
                ~LUAEngine();

                void init();
                void cleanup();
        };
    };
};

#endif