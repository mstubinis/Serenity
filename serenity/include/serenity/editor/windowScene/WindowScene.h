#pragma once
#ifndef ENGINE_EDITOR_WINDOW_SCENE_H
#define ENGINE_EDITOR_WINDOW_SCENE_H

class  Scene;
class  ComponentScript;
class  Shader;
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
                        Systems,
                        Renderer,
                        Resources,
                        Profiler,
                        Network,
                        _TOTAL,
                    };
                    BUILD_ENUM_CLASS_MEMBERS(TabType, Type)
            };
        private:
             int                                          m_Tab = 0;
        public:
             EditorWindowScene() = default;

             void update();

             bool addComponentScriptData(uint32_t entityID, std::string_view scriptFilePathOrData, bool isFile);
             void addShaderData(Shader&, std::string_view shaderCode);
    };
}

#endif