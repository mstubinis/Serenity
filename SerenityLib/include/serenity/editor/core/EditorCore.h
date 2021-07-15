#pragma once
#ifndef ENGINE_EDITOR_CORE_H
#define ENGINE_EDITOR_CORE_H

struct EngineOptions;
class  Window;
class  Scene;
class  Camera;
namespace Engine::priv {
    class ResourceManager;
}

#include <serenity/types/Types.h>
#include <unordered_set>
#include <SFML/Window/Event.hpp>

namespace Engine::priv {
    class EditorCore final {
        public:
            static Engine::view_ptr<EditorCore> EDITOR;
        private:
            std::unordered_set<Window*> m_RegisteredWindows;

            bool                        m_Shown              = true;
            bool                        m_Enabled            = false;

            Handle                      m_RodLightTexture;
            Handle                      m_SpotLightTexture;
            Handle                      m_SunLightTexture;
            Handle                      m_PointLightTexture;


            Handle internal_load_embedded_image(const uint8_t* data, int width, int height, const char* textureName);
        public:
            EditorCore(const EngineOptions&);
            ~EditorCore();

            bool isWindowRegistered(Window&) const noexcept;

            void init(const EngineOptions&, Engine::priv::ResourceManager&);

            inline bool isShown() const noexcept { return m_Shown; }

            inline void show(bool shown = true) noexcept { m_Shown = shown; }

            //TODO: restore previous cursor?
            inline void hide() noexcept { m_Shown = false; }

            void processEvent(const sf::Event&);
            void update(Window&, const float dt);
            void renderLightIcons(Scene&);
            void render(Window&);

    };
}

#endif