#pragma once
#ifndef ENGINE_CAMERA_H
#define ENGINE_CAMERA_H

#include <ecs/Components.h>

class  Scene;
struct CameraType final{enum Type {
    Perspective, 
    Orthographic,
};};
class Camera: public Entity{
    friend struct Engine::priv::ComponentCamera_Functions;
    public:
        Camera(const float angle, const float aspectRatio, const float nearPlane, const float farPlane,Scene* = nullptr);
        Camera(const float left, const float right, const float bottom, const float top, const float nearPlane, const float farPlane,Scene* = nullptr);
        virtual ~Camera();

        void lookAt(const glm_vec3& eye, const glm_vec3& center, const glm_vec3& up);

        glm_vec3 getPosition() const;
        glm_vec3 forward() const;
        glm_vec3 right() const;
        glm_vec3 up() const;
        glm::quat getOrientation() const;

        decimal getDistance(const Entity&) const;
        decimal getDistance(const glm_vec3&) const;
        decimal getDistanceSquared(const Entity&) const;
        decimal getDistanceSquared(const glm_vec3&) const;
        decimal getDistanceSquared(const Entity& e, const glm_vec3& this_pos) const;
        decimal getDistanceSquared(const glm_vec3& objPos, const glm_vec3& this_pos) const;

        float getAngle() const;    void setAngle(const float angle);
        float getAspect() const;   void setAspect(const float aspectRatio);
        float getNear() const;     void setNear(const float nearPlane);
        float getFar() const;      void setFar(const float farPlane);

        void setProjectionMatrix(const glm::mat4& perspectiveMatrix);
        void setViewMatrix(const glm::mat4& viewMatrix);

        glm::mat4 getProjection() const;
        glm::mat4 getProjectionInverse() const;
        glm::vec3 getViewVector() const;

        glm::mat4 getView() const;
        glm::mat4 getViewInverse() const;
        glm::mat4 getViewProjection() const;
        glm::mat4 getViewProjectionInverse() const;

        unsigned int sphereIntersectTest(const glm_vec3& pos, const float radius) const;
        unsigned int pointIntersectTest(const glm_vec3& pos) const;

        bool rayIntersectSphere(const Entity&) const;
};
#endif
