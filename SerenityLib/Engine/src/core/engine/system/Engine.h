#pragma once
#ifndef ENGINE_ENGINE_H
#define ENGINE_ENGINE_H

struct EngineOptions;
class  Window;
namespace Engine::priv {
    class WindowData;
};

#include <core/engine/utils/Engine_Debugging.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/physics/PhysicsModule.h>
#include <core/engine/renderer/Renderer.h>
#include <core/engine/sounds/SoundModule.h>
#include <core/engine/events/EventModule.h>
#include <core/engine/system/EngineEventHandler.h>
#include <core/engine/math/SimplexNoise.h>
#include <core/engine/threading/ThreadingModule.h>
#include <core/engine/networking/NetworkingModule.h>
#include <core/engine/discord/DiscordModule.h>
#include <core/engine/lua/LuaModule.h>
#include <core/engine/mesh/BuiltInMeshes.h>
#include <core/engine/containers/Queue_ts.h>

namespace Engine::priv {
    class EngineCore final {
        friend class Window;
        friend class Engine::priv::WindowData;
        private:
            void internal_init_os_specific(const EngineOptions& options);
            void internal_cleanup_os_specific();

            void internal_pre_update(Scene& scene, Window& window, const float dt);
            void internal_post_update(Scene& scene, Window& window, const float dt);

            void internal_update_logic(Scene& scene, Window& window, const float dt);
            void internal_update_sounds(Scene& scene, Window& window, const float dt);
            void internal_update_physics(Scene& scene, Window& window, const float dt);
            void internal_render(Scene& scene, Window& window, const float dt);
            void internal_cleanup(Window& window, const float dt);
        public:
            struct Misc final {
                Engine::priv::BuiltInMeshses             m_BuiltInMeshes;
                SimplexNoise                             m_SimplexNoise;
                Engine::queue_ts<std::function<void()>>  m_QueuedCommands;
                std::thread::id                          m_MainThreadID;
                bool                                     m_Paused          = false;
                bool                                     m_Destroyed       = false;
            };
            Misc                m_Misc;

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

            EngineCore(const EngineOptions& options);
            ~EngineCore();

            void init(const EngineOptions& options);
            void run();
    };
    class Core final {
        public:
            static EngineCore* m_Engine;
    };
}

namespace Engine{
    //TODO: move / remove
    //void reset_malloc_count() noexcept;
    //void print_malloc_count() noexcept;

    void pause(bool pause = true) noexcept;
    bool paused() noexcept;
    void unpause() noexcept;

    float getFPS() noexcept;
    Window& getWindow() noexcept;
    glm::uvec2 getWindowSize() noexcept;
    void setWindowIcon(const Texture& texture);
    void setWindowIcon(Handle textureHandle);
    void showMouseCursor() noexcept;
    void hideMouseCursor() noexcept;
    void setTimeScale(float timeScale) noexcept;
    void stop() noexcept;
    bool setFullscreen(bool isFullscreen) noexcept;
    bool setFullscreenWindowed(bool isFullscreen) noexcept;
};

#endif