#pragma once
#ifndef ENGINE_EDITOR_CORE_H
#define ENGINE_EDITOR_CORE_H

struct EngineOptions;
class  Window;
class  Scene;
class  Camera;
class  Shader;
class  Viewport;
namespace Engine::priv {
    class  ResourceManager;
    class  EditorWindowScene;
}

#include <serenity/types/Types.h>
#include <unordered_set>
#include <SFML/Window/Event.hpp>
#include <serenity/ecs/entity/Entity.h>

namespace Engine::priv {
    class EditorCore final {
        public:
            static Engine::view_ptr<EditorCore> EDITOR;
        private:
            std::unordered_set<Window*>                          m_RegisteredWindows;

            Engine::priv::EditorWindowScene*                     m_WindowScene = nullptr;

            Handle                                               m_RodLightTexture;
            Handle                                               m_SpotLightTexture;
            Handle                                               m_SunLightTexture;
            Handle                                               m_PointLightTexture;

            bool                                                 m_Shown = true;
            bool                                                 m_Enabled = false;
        public:
            EditorCore(const EngineOptions&, Window&);
            ~EditorCore();

            bool addComponentScriptData(Entity, std::string_view scriptFilePathOrData, bool isFile);
            void addShaderData(Shader&, std::string_view shaderCode);

            bool isWindowRegistered(Window&) const noexcept;

            [[nodiscard]] inline bool isEnabled() const noexcept { return m_Enabled; }
            [[nodiscard]] inline bool isShown() const noexcept { return m_Shown; }

            inline void show(bool shown = true) noexcept { m_Shown = shown; }

            //TODO: restore previous cursor?
            inline void hide() noexcept { m_Shown = false; }

            void processEvent(const sf::Event&);
            void update(Window&, const float dt);
            void renderLightIcons(Scene&, Viewport&);
            void render(Window&);

    };
}

#endif