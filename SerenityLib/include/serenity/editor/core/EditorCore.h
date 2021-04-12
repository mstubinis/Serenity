#pragma once
#ifndef ENGINE_EDITOR_CORE_H
#define ENGINE_EDITOR_CORE_H

struct EngineOptions;
class  Window;
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
        public:
            EditorCore(const EngineOptions&);
            ~EditorCore();

            bool isWindowRegistered(Window&) const noexcept;

            void init(const EngineOptions&, Engine::priv::ResourceManager&);

            inline bool isShown() const noexcept { return m_Shown; }

            inline void show(bool shown = true) noexcept { m_Shown = shown; }

            //TODO: restore previous cursor?
            inline void hide() noexcept {
                m_Shown = false;
            }

            void processEvent(const sf::Event&);
            void update(Window&, const float dt);
            void render(Window&);

    };
}

#endif