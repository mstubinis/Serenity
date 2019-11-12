#pragma once
#ifndef ENGINE_ECS_COMPONENT_CAMERA_H
#define ENGINE_ECS_COMPONENT_CAMERA_H

#include <ecs/ComponentBaseClass.h>
#include <ecs/ECSSystemConstructorInfo.h>
#include <core/engine/math/Numbers.h>

class Camera;
class ComponentModel;
class ComponentCamera;

namespace Engine {
    namespace epriv {
        struct ComponentCamera_UpdateFunction;
        struct ComponentCamera_EntityAddedToSceneFunction;
        struct ComponentCamera_ComponentAddedToEntityFunction;
        struct ComponentCamera_SceneEnteredFunction;
        struct ComponentCamera_SceneLeftFunction;

        struct ComponentCamera_Functions final {
            static void            RebuildProjectionMatrix(ComponentCamera& componentCamera);
            static const glm::mat4 GetViewNoTranslation(Camera& camera);
            static const glm::mat4 GetViewInverseNoTranslation(Camera& camera);
            static const glm::mat4 GetViewProjectionNoTranslation(Camera& camera);
            static const glm::mat4 GetViewProjectionInverseNoTranslation(Camera& camera);
            static const glm::vec3 GetViewVectorNoTranslation(Camera& camera);
        };
    };
};

class ComponentCamera : public ComponentBaseClass {
    friend struct Engine::epriv::ComponentCamera_UpdateFunction;
    friend struct Engine::epriv::ComponentCamera_EntityAddedToSceneFunction;
    friend struct Engine::epriv::ComponentCamera_ComponentAddedToEntityFunction;
    friend struct Engine::epriv::ComponentCamera_SceneEnteredFunction;
    friend struct Engine::epriv::ComponentCamera_SceneLeftFunction;
    friend struct Engine::epriv::ComponentCamera_Functions;
    friend class  ::Camera;
    friend class  ::ComponentModel;
    private:
        enum Type { 
			Perspective,
			Orthographic, 
		};
        Type        m_Type;
        glm_vec3    m_Eye, m_Up, m_Forward;
        glm::mat4   m_ViewMatrix, m_ViewMatrixNoTranslation, m_ProjectionMatrix;
        glm::vec4   m_FrustumPlanes[6];
        float       m_NearPlane, m_FarPlane, m_Bottom, m_Top;
        union { 
			float   m_Angle;
			float   m_Left; 
		};
        union { 
			float   m_AspectRatio;
			float   m_Right; 
		};
    public:
        BOOST_TYPE_INDEX_REGISTER_CLASS
        ComponentCamera(const Entity& entity, const float angle, const float aspectRatio, const float nearPlane, const float farPlane);
		ComponentCamera(const Entity& entity, const float left, const float right, const float bottom, const float top, const float nearPlane, const float farPlane);

        ComponentCamera(const ComponentCamera& other)                = delete;
        ComponentCamera& operator=(const ComponentCamera& other)     = delete;
        ComponentCamera(ComponentCamera&& other) noexcept            = default;
        ComponentCamera& operator=(ComponentCamera&& other) noexcept = default;

        ~ComponentCamera();

        void resize(const unsigned int width, const unsigned int height);
        void lookAt(const glm_vec3& eye, const glm_vec3& forward, const glm_vec3& up);

		const float getAngle() const;    void setAngle(const float angle);
		const float getAspect() const;   void setAspect(const float aspectRatio);
		const float getNear() const;     void setNear(const float near);
		const float getFar() const;      void setFar(const float far);

		const glm::mat4 getProjection() const;
		const glm::mat4 getProjectionInverse() const;
		const glm::mat4 getView() const;
		const glm::mat4 getViewInverse() const;
		const glm::mat4 getViewProjection() const;
		const glm::mat4 getViewProjectionInverse() const;
		const glm::vec3 getViewVector() const;

		const glm_vec3 forward() const;
		const glm_vec3 right() const;
		const glm_vec3 up() const;

		const unsigned int pointIntersectTest(const glm_vec3& objectPosition) const;
		const unsigned int sphereIntersectTest(const glm_vec3& objectPosition, const float& objectRadius) const;
};

class ComponentCamera_System_CI : public Engine::epriv::ECSSystemCI {
    public:
        ComponentCamera_System_CI();
        ~ComponentCamera_System_CI() = default;
};

#endif