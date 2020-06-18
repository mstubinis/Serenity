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
        static const glm::mat4 GetViewNoTranslation(const Camera& camera);
        static const glm::mat4 GetViewInverseNoTranslation(const Camera& camera);
        static const glm::mat4 GetViewProjectionNoTranslation(const Camera& camera);
        static const glm::mat4 GetViewProjectionInverseNoTranslation(const Camera& camera);
        static const glm::vec3 GetViewVectorNoTranslation(const Camera& camera);
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

        CameraType  m_Type;
        glm_vec3    m_Eye                       = glm_vec3(0.0);
        glm_vec3    m_Up                        = glm_vec3(0.0, 1.0, 0.0);
        glm_vec3    m_Forward                   = glm_vec3(0.0, 0.0, -1.0);
        glm::mat4   m_ViewMatrix;
        glm::mat4   m_ViewMatrixNoTranslation;
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

        ~ComponentCamera();

        void resize(const unsigned int width, const unsigned int height);
        void lookAt(const glm_vec3& eye, const glm_vec3& forward, const glm_vec3& up);

        void setViewMatrix(const glm::mat4& viewMatrix);
        void setProjectionMatrix(const glm::mat4& projectionMatrix);

	    float getAngle() const;    void setAngle(const float angle);
	    float getAspect() const;   void setAspect(const float aspectRatio);
	    float getNear() const;     void setNear(const float Near);
	    float getFar() const;      void setFar(const float Far);

        glm::mat4 getProjection() const;
        glm::mat4 getProjectionInverse() const;
        glm::mat4 getView() const;
        glm::mat4 getViewInverse() const;
        glm::mat4 getViewProjection() const;
        glm::mat4 getViewProjectionInverse() const;
        glm::vec3 getViewVector() const;

	    glm_vec3 forward() const;
		glm_vec3 right() const;
		glm_vec3 up() const;

		unsigned int pointIntersectTest(const glm_vec3& objectPosition) const;
		unsigned int sphereIntersectTest(const glm_vec3& objectPosition, const float objectRadius) const;
};

class ComponentCamera_System_CI : public Engine::priv::ECSSystemCI {
    public:
        ComponentCamera_System_CI();
        ~ComponentCamera_System_CI() = default;
};

#endif