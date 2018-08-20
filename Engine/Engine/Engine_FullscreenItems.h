#pragma once
#ifndef ENGINE_FULLSCREEN_ITEMS_H
#define ENGINE_FULLSCREEN_ITEMS_H

#include <memory>

namespace Engine{
    namespace epriv{
        class FullscreenTriangle final{
            private:
                class impl; std::unique_ptr<impl> m_i;
                FullscreenTriangle(const FullscreenTriangle&); // non construction-copyable
                FullscreenTriangle& operator=(const FullscreenTriangle&); // non copyable
            public:
                FullscreenTriangle();
                ~FullscreenTriangle();

                void render();
        };
        class FullscreenQuad final{
            private:
                class impl; std::unique_ptr<impl> m_i;
                FullscreenQuad(const FullscreenQuad&); // non construction-copyable
                FullscreenQuad& operator=(const FullscreenQuad&); // non copyable
            public:
                FullscreenQuad();
                ~FullscreenQuad();

                void render();
        };
    };
};
#endif