#pragma once
#ifndef ENGINE_EDITOR_WINDOW_SCENE_H
#define ENGINE_EDITOR_WINDOW_SCENE_H

class  Scene;

#include <vector>

namespace Engine::priv {
    class EditorWindowScene final {
        private:
            int  m_Tab = 0;

            void internal_render_entities(Scene& currentScene);
            void internal_render_profiler();
            void internal_render_renderer();
            void internal_render_network();
        public:
            EditorWindowScene() = default;

            void update();
    };
}

#endif