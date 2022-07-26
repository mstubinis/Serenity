#pragma once
#ifndef ENGINE_ECS_COMPONENT_CAMERA_H
#define ENGINE_ECS_COMPONENT_CAMERA_H

class Camera;
class ComponentModel;
class ComponentCamera;
class SystemComponentCamera;
namespace Engine::priv {
    class  EditorWindowSceneFunctions;
};

#include <serenity/dependencies/glm.h>
#include <serenity/ecs/ECS.h>
#include <array>

namespace Engine::priv {
    struct ComponentCamera_Functions final {
        static void RebuildProjectionMatrix(ComponentCamera&) noexcept;
    };
};
class ComponentCamera final : public ComponentBaseClass<ComponentCamera> {
    friend class  Camera;
    friend class  ComponentModel;
    friend class  SystemComponentCamera;
    friend struct Engine::priv::ComponentCamera_Functions;
    friend class  Engine::priv::EditorWindowSceneFunctions;
    public:
        enum class CameraType : uint8_t {
            Perspective,
            Orthographic,
        };
    private:
        std::array<glm::vec4, 6> m_FrustumPlanes;
        glm::mat4                m_ViewMatrix          = glm::mat4{ 1.0f };
        glm::mat4                m_ProjectionMatrix    = glm::mat4{ 1.0f };
        float                    m_NearPlane           = 0.01f;
        float                    m_FarPlane            = 2000.0f;
        float                    m_Bottom              = 0.0f;
        float                    m_Top                 = 0.0f;
	    float                    m_AngleOrLeft         = 60.0f;
		float                    m_AspectRatioOrRight  = 1.0f;
        CameraType               m_Type                = CameraType::Perspective;

        ComponentCamera() = delete;
    public:
        ComponentCamera(Entity, float angle, float aspectRatio, float nearPlane, float farPlane);
		ComponentCamera(Entity, float left, float right, float bottom, float top, float nearPlane, float farPlane);

        ComponentCamera(const ComponentCamera&) = delete;
        ComponentCamera& operator=(const ComponentCamera&) = delete;
        ComponentCamera(ComponentCamera&&) noexcept;
        ComponentCamera& operator=(ComponentCamera&&) noexcept;

        void resize(uint32_t width, uint32_t height) noexcept;
        void lookAt(const glm::vec3& eye, const glm::vec3& forward, const glm::vec3& up) noexcept;

        void setViewMatrix(const glm::mat4& viewMatrix) noexcept;
        void setViewMatrix(glm::mat4&& viewMatrix) noexcept;
        void setProjectionMatrix(const glm::mat4& projectionMatrix) noexcept;
        void setProjectionMatrix(glm::mat4&& projectionMatrix) noexcept;

        [[nodiscard]] inline CameraType getType() const noexcept { return m_Type; }
        [[nodiscard]] inline float getAngle() const noexcept { return m_AngleOrLeft; }
        [[nodiscard]] inline float getAspectRatio() const noexcept { return m_AspectRatioOrRight; }
        [[nodiscard]] inline float getNear() const noexcept { return m_NearPlane; }
        [[nodiscard]] inline float getFar() const noexcept { return m_FarPlane; }

        void setAngle(float angle) noexcept;
        void setAspectRatio(float aspectRatio) noexcept;
        void setNear(float Near) noexcept;
        void setFar(float Far) noexcept;

        [[nodiscard]] inline glm::mat4 getProjection() const noexcept { return m_ProjectionMatrix; }
        [[nodiscard]] inline glm::mat4 getView() const noexcept { return m_ViewMatrix; }
        [[nodiscard]] inline glm::mat4 getProjectionInverse() const noexcept { return glm::inverse(m_ProjectionMatrix); }
        [[nodiscard]] inline glm::mat4 getViewInverse() const noexcept { return glm::inverse(m_ViewMatrix); }
        [[nodiscard]] inline glm::mat4 getViewProjection() const noexcept { return m_ProjectionMatrix * m_ViewMatrix; }
        [[nodiscard]] inline glm::mat4 getViewProjectionInverse() const noexcept { return glm::inverse(m_ProjectionMatrix * m_ViewMatrix); }
        [[nodiscard]] inline glm::vec3 getViewVector() const noexcept { return glm::normalize(glm::vec3(m_ViewMatrix[0][2], m_ViewMatrix[1][2], m_ViewMatrix[2][2])); }

        [[nodiscard]] inline const std::array<glm::vec4, 6>& getFrustrumPlanes() const noexcept { return m_FrustumPlanes; }
        [[nodiscard]] inline glm::vec3 getRight() const noexcept { return glm::normalize(glm::vec3(m_ViewMatrix[0][0], m_ViewMatrix[1][0], m_ViewMatrix[2][0])); }
};

namespace Engine::priv {
    class ComponentCameraLUABinder {
        private:
            Entity m_Owner;

            ComponentCameraLUABinder() = delete;
        public:
            ComponentCameraLUABinder(Entity owner)
                : m_Owner{ owner }
            {}
    };
}

#endif