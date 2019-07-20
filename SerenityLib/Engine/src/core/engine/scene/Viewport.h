#pragma once
#ifndef ENGINE_SCENE_VIEWPORT_H
#define ENGINE_SCENE_VIEWPORT_H

#include <glm/vec4.hpp>

class  Scene;
class  Camera;
class Viewport final {
    private:
        Scene&       m_Scene;
        Camera*      m_Camera;
        glm::uvec4   m_Viewport_Dimensions;
        bool         m_Active;
        bool         m_Using2DAPI;
        bool         m_AspectRatioSynced;
    public:
        Viewport(const Scene& scene, const Camera& camera);

        Viewport& operator=(const Viewport& other)     = delete;
        Viewport(const Viewport& other)                = delete;
        Viewport(Viewport&& other) noexcept            = default;
        Viewport& operator=(Viewport&& other) noexcept = default;

        ~Viewport();

        void setAspectRatioSynced(const bool synced);
        const bool isAspectRatioSynced() const;

        void deactivate();
        void activate();
        const bool isActive() const;

        void deactivate2DAPI();
        void activate2DAPI();
        const bool isUsing2DAPI() const;

        const Scene& getScene() const;
        const Camera& getCamera() const;
        const glm::uvec4& getViewportDimensions() const;

        bool setCamera(const Camera& camera);
        void setViewportDimensions(const unsigned int& x, const unsigned int& y, const unsigned int& width, const unsigned int& height);
};

#endif