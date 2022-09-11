#pragma once
#ifndef ENGINE_EDITOR_WINDOW_SCENE_H
#define ENGINE_EDITOR_WINDOW_SCENE_H

class  Scene;
class  ComponentScript;
class  Shader;
class  Texture;
namespace Engine::priv {
    class EditorCore;
    class EditorWindowSceneImpl;
}

#include <sstream>
#include <unordered_map>
#include <serenity/system/Macros.h>

namespace Engine::priv {
    struct ScriptContent {
        std::string  data;
        bool         fromFile = false;
    };
    struct TextureContent {
        float asiotropicFiltering = 1.0f;
    };

    class EditorWindowScene final {
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
        friend class EditorWindowSceneImpl;
        private:
            std::unordered_map<uint32_t, Engine::priv::ScriptContent>     COMPONENT_SCRIPT_CONTENT; //entity id => ScriptContent(string, bool)
            std::unordered_map<std::string, Engine::priv::ScriptContent>  SHADER_CONTENT; //shader name => ScriptContent(string, bool)

            std::unordered_map<Texture*, Engine::priv::TextureContent>     TEXTURE_CONTENT;
             int                                          m_Tab = 0;
        public:
             EditorWindowScene() = default;

             void update();

             bool addComponentScriptData(uint32_t entityID, std::string_view scriptFilePathOrData, bool isFile);
             void addShaderData(Shader&, std::string_view shaderCode);
    };
}

#endif