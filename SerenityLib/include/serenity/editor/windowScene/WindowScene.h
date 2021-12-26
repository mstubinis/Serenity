#pragma once
#ifndef ENGINE_EDITOR_WINDOW_SCENE_H
#define ENGINE_EDITOR_WINDOW_SCENE_H

class  Scene;
class  ComponentScript;
namespace Engine::priv {
    class  EditorCore;
    class  EditorWindowSceneFunctions;
}

#include <sstream>
#include <unordered_map>
#include <serenity/system/Macros.h>

namespace Engine::priv {
    class EditorWindowScene final {
        friend class Engine::priv::EditorWindowSceneFunctions;
        friend class Engine::priv::EditorCore;
        public:
            class TabType {
                public: 
                    enum Type : uint32_t {
                        Entities = 0,
                        Renderer,
                        Resources,
                        Profiler,
                        Network,
                        _TOTAL,
                    };
                    BUILD_ENUM_CLASS_MEMBERS(TabType, Type)
            };
            struct ScriptContent {
                std::string  data;
                bool         fromFile = false;
            };
         private:
             std::unordered_map<uint32_t, ScriptContent>  m_ComponentScriptContent; //entity id => ScriptContent
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