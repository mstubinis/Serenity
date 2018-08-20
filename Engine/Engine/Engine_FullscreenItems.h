#pragma once
#ifndef ENGINE_FULLSCREEN_ITEMS_H
#define ENGINE_FULLSCREEN_ITEMS_H

#include <memory>
#include "Engine_Utils.h"

namespace Engine{
    namespace epriv{
        class FullscreenTriangle final: private Engine::epriv::noncopyable{
            private:
                class impl; std::unique_ptr<impl> m_i;
            public:
                FullscreenTriangle();
                ~FullscreenTriangle();

                void render();
        };
        class FullscreenQuad final: private Engine::epriv::noncopyable{
            private:
                class impl; std::unique_ptr<impl> m_i;
            public:
                FullscreenQuad();
                ~FullscreenQuad();

                void render();
        };
    };
};
#endif