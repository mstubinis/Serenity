#pragma once
#ifndef ENGINE_ECS_COMPONENT_CAMERA_H
#define ENGINE_ECS_COMPONENT_CAMERA_H

#include "ComponentBaseClass.h"
#include "ECSComponentSystem.h"

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

/*

class Camera;
class ComponentCamera;
namespace Engine {
    namespace epriv {
        class ComponentCameraSystem;
    };
};

class ComponentCamera : public ComponentBaseClass {
    friend class ::Camera;
    friend class ::Engine::epriv::ComponentCameraSystem;
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
        ComponentCamera();
        ComponentCamera(float angle, float aspectRatio, float nearPlane, float farPlane);
        ComponentCamera(float left, float right, float bottom, float top, float nearPlane, float farPlane);
        ~ComponentCamera();

        virtual void update(const float& dt);
        void resize(uint width, uint height);
        void lookAt(glm::vec3 eye, glm::vec3 forward, glm::vec3 up);

        glm::mat4 getProjection();
        glm::mat4 getProjectionInverse();
        glm::mat4 getView();
        glm::mat4 getViewInverse();
        glm::mat4 getViewProjection();
        glm::mat4 getViewProjectionInverse();
        glm::vec3 getViewVector();

        float getAngle();    void setAngle(float);
        float getAspect();   void setAspect(float);
        float getNear();     void setNear(float);
        float getFar();      void setFar(float);

        uint pointIntersectTest(glm::vec3& objectPosition);
        uint sphereIntersectTest(glm::vec3& objectPosition, float objectRadius);
};

namespace Engine {
    namespace epriv {
        class ComponentCameraSystem final : public ECSComponentSystem<ComponentCamera> {
            friend class ::Camera;
            friend class ::ComponentCamera;
            private:
                static void RebuildProjectionMatrix(ComponentCamera&);
            public:
                ComponentCameraSystem();
                ~ComponentCameraSystem();

                void update(const float& dt);
        };
    };
};

*/

#endif