#pragma once
#ifndef ENGINE_ECS_COMPONENT_CAMERA_H
#define ENGINE_ECS_COMPONENT_CAMERA_H

class Camera;
class ComponentModel;
class ComponentCamera;

#include <serenity/ecs/Entity.h>
#include <serenity/ecs/ECSSystemConstructorInfo.h>
#include <serenity/dependencies/glm.h>
#include <array>

namespace Engine::priv {
    struct ComponentCamera_Functions final {
        static void      RebuildProjectionMatrix(ComponentCamera& componentCamera) noexcept;
        [[nodiscard]] static glm::mat4 GetViewNoTranslation(const Camera& camera) noexcept;
        [[nodiscard]] static glm::mat4 GetViewInverseNoTranslation(const Camera& camera) noexcept;
        [[nodiscard]] static glm::mat4 GetViewProjectionNoTranslation(const Camera& camera) noexcept;
        [[nodiscard]] static glm::mat4 GetViewProjectionInverseNoTranslation(const Camera& camera) noexcept;
        [[nodiscard]] static glm::vec3 GetViewVectorNoTranslation(const Camera& camera) noexcept;
    };
};
class ComponentCamera final {
    friend class  Camera;
    friend class  ComponentModel;
    friend struct Engine::priv::ComponentCamera_Functions;
    private:
        enum class CameraType : unsigned char { 
			Perspective,
			Orthographic, 
		};

        glm::mat4                m_ViewMatrix                = glm::mat4(1.0f);
        glm::mat4                m_ViewMatrixNoTranslation   = glm::mat4(1.0f);
        glm::mat4                m_ProjectionMatrix;

        std::array<glm::vec4, 6> m_FrustumPlanes;
        glm::vec3                m_Eye                       = glm::vec3(0.0f);
        glm::vec3                m_Up                        = glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3                m_Forward                   = glm::vec3(0.0f, 0.0f, -1.0f);
        Entity                   m_Owner;

        float                    m_NearPlane                 = 0.01f;
        float                    m_FarPlane                  = 2000.0f;
        float                    m_Bottom                    = 0.0f;
        float                    m_Top                       = 0.0f;
        union { 
			float                m_Angle;
			float                m_Left; 
		};
        union { 
			float                m_AspectRatio;
			float                m_Right; 
		};
        CameraType               m_Type = CameraType::Perspective;

        ComponentCamera() = delete;
    public:
        //BOOST_TYPE_INDEX_REGISTER_CLASS
        ComponentCamera(Entity entity, float angle, float aspectRatio, float nearPlane, float farPlane);
		ComponentCamera(Entity entity, float left, float right, float bottom, float top, float nearPlane, float farPlane);

        ComponentCamera(const ComponentCamera& other)                = delete;
        ComponentCamera& operator=(const ComponentCamera& other)     = delete;
        ComponentCamera(ComponentCamera&& other) noexcept;
        ComponentCamera& operator=(ComponentCamera&& other) noexcept;

        ~ComponentCamera() = default;

        void resize(uint32_t width, uint32_t height) noexcept;
        void lookAt(const glm::vec3& eye, const glm::vec3& forward, const glm::vec3& up) noexcept;
        void lookAt(glm::vec3&& eye, glm::vec3&& forward, glm::vec3&& up) noexcept;

        void setViewMatrix(const glm::mat4& viewMatrix) noexcept;
        void setViewMatrix(glm::mat4&& viewMatrix) noexcept;
        inline void setProjectionMatrix(const glm::mat4& projectionMatrix) noexcept { m_ProjectionMatrix = projectionMatrix; }

        [[nodiscard]] inline constexpr float getAngle() const noexcept { return m_Angle; }
        [[nodiscard]] inline constexpr float getAspect() const noexcept { return m_AspectRatio; }
        [[nodiscard]] inline constexpr float getNear() const noexcept { return m_NearPlane; }
        [[nodiscard]] inline constexpr float getFar() const noexcept { return m_FarPlane; }

        void setAngle(float angle) noexcept;
        void setAspect(float aspectRatio) noexcept;
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
        [[nodiscard]] inline constexpr glm::vec3 forward() const noexcept { return m_Forward; }
        [[nodiscard]] inline glm::vec3 right() const noexcept { return glm::normalize(glm::vec3(m_ViewMatrixNoTranslation[0][0], m_ViewMatrixNoTranslation[1][0], m_ViewMatrixNoTranslation[2][0])); }
        [[nodiscard]] inline constexpr glm::vec3 up() const noexcept { return m_Up; /*normalize later?*/ }

        [[nodiscard]] uint32_t pointIntersectTest(const glm_vec3& objectPosition) const noexcept;
        [[nodiscard]] uint32_t sphereIntersectTest(const glm_vec3& objectPosition, float objectRadius) const noexcept;
};

class ComponentCamera_System_CI : public Engine::priv::ECSSystemCI {
    public:
        ComponentCamera_System_CI();
        ~ComponentCamera_System_CI() = default;
};

#endif