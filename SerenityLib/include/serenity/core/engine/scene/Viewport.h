#pragma once
#ifndef ENGINE_SCENE_VIEWPORT_H
#define ENGINE_SCENE_VIEWPORT_H

class  Scene;
class  Camera;
class  LightProbe;
namespace Engine::priv {
    class RenderModule;
};

#include <serenity/core/engine/system/TypeDefs.h>
#include <serenity/core/engine/dependencies/glm.h>
#include <serenity/core/engine/types/Flag.h>

struct ViewportRenderingFlag final { enum Flag: uint16_t {
    GodRays      = 1 << 0,
    SSAO         = 1 << 1,
    API2D        = 1 << 2,
    HDR          = 1 << 3,
    PhysicsDebug = 1 << 4,
    AntiAliasing = 1 << 5,
    Fog          = 1 << 6,
    DepthOfField = 1 << 7,
    Skybox       = 1 << 8,
    Bloom        = 1 << 9,
    Particles    = 1 << 10,
    _ALL         = 65535,
};};

class Viewport final {
    friend class Scene;
    friend class Engine::priv::RenderModule;
    friend class LightProbe;
    private:
        struct StateFlags final { enum Flag : uint8_t {
            Active              = 1 << 0,
            AspectRatioSynced   = 1 << 1,
            DepthMaskActive     = 1 << 2,
        };};

        glm::vec4                     m_Viewport_Dimensions = glm::vec4(0.0f, 0.0f, 256.0f, 256.0f);
        glm::vec4                     m_BackgroundColor     = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        Scene*                        m_Scene               = nullptr;
        Camera*                       m_Camera              = nullptr;
        float                         m_DepthMaskValue      = 50.0f;
        uint32_t                      m_ID                  = 0;
        Engine::Flag<uint16_t>        m_RenderFlags;
        Engine::Flag<uint8_t>         m_StateFlags;

        Viewport();
    public:
        Viewport(Scene& scene, Camera& camera);

        Viewport(Viewport&& other) noexcept;
        Viewport& operator=(Viewport&& other) noexcept;

        ~Viewport() {}

        inline constexpr uint32_t id() const noexcept { return m_ID; }
        inline void setID(uint32_t id) noexcept { m_ID = id; }

        inline Engine::Flag<uint16_t> getRenderFlags() const noexcept { return m_RenderFlags; }
        inline void setRenderFlag(ViewportRenderingFlag::Flag flag) noexcept { m_RenderFlags = flag; }
        inline void addRenderFlag(ViewportRenderingFlag::Flag flag) noexcept { m_RenderFlags.add(flag); }
        inline void removeRenderFlag(ViewportRenderingFlag::Flag flag) noexcept { m_RenderFlags.remove(flag); }

        inline constexpr float getDepthMaskValue() const noexcept { return m_DepthMaskValue; }
        inline void setDepthMaskValue(float depth) noexcept { m_DepthMaskValue = depth; }
        void activateDepthMask(bool active = true);
        inline constexpr bool isDepthMaskActive() const noexcept { return m_StateFlags.has(StateFlags::DepthMaskActive); }

        inline constexpr const glm::vec4& getBackgroundColor() const noexcept { return m_BackgroundColor; }
        void setBackgroundColor(float r, float g, float b, float a);

        void setAspectRatioSynced(bool synced);
        bool isAspectRatioSynced() const;

        void activate(bool activate = true);
        inline constexpr bool isActive() const noexcept { return m_StateFlags.has(StateFlags::Active); }

        inline constexpr Scene& getScene() noexcept { return *m_Scene; }
        inline constexpr Camera& getCamera() noexcept { return *m_Camera; }
        inline constexpr const glm::vec4& getViewportDimensions() const noexcept { return m_Viewport_Dimensions; }

        inline void setCamera(Camera& camera) noexcept { m_Camera = &camera; }
        void setViewportDimensions(float x, float y, float width, float height);
};

#endif