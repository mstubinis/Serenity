#pragma once
#ifndef ENGINE_SCENE_VIEWPORT_H
#define ENGINE_SCENE_VIEWPORT_H

#include <glm/vec4.hpp>

class  Scene;
class  Camera;

namespace Engine {
    namespace epriv {
        class RenderManager;
    };
};

class Viewport final {
    friend class Scene;
    friend class Engine::epriv::RenderManager;
    private:
        Scene&         m_Scene;
        Camera*        m_Camera;
        glm::uvec4     m_Viewport_Dimensions;
        bool           m_Active;
        bool           m_Using2DAPI;
        bool           m_AspectRatioSynced;
        bool           m_SkyboxVisible;
        glm::vec4      m_BackgroundColor;
        bool           m_DepthMaskActive;
        float          m_DepthMaskValue;
        unsigned int   m_ID;
    public:
        Viewport(const Scene& scene, const Camera& camera);

        Viewport& operator=(const Viewport& other)     = delete;
        Viewport(const Viewport& other)                = delete;
        Viewport(Viewport&& other) noexcept            = default;
        Viewport& operator=(Viewport&& other) noexcept = default;

        ~Viewport();

        const unsigned int& id() const;

        const bool& isSkyboxVisible() const;
        void setSkyboxVisible(const bool& visible = true);


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

        void activate2DAPI(const bool& active = true);
        const bool isUsing2DAPI() const;

        const Scene& getScene() const;
        const Camera& getCamera() const;
        const glm::uvec4& getViewportDimensions() const;

        bool setCamera(const Camera& camera);
        void setViewportDimensions(const unsigned int& x, const unsigned int& y, const unsigned int& width, const unsigned int& height);
};

#endif