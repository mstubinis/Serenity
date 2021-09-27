#pragma once
#ifndef ENGINE_ECS_COMPONENT_CAMERA_H
#define ENGINE_ECS_COMPONENT_CAMERA_H

class Camera;
class ComponentModel;
class ComponentCamera;
namespace Engine::priv {
    class  EditorWindowScene;
};
namespace Engine::priv::Culling {
    class  SphereIntersectTest;
    class  PointIntersectTest;
}

#include <serenity/dependencies/glm.h>
#include <serenity/ecs/components/ComponentBaseClass.h>
#include <array>

namespace Engine::priv {
    struct ComponentCamera_Functions final {
        static void RebuildProjectionMatrix(ComponentCamera&) noexcept;
    };
};
class ComponentCamera final : public ComponentBaseClass<ComponentCamera> {
    friend class  Camera;
    friend class  ComponentModel;
    friend struct Engine::priv::ComponentCamera_Functions;
    friend class  Engine::priv::EditorWindowScene;
    friend class  Engine::priv::Culling::SphereIntersectTest;
    friend class  Engine::priv::Culling::PointIntersectTest;
    public:
        enum class CameraType : uint8_t {
            Perspective,
            Orthographic,
        };
    private:
        glm::mat4                m_ViewMatrix        = glm::mat4{ 1.0f };
        glm::mat4                m_ProjectionMatrix  = glm::mat4{ 1.0f };

        std::array<glm::vec4, 6> m_FrustumPlanes;

        float                    m_NearPlane         = 0.01f;
        float                    m_FarPlane          = 2000.0f;
        float                    m_Bottom            = 0.0f;
        float                    m_Top               = 0.0f;
        union { 
			float                m_Angle             = 60.0f;
			float                m_Left; 
		};
        union { 
			float                m_AspectRatio       = 1.0f;
			float                m_Right; 
		};
        CameraType               m_Type              = CameraType::Perspective;

        ComponentCamera() = delete;
    public:
        ComponentCamera(Entity, float angle, float aspectRatio, float nearPlane, float farPlane);
		ComponentCamera(Entity, float left, float right, float bottom, float top, float nearPlane, float farPlane);

        ComponentCamera(const ComponentCamera&);
        ComponentCamera& operator=(const ComponentCamera&);
        ComponentCamera(ComponentCamera&&) noexcept;
        ComponentCamera& operator=(ComponentCamera&&) noexcept;

        void resize(uint32_t width, uint32_t height) noexcept;
        void lookAt(const glm::vec3& eye, const glm::vec3& forward, const glm::vec3& up) noexcept;

        void setViewMatrix(const glm::mat4& viewMatrix) noexcept;
        void setViewMatrix(glm::mat4&& viewMatrix) noexcept;
        inline void setProjectionMatrix(const glm::mat4& projectionMatrix) noexcept { m_ProjectionMatrix = projectionMatrix; }

        [[nodiscard]] inline constexpr CameraType getType() const noexcept { return m_Type; }
        [[nodiscard]] inline constexpr float getAngle() const noexcept { return m_Angle; }
        [[nodiscard]] inline constexpr float getAspectRatio() const noexcept { return m_AspectRatio; }
        [[nodiscard]] inline constexpr float getNear() const noexcept { return m_NearPlane; }
        [[nodiscard]] inline constexpr float getFar() const noexcept { return m_FarPlane; }

        void setAngle(float angle) noexcept;
        void setAspectRatio(float aspectRatio) noexcept;
        void setNear(float Near) noexcept;
        void setFar(float Far) noexcept;

        [[nodiscard]] inline constexpr glm::mat4 getProjection() const noexcept { return m_ProjectionMatrix; }
        [[nodiscard]] inline constexpr glm::mat4 getView() const noexcept { return m_ViewMatrix; }
        [[nodiscard]] inline glm::mat4 getProjectionInverse() const noexcept { return glm::inverse(m_ProjectionMatrix); }
        [[nodiscard]] inline glm::mat4 getViewInverse() const noexcept { return glm::inverse(m_ViewMatrix); }
        [[nodiscard]] inline glm::mat4 getViewProjection() const noexcept { return m_ProjectionMatrix * m_ViewMatrix; }
        [[nodiscard]] inline glm::mat4 getViewProjectionInverse() const noexcept { return glm::inverse(m_ProjectionMatrix * m_ViewMatrix); }
        [[nodiscard]] inline glm::vec3 getViewVector() const noexcept { return glm::normalize(glm::vec3(m_ViewMatrix[0][2], m_ViewMatrix[1][2], m_ViewMatrix[2][2])); }

        [[nodiscard]] inline std::array<glm::vec4, 6>& getFrustrumPlanes() noexcept { return m_FrustumPlanes; }
        [[nodiscard]] inline glm::vec3 getRight() const noexcept { return glm::normalize(glm::vec3(m_ViewMatrix[0][0], m_ViewMatrix[1][0], m_ViewMatrix[2][0])); }
};

#endif