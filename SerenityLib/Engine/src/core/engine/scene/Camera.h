#pragma once
#ifndef ENGINE_CAMERA_H
#define ENGINE_CAMERA_H

#include <ecs/Components.h>

class  Scene;
struct CameraType final{enum Type {
    Perspective, 
    Orthographic,
};};
class Camera: public EntityWrapper{
    friend struct Engine::epriv::ComponentCamera_Functions;
    public:
        Camera(const float angle, const float aspectRatio, const float nearPlane, const float farPlane,Scene* = nullptr);
        Camera(const float left, const float right, const float bottom, const float top, const float nearPlane, const float farPlane,Scene* = nullptr);
        virtual ~Camera();

        void lookAt(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up);

        const glm::vec3 getPosition();
        const glm::vec3 forward();
        const glm::vec3 right();
        const glm::vec3 up();
        const glm::quat getOrientation();

        const float getDistance(Entity&);
        const float getDistance(const glm::vec3&);
        const float getDistanceSquared(Entity&);
        const float getDistanceSquared(const glm::vec3&);

        const float& getAngle();    void setAngle(const float angle);
        const float& getAspect();   void setAspect(const float aspectRatio);
        const float& getNear();     void setNear(const float nearPlane);
        const float& getFar();      void setFar(const float farPlane);

        const glm::mat4 getProjection();
        const glm::mat4 getProjectionInverse();
        const glm::vec3 getViewVector();

        const glm::mat4 getView();
        const glm::mat4 getViewInverse();
        const glm::mat4 getViewProjection();
        const glm::mat4 getViewProjectionInverse();

        const uint sphereIntersectTest(const glm::vec3& pos, const float radius);
        const uint pointIntersectTest(const glm::vec3& pos);

        const bool rayIntersectSphere(Entity&);
};
#endif
