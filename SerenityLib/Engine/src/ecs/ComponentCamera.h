#pragma once
#ifndef ENGINE_ECS_COMPONENT_CAMERA_H
#define ENGINE_ECS_COMPONENT_CAMERA_H

class Camera;
class ComponentModel;
class ComponentCamera;

#include <ecs/Entity.h>
#include <ecs/ECSSystemConstructorInfo.h>
#include <array>

namespace Engine::priv {
    struct ComponentCamera_Functions final {
        static void      RebuildProjectionMatrix(ComponentCamera& componentCamera);
        static glm::mat4 GetViewNoTranslation(const Camera& camera);
        static glm::mat4 GetViewInverseNoTranslation(const Camera& camera);
        static glm::mat4 GetViewProjectionNoTranslation(const Camera& camera);
        static glm::mat4 GetViewProjectionInverseNoTranslation(const Camera& camera);
        static glm::vec3 GetViewVectorNoTranslation(const Camera& camera);
    };
};

class ComponentCamera final {
    friend class  Camera;
    friend class  ComponentModel;
    friend struct Engine::priv::ComponentCamera_Functions;
    private:
        enum CameraType : unsigned char { 
			Perspective,
			Orthographic, 
		};

        Entity                   m_Owner;

        CameraType               m_Type                      = CameraType::Perspective;
        glm_vec3                 m_Eye                       = glm_vec3(0.0);
        glm_vec3                 m_Up                        = glm_vec3(0.0, 1.0, 0.0);
        glm_vec3                 m_Forward                   = glm_vec3(0.0, 0.0, -1.0);
        glm::mat4                m_ViewMatrix                = glm::mat4(1.0f);
        glm::mat4                m_ViewMatrixNoTranslation   = glm::mat4(1.0f);
        glm::mat4                m_ProjectionMatrix;

        std::array<glm::vec4, 6> m_FrustumPlanes;

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

        void resize(unsigned int width, unsigned int height);
        void lookAt(const glm_vec3& eye, const glm_vec3& forward, const glm_vec3& up);

        void setViewMatrix(const glm::mat4& viewMatrix);
        void setProjectionMatrix(const glm::mat4& projectionMatrix);

        inline constexpr float getAngle() const noexcept {
            return m_Angle;
        }
        inline constexpr float getAspect() const noexcept {
            return m_AspectRatio;
        }
        inline constexpr float getNear() const noexcept {
            return m_NearPlane;
        }
        inline constexpr float getFar() const noexcept {
            return m_FarPlane;
        }

        void setAngle(float angle);
        void setAspect(float aspectRatio);
        void setNear(float Near);
        void setFar(float Far);

        inline constexpr glm::mat4 getProjection() const noexcept {
            return m_ProjectionMatrix;
        }
        glm::mat4 getProjectionInverse() const;
        inline constexpr glm::mat4 getView() const noexcept {
            return m_ViewMatrix;
        }
        glm::mat4 getViewInverse() const;
        glm::mat4 getViewProjection() const;
        glm::mat4 getViewProjectionInverse() const;
        glm::vec3 getViewVector() const;

        inline constexpr std::array<glm::vec4, 6> getFrustrumPlanes() const noexcept {
            return m_FrustumPlanes;
        }
        inline constexpr glm_vec3 forward() const noexcept {
            return m_Forward;
        }
        inline glm_vec3 right() const noexcept {
            return glm::normalize(glm_vec3(m_ViewMatrixNoTranslation[0][0], m_ViewMatrixNoTranslation[1][0], m_ViewMatrixNoTranslation[2][0]));
        }
        inline constexpr glm_vec3 up() const noexcept {
            return m_Up; //normalize later?
        }

		unsigned int pointIntersectTest(const glm_vec3& objectPosition) const;
		unsigned int sphereIntersectTest(const glm_vec3& objectPosition, float objectRadius) const;
};

class ComponentCamera_System_CI : public Engine::priv::ECSSystemCI {
    public:
        ComponentCamera_System_CI();
        ~ComponentCamera_System_CI() = default;
};

#endif