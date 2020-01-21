#pragma once
#ifndef ENGINE_SCENE_VIEWPORT_H
#define ENGINE_SCENE_VIEWPORT_H

#include <glm/vec4.hpp>

class  Scene;
class  Camera;

namespace Engine {
    namespace priv {
        class Renderer;
    };
};

struct ViewportRenderingFlag final { enum Flag: unsigned int {
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
    _ALL         = 4294967295,
};};

class Viewport final {
    friend class Scene;
    friend class Engine::priv::Renderer;
    private:
        Scene&         m_Scene;
        Camera*        m_Camera;
        glm::vec4      m_Viewport_Dimensions;
        bool           m_Active;
        bool           m_AspectRatioSynced;
        glm::vec4      m_BackgroundColor;
        bool           m_DepthMaskActive;
        float          m_DepthMaskValue;
        unsigned int   m_ID;
        unsigned int   m_RenderFlags;
    public:
        Viewport(const Scene& scene, const Camera& camera);

        Viewport& operator=(const Viewport& other)     = delete;
        Viewport(const Viewport& other)                = delete;
        Viewport(Viewport&& other) noexcept            = default;
        Viewport& operator=(Viewport&& other) noexcept = default;

        ~Viewport();

        const unsigned int id() const;
        void setID(const unsigned int id);

        const unsigned int& getRenderFlags() const;
        void setRenderFlag(const ViewportRenderingFlag::Flag& flag);
        void addRenderFlag(const ViewportRenderingFlag::Flag& flag);
        void removeRenderFlag(const ViewportRenderingFlag::Flag& flag);

        const float& getDepthMaskValue() const;
        void setDepthMaskValue(const float& depth);

        void activateDepthMask(const bool& active = true);
        const bool isDepthMaskActive() const;

        const glm::vec4& getBackgroundColor() const;
        void setBackgroundColor(const float& r, const float& g, const float& b, const float& a);

        void setAspectRatioSynced(const bool& synced);
        const bool isAspectRatioSynced() const;

        void activate(const bool& activate = true);
        const bool isActive() const;


        const Scene& getScene() const;
        const Camera& getCamera() const;
        const glm::vec4& getViewportDimensions() const;

        bool setCamera(const Camera& camera);
        void setViewportDimensions(const float x, const float y, const float width, const float height);
};

#endif