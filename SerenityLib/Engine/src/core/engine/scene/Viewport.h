#pragma once
#ifndef ENGINE_SCENE_VIEWPORT_H
#define ENGINE_SCENE_VIEWPORT_H

class  Scene;
class  Camera;
class  LightProbe;
namespace Engine::priv {
    class Renderer;
};

struct ViewportRenderingFlag final { enum Flag: unsigned short {
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

class Viewport final : public Engine::NonCopyable {
    friend class Scene;
    friend class Engine::priv::Renderer;
    friend class LightProbe;
    private:
        struct StateFlags final { enum Flag : unsigned char {
            Active              = 1 << 0,
            AspectRatioSynced   = 1 << 1,
            DepthMaskActive     = 1 << 2,
        };};

        Engine::Flag<unsigned char>   m_StateFlags;
        Engine::Flag<unsigned short>  m_RenderFlags;

        Scene*                        m_Scene               = nullptr;
        Camera*                       m_Camera              = nullptr;
        glm::vec4                     m_Viewport_Dimensions = glm::vec4(0.0f, 0.0f, 256.0f, 256.0f);
        glm::vec4                     m_BackgroundColor     = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
        float                         m_DepthMaskValue      = 50.0f;
        unsigned int                  m_ID                  = 0;

        Viewport();
    public:
        Viewport(Scene& scene, Camera& camera);

        Viewport(Viewport&& other) noexcept;
        Viewport& operator=(Viewport&& other) noexcept;

        ~Viewport() {}

        inline constexpr unsigned int id() const noexcept { return m_ID; }
        void setID(unsigned int id) noexcept { m_ID = id; }

        Engine::Flag<unsigned short> getRenderFlags() const noexcept { return m_RenderFlags; }
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

        inline Scene& getScene() noexcept { return *m_Scene; }
        inline Camera& getCamera() noexcept { return *m_Camera; }
        inline constexpr const glm::vec4& getViewportDimensions() const noexcept { return m_Viewport_Dimensions; }

        inline void setCamera(Camera& camera) noexcept { m_Camera = &camera; }
        void setViewportDimensions(float x, float y, float width, float height);
};

#endif