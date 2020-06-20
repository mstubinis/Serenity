#pragma once
#ifndef ENGINE_ECS_COMPONENT_CAMERA_H
#define ENGINE_ECS_COMPONENT_CAMERA_H

class Camera;
class ComponentModel;
class ComponentCamera;
namespace Engine::priv {
    struct ComponentCamera_UpdateFunction;
    struct ComponentCamera_EntityAddedToSceneFunction;
    struct ComponentCamera_ComponentAddedToEntityFunction;
    struct ComponentCamera_ComponentRemovedFromEntityFunction;
    struct ComponentCamera_SceneEnteredFunction;
    struct ComponentCamera_SceneLeftFunction;
};

#include <ecs/Entity.h>
#include <ecs/ECSSystemConstructorInfo.h>

namespace Engine::priv {
    struct ComponentCamera_Functions final {
        static void            RebuildProjectionMatrix(ComponentCamera& componentCamera);
        static glm::mat4 GetViewNoTranslation(const Camera& camera);
        static glm::mat4 GetViewInverseNoTranslation(const Camera& camera);
        static glm::mat4 GetViewProjectionNoTranslation(const Camera& camera);
        static glm::mat4 GetViewProjectionInverseNoTranslation(const Camera& camera);
        static glm::vec3 GetViewVectorNoTranslation(const Camera& camera);
    };
};

class ComponentCamera {
    friend struct Engine::priv::ComponentCamera_UpdateFunction;
    friend struct Engine::priv::ComponentCamera_EntityAddedToSceneFunction;
    friend struct Engine::priv::ComponentCamera_ComponentAddedToEntityFunction;
    friend struct Engine::priv::ComponentCamera_ComponentRemovedFromEntityFunction;
    friend struct Engine::priv::ComponentCamera_SceneEnteredFunction;
    friend struct Engine::priv::ComponentCamera_SceneLeftFunction;
    friend struct Engine::priv::ComponentCamera_Functions;
    friend class  ::Camera;
    friend class  ::ComponentModel;
    private:
        enum CameraType : unsigned char { 
			Perspective,
			Orthographic, 
		};

        Entity m_Owner;

        CameraType  m_Type                      = CameraType::Perspective;
        glm_vec3    m_Eye                       = glm_vec3(0.0);
        glm_vec3    m_Up                        = glm_vec3(0.0, 1.0, 0.0);
        glm_vec3    m_Forward                   = glm_vec3(0.0, 0.0, -1.0);
        glm::mat4   m_ViewMatrix                = glm::mat4(1.0f);
        glm::mat4   m_ViewMatrixNoTranslation   = glm::mat4(1.0f);
        glm::mat4   m_ProjectionMatrix;
        glm::vec4   m_FrustumPlanes[6];
        float       m_NearPlane                 = 0.01f;
        float       m_FarPlane                  = 2000.0f;
        float       m_Bottom                    = 0.0f;
        float       m_Top                       = 0.0f;
        union { 
			float   m_Angle;
			float   m_Left; 
		};
        union { 
			float   m_AspectRatio;
			float   m_Right; 
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

        inline constexpr float getAngle() const {
            return m_Angle;
        }
        inline constexpr float getAspect() const {
            return m_AspectRatio;
        }
        inline constexpr float getNear() const {
            return m_NearPlane;
        }
        inline constexpr float getFar() const {
            return m_FarPlane;
        }

        void setAngle(const float angle);
        void setAspect(const float aspectRatio);
        void setNear(const float Near);
        void setFar(const float Far);

        inline constexpr glm::mat4 getProjection() const {
            return m_ProjectionMatrix;
        }
        glm::mat4 getProjectionInverse() const;
        inline constexpr glm::mat4 getView() const {
            return m_ViewMatrix;
        }
        glm::mat4 getViewInverse() const;
        glm::mat4 getViewProjection() const;
        glm::mat4 getViewProjectionInverse() const;
        glm::vec3 getViewVector() const;

        inline constexpr glm_vec3 forward() const {
            return m_Forward;
        }
        inline glm_vec3 right() const {
            return glm::normalize(glm_vec3(m_ViewMatrixNoTranslation[0][0], m_ViewMatrixNoTranslation[1][0], m_ViewMatrixNoTranslation[2][0]));
        }
        inline constexpr glm_vec3 up() const {
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