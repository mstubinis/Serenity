#pragma once
#ifndef ENGINE_FULLSCREEN_ITEMS_H
#define ENGINE_FULLSCREEN_ITEMS_H

#include <memory>
#include "Engine_EventObject.h"

namespace Engine{
    namespace epriv{
        class FullscreenTriangle final: public EventObserver{
            private:
                class impl; std::unique_ptr<impl> m_i;
            public:
                FullscreenTriangle();
                ~FullscreenTriangle();

                void render();
				void onEvent(const Event& e);
        };
        class FullscreenQuad final: public EventObserver{
            private:
                class impl; std::unique_ptr<impl> m_i;
            public:
                FullscreenQuad();
                ~FullscreenQuad();

                void render();
				void onEvent(const Event& e);
        };
    };
};
#endif