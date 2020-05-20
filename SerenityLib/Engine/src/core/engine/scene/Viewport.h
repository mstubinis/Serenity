#pragma once
#ifndef ENGINE_SCENE_VIEWPORT_H
#define ENGINE_SCENE_VIEWPORT_H

class  Scene;
class  Camera;
class  LightProbe;
namespace Engine::priv {
    class Renderer;
};

#include <glm/vec4.hpp>

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
        Viewport(const Scene& scene, const Camera& camera);

        Viewport(Viewport&& other) noexcept;
        Viewport& operator=(Viewport&& other) noexcept;

        ~Viewport();

        unsigned int id() const;
        void setID(const unsigned int id);

        Engine::Flag<unsigned short> getRenderFlags() const;
        void setRenderFlag(const ViewportRenderingFlag::Flag flag);
        void addRenderFlag(const ViewportRenderingFlag::Flag flag);
        void removeRenderFlag(const ViewportRenderingFlag::Flag flag);

        float getDepthMaskValue() const;
        void setDepthMaskValue(const float depth);

        void activateDepthMask(const bool active = true);
        bool isDepthMaskActive() const;

        const glm::vec4& getBackgroundColor() const;
        void setBackgroundColor(const float r, const float g, const float b, const float a);

        void setAspectRatioSynced(const bool synced);
        bool isAspectRatioSynced() const;

        void activate(const bool activate = true);
        bool isActive() const;


        const Scene& getScene() const;
        const Camera& getCamera() const;
        const glm::vec4& getViewportDimensions() const;

        void setCamera(const Camera& camera);
        void setViewportDimensions(const float x, const float y, const float width, const float height);
};

#endif