#pragma once
#ifndef GAME_COMPONENT_GAME_CAMERA_H
#define GAME_COMPONENT_GAME_CAMERA_H

#include "ecs/ComponentBaseClass.h"
#include "ecs/ECSSystem.h"

#include <glm/vec3.hpp>
#include <glm/vec4.hpp>
#include <glm/mat4x4.hpp>

class Camera;
class ComponentModel;

namespace Engine {
    namespace epriv {
        struct ComponentGameCameraUpdateFunction;
        struct ComponentGameCameraEntityAddedToSceneFunction;
        struct ComponentGameCameraComponentAddedToEntityFunction;
        struct ComponentGameCameraSceneEnteredFunction;
        struct ComponentGameCameraSceneLeftFunction;
        struct ComponentGameCameraFunctions;
    };
};

class ComponentGameCamera : public ComponentBaseClass {
    friend struct Engine::epriv::ComponentGameCameraUpdateFunction;
    friend struct Engine::epriv::ComponentGameCameraEntityAddedToSceneFunction;
    friend struct Engine::epriv::ComponentGameCameraComponentAddedToEntityFunction;
    friend struct Engine::epriv::ComponentGameCameraSceneEnteredFunction;
    friend struct Engine::epriv::ComponentGameCameraSceneLeftFunction;
    friend struct Engine::epriv::ComponentGameCameraFunctions;
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
        ComponentGameCamera(Entity&, float angle, float aspectRatio, float nearPlane, float farPlane);
        ComponentGameCamera(Entity&, float left, float right, float bottom, float top, float nearPlane, float farPlane);

        ComponentGameCamera& operator=(const ComponentGameCamera& other) noexcept = default;
        ComponentGameCamera(const ComponentGameCamera& other) noexcept = default;
        ComponentGameCamera(ComponentGameCamera&& other) noexcept = default;
        ComponentGameCamera& operator=(ComponentGameCamera&& other) noexcept = default;

        ~ComponentGameCamera();

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

class ComponentGameCameraSystem : public Engine::epriv::ECSSystemCI {
    public:
        ComponentGameCameraSystem();
        ~ComponentGameCameraSystem() = default;
};

#endif