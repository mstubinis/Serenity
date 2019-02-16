#pragma once
#ifndef ENGINE_ECS_COMPONENT_CAMERA_H
#define ENGINE_ECS_COMPONENT_CAMERA_H

#include "ecs/ComponentBaseClass.h"
#include "ecs/ECSSystem.h"

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

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
            static void      RebuildProjectionMatrix(ComponentCamera& cam);
            static glm::mat4 GetViewNoTranslation(Camera& c);
            static glm::mat4 GetViewInverseNoTranslation(Camera& c);
            static glm::mat4 GetViewProjectionNoTranslation(Camera& c);
            static glm::mat4 GetViewProjectionInverseNoTranslation(Camera& c);
            static glm::vec3 GetViewVectorNoTranslation(Camera& c);
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
        enum Type { Perspective, Orthographic, };
        Type _type;
        glm::vec3 _eye, _up;
        glm::mat4 _viewMatrix, _viewMatrixNoTranslation, _projectionMatrix;
        glm::vec4 _planes[6];
        float _nearPlane, _farPlane, _bottom, _top;
        union { float _angle;        float _left; };
        union { float _aspectRatio;  float _right; };
    public:
        BOOST_TYPE_INDEX_REGISTER_CLASS
        ComponentCamera(Entity&, float angle, float aspectRatio, float nearPlane, float farPlane);
        ComponentCamera(Entity&, float left, float right, float bottom, float top, float nearPlane, float farPlane);

        ComponentCamera(const ComponentCamera& other) = default;
        ComponentCamera& operator=(const ComponentCamera& other) = default;
        ComponentCamera(ComponentCamera&& other) noexcept = default;
        ComponentCamera& operator=(ComponentCamera&& other) noexcept = default;

        ~ComponentCamera();

        void resize(uint width, uint height);
        void lookAt(glm::vec3 eye, glm::vec3 forward, glm::vec3 up);

        glm::mat4 getProjection();
        glm::mat4 getProjectionInverse();
        glm::mat4 getView();
        glm::mat4 getViewInverse();
        glm::mat4 getViewProjection();
        glm::mat4 getViewProjectionInverse();
        glm::vec3 getViewVector();

        glm::vec3 forward();
        glm::vec3 right();
        glm::vec3 up();

        float getAngle();    void setAngle(float);
        float getAspect();   void setAspect(float);
        float getNear();     void setNear(float);
        float getFar();      void setFar(float);

        uint pointIntersectTest(glm::vec3& objectPosition);
        uint sphereIntersectTest(glm::vec3& objectPosition, float objectRadius);
};

class ComponentCamera_System : public Engine::epriv::ECSSystemCI {
    public:
        ComponentCamera_System();
        ~ComponentCamera_System() = default;
};

#endif