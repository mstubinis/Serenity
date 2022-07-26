#pragma once
#ifndef ENGINE_ENGINE_H
#define ENGINE_ENGINE_H

struct EngineOptions;
class  Window;
class  GameCore;
namespace Engine::priv {
    class WindowData;
};

#include <serenity/system/window/Window.h>
#include <serenity/utils/Engine_Debugging.h>
#include <serenity/resources/Engine_Resources.h>
#include <serenity/resources/sound/SoundModule.h>
#include <serenity/resources/mesh/BuiltInMeshes.h>
#include <serenity/physics/PhysicsModule.h>
#include <serenity/renderer/Renderer.h>
#include <serenity/events/EventModule.h>
#include <serenity/input/InputModule.h>
#include <serenity/system/EngineEventHandler.h>
#include <serenity/math/SimplexNoise.h>
#include <serenity/threading/ThreadingModule.h>
#include <serenity/networking/NetworkingModule.h>
#include <serenity/discord/DiscordModule.h>
#include <serenity/lua/LuaModule.h>
#include <serenity/editor/core/EditorCore.h>
#include <serenity/containers/Queue_ts.h>

namespace Engine::priv {
    class EngineWindows final {
        using ContainerType = std::vector<Window>;
        private:
            ContainerType m_WindowsVector;
            EngineWindows() = delete;
        public:
            EngineWindows(const EngineOptions&);

            EngineWindows(const EngineWindows&)                = delete;
            EngineWindows& operator=(const EngineWindows&)     = delete;
            EngineWindows(EngineWindows&&) noexcept            = delete;
            EngineWindows& operator=(EngineWindows&&) noexcept = delete;

            [[nodiscard]] inline size_t size() const noexcept { return m_WindowsVector.size(); }
            [[nodiscard]] inline bool empty() const noexcept { return m_WindowsVector.empty(); }
            [[nodiscard]] inline Window& operator[](size_t index) noexcept { return m_WindowsVector[index]; }
            [[nodiscard]] inline const Window& operator[](size_t index) const noexcept { return m_WindowsVector[index]; }

            inline ContainerType::iterator begin() noexcept { return m_WindowsVector.begin(); }
            inline ContainerType::const_iterator begin() const noexcept { return m_WindowsVector.begin(); }
            inline ContainerType::iterator end() noexcept { return m_WindowsVector.end(); }
            inline ContainerType::const_iterator end() const noexcept { return m_WindowsVector.end(); }
            inline const ContainerType::const_iterator cbegin() const noexcept { return m_WindowsVector.cbegin(); }
            inline const ContainerType::const_iterator cend() const noexcept { return m_WindowsVector.cend(); }
    };
    class EngineCore final {
        friend class ::Window;
        friend class Engine::priv::WindowData;
        private:
            void internal_post_input_update(int frameIteration);
            void internal_pre_update(GameCore&, int frameIteration, Scene&, const float dt);
            void internal_post_update(Scene&, Window&, const float dt);

            void internal_update_logic(GameCore&, int frameIteration, Scene&, const float dt);
            void internal_update_sounds(GameCore&, Scene&, const float dt);
            void internal_render(GameCore&, Scene&, Window&, const float dt, const double alpha);
            void internal_cleanup();

            void init(const EngineOptions&, GameCore*);
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
                Engine::priv::EngineCore::FPSTimer       m_FPS;
                double                                   m_Dt              = 0.0; //delta time in seconds
                float                                    m_DtSimulation    = 0.0; //how much the current simulation is stepping by
                std::thread::id                          m_MainThreadID;
                bool                                     m_Paused          = false;
                bool                                     m_Destroyed       = false;
            };
            struct Modules {
                RenderModule          m_RenderModule;
                EditorCore            m_Editor;
                PhysicsModule         m_PhysicsModule;
                DebugManager          m_DebugManager;
                EngineEventHandler    m_EngineEventHandler;

                Modules() = delete;
                Modules(const EngineOptions&, EventModule&, ResourceManager&, InputModule&);
            };
            std::unique_ptr<Modules>  m_Modules;
            LUAModule                 m_LUAModule;
            NetworkingModule          m_NetworkingModule;
            DiscordModule             m_DiscordModule;
            SoundModule               m_SoundModule;
            ThreadingModule           m_ThreadingModule;
            InputModule               m_InputModule;
            EventModule               m_EventModule;
            ResourceManager           m_ResourceManager;
            Misc                      m_Misc;

            Engine::view_ptr<GameCore>   m_GameCore = nullptr;

            EngineCore(const EngineOptions&);
            ~EngineCore();

            EngineCore(const EngineCore&) = delete;
            EngineCore& operator=(const EngineCore&) = delete;
            EngineCore(EngineCore&&) noexcept = delete;
            EngineCore& operator=(EngineCore&&) noexcept = delete;

            void run(const EngineOptions&, GameCore*);
    };
    class Core final {
        public:
            static inline Engine::view_ptr<Engine::priv::EngineWindows> m_Windows;
            static inline Engine::view_ptr<Engine::priv::EngineCore> m_Engine;
    };
}

namespace Engine{
    //TODO: move / remove
    //void reset_malloc_count() noexcept;
    //void print_malloc_count() noexcept;

    [[nodiscard]] inline Engine::priv::ResourceManager& getResourceManager() noexcept { return Engine::priv::Core::m_Engine->m_ResourceManager; }
    [[nodiscard]] inline Engine::priv::PhysicsModule& getPhysicsManager() noexcept { return Engine::priv::Core::m_Engine->m_Modules->m_PhysicsModule; }
    [[nodiscard]] inline Engine::priv::EditorCore& getEditor() noexcept { return Engine::priv::Core::m_Engine->m_Modules->m_Editor; }
    [[nodiscard]] inline Engine::priv::RenderModule& getRenderer() noexcept { return Engine::priv::Core::m_Engine->m_Modules->m_RenderModule; }


    [[nodiscard]] Window& getWindow();
    [[nodiscard]] glm::uvec2 getWindowSize();
    [[nodiscard]] Window& getWindow(uint32_t index);
    [[nodiscard]] glm::uvec2 getWindowSize(uint32_t index);
    [[nodiscard]] size_t getNumWindows();

    void pause(bool pause = true) noexcept;
    [[nodiscard]] bool paused() noexcept;
    void unpause() noexcept;

    [[nodiscard]] double getFPS() noexcept;

    inline void setWindowIcon(const Texture& texture) { Engine::getWindow().setIcon(texture); }
    inline void setWindowIcon(Handle textureHandle) { Engine::getWindow().setIcon(*textureHandle.get<Texture>()); }
    inline void showMouseCursor() noexcept { Engine::getWindow().setMouseCursorVisible(true); }
    inline void hideMouseCursor() noexcept { Engine::getWindow().setMouseCursorVisible(false); }

    void setTimeScale(float timeScale) noexcept;
    void stop() noexcept;
};

#endif