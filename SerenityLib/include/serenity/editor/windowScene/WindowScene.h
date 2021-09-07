#pragma once
#ifndef ENGINE_EDITOR_WINDOW_SCENE_H
#define ENGINE_EDITOR_WINDOW_SCENE_H

class  Scene;
namespace Engine::priv {
    class EditorWindowSceneFunctions;
}

#include <sstream>

namespace Engine::priv {
    class EditorWindowScene final {
        friend class EditorWindowSceneFunctions;
        public:
            enum class TabType {
                Entities = 0,
                Renderer,
                Resources,
                Profiler,
                Network,
            _TOTAL,};
         private:
            std::stringstream  m_Strm;
            int                m_Tab = 0;


            void internal_render_entities(Scene& currentScene);
            void internal_render_renderer(Scene& currentScene);
            void internal_render_resources(Scene& currentScene);
            void internal_render_profiler(Scene& currentScene);
            void internal_render_network(Scene& currentScene);
        public:
            EditorWindowScene() = default;

            void update();
    };
}

#endif