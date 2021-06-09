#pragma once
#ifndef ENGINE_ENGINE_H
#define ENGINE_ENGINE_H

struct EngineOptions;
class  Window;
namespace Engine::priv {
    class WindowData;
};

#include <serenity/system/window/Window.h>
#include <serenity/utils/Engine_Debugging.h>
#include <serenity/resources/Engine_Resources.h>
#include <serenity/physics/PhysicsModule.h>
#include <serenity/renderer/Renderer.h>
#include <serenity/resources/sound/SoundModule.h>
#include <serenity/events/EventModule.h>
#include <serenity/system/EngineEventHandler.h>
#include <serenity/math/SimplexNoise.h>
#include <serenity/threading/ThreadingModule.h>
#include <serenity/networking/NetworkingModule.h>
#include <serenity/discord/DiscordModule.h>
#include <serenity/lua/LuaModule.h>
#include <serenity/resources/mesh/BuiltInMeshes.h>
#include <serenity/editor/core/EditorCore.h>
#include <serenity/containers/Queue_ts.h>

namespace Engine::priv {
    class EngineCore final {
        friend class Window;
        friend class Engine::priv::WindowData;
        private:
            void internal_init_os_specific(const EngineOptions&);
            void internal_cleanup_os_specific();

            void internal_pre_input_update(Window&);
            void internal_pre_update(Scene&, Window&, const float dt);
            void internal_post_update(Scene&, Window&, const float dt);

            void internal_update_logic(Scene&, Window&, const float dt);
            void internal_update_sounds(Scene&, Window&, const float dt);
            void internal_render(Scene&, Window&, const float dt, const double alpha);
            void internal_cleanup(Window&, const float dt);
        public:
            class FPSTimer final {
                private:
                    double    m_FPSTimer   = 0.0; //in seconds
                    uint32_t  m_FPSCounter = 0;
                    uint32_t  m_FPS        = 0;
                public:
                    inline void update(const double dt) noexcept {
                        m_FPSTimer += dt;
                        if (m_FPSTimer > 1.0f) {
                            m_FPS        = m_FPSCounter;
                            m_FPSCounter = 0;
                            m_FPSTimer   = 0.0;
                        }
                    }
                    inline void incrementFPSCount() noexcept { ++m_FPSCounter; }
                    inline constexpr uint32_t fps() const noexcept { return m_FPS; }
            };

            struct Misc final {
                Engine::priv::BuiltInMeshses             m_BuiltInMeshes;
                SimplexNoise                             m_SimplexNoise;
                Engine::queue_ts<std::function<void()>>  m_QueuedCommands;
                Engine::priv::EngineCore::FPSTimer       m_FPS;
                std::chrono::high_resolution_clock       m_Timer;
                double                                   m_Dt              = 0.0; //delta time in seconds
                std::thread::id                          m_MainThreadID;
                bool                                     m_Paused          = false;
                bool                                     m_Destroyed       = false;
            };
            Misc                m_Misc;

            EditorCore          m_Editor;
            LUAModule           m_LUAModule;
            NetworkingModule    m_NetworkingModule;
            DiscordModule       m_DiscordModule;
            EventModule         m_EventModule;
            RenderModule        m_RenderModule;
            PhysicsModule       m_PhysicsModule;
            ResourceManager     m_ResourceManager;
            SoundModule         m_SoundModule;
            DebugManager        m_DebugManager;
            ThreadingModule     m_ThreadingModule;
            EngineEventHandler  m_EngineEventHandler;

            EngineCore(const EngineOptions&);
            ~EngineCore();

            void init(const EngineOptions&);
            void run();
    };
    class Core final {
        public:
            static inline EngineCore* m_Engine = nullptr;
    };
}

namespace Engine{
    //TODO: move / remove
    //void reset_malloc_count() noexcept;
    //void print_malloc_count() noexcept;

    void pause(bool pause = true) noexcept;
    [[nodiscard]] bool paused() noexcept;
    void unpause() noexcept;

    [[nodiscard]] double getFPS() noexcept;
    [[nodiscard]] inline Window& getWindow() noexcept { return Engine::Resources::getWindow(); }
    [[nodiscard]] inline glm::uvec2 getWindowSize() noexcept { return Engine::Resources::getWindowSize(); }

    inline void setWindowIcon(const Texture& texture) { Engine::Resources::getWindow().setIcon(texture); }
    inline void setWindowIcon(Handle textureHandle) { Engine::Resources::getWindow().setIcon(*textureHandle.get<Texture>()); }
    inline void showMouseCursor() noexcept { Engine::Resources::getWindow().setMouseCursorVisible(true); }
    inline void hideMouseCursor() noexcept { Engine::Resources::getWindow().setMouseCursorVisible(false); }

    void setTimeScale(float timeScale) noexcept;
    void stop() noexcept;
    inline bool setFullscreen(bool fullscreen) noexcept { return Engine::Resources::getWindow().setFullscreen(fullscreen); }
    inline bool setFullscreenWindowed(bool fullscreenWindowed) noexcept { return Engine::Resources::getWindow().setFullscreenWindowed(fullscreenWindowed); }
};

#endif