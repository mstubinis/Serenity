#pragma once
#ifndef ENGINE_CAMERA_H
#define ENGINE_CAMERA_H

#include <ecs/Components.h>
#include <ecs/EntityWrapper.h>

class  Scene;
struct CameraType final{enum Type {
    Perspective, 
    Orthographic,
};};
class Camera: public EntityWrapper{
    friend struct Engine::priv::ComponentCamera_Functions;
    public:
        Camera(const float angle, const float aspectRatio, const float nearPlane, const float farPlane,Scene* = nullptr);
        Camera(const float left, const float right, const float bottom, const float top, const float nearPlane, const float farPlane,Scene* = nullptr);
        virtual ~Camera();

        void lookAt(const glm_vec3& eye, const glm_vec3& center, const glm_vec3& up);

        const glm_vec3 getPosition() const;
        const glm_vec3 forward() const;
        const glm_vec3 right() const;
        const glm_vec3 up() const;
        const glm::quat getOrientation() const;

        const decimal getDistance(const Entity&) const;
        const decimal getDistance(const glm_vec3&) const;
        const decimal getDistanceSquared(const Entity&) const;
        const decimal getDistanceSquared(const glm_vec3&) const;
        const decimal getDistanceSquared(const Entity& e, const glm_vec3& this_pos) const;
        const decimal getDistanceSquared(const glm_vec3& objPos, const glm_vec3& this_pos) const;

        const float getAngle() const;    void setAngle(const float angle) const;
        const float getAspect() const;   void setAspect(const float aspectRatio) const;
        const float getNear() const;     void setNear(const float nearPlane) const;
        const float getFar() const;      void setFar(const float farPlane) const;

        void setProjectionMatrix(const glm::mat4& perspectiveMatrix);
        void setViewMatrix(const glm::mat4& viewMatrix);

        const glm::mat4 getProjection() const;
        const glm::mat4 getProjectionInverse() const;
        const glm::vec3 getViewVector() const;

        const glm::mat4 getView() const;
        const glm::mat4 getViewInverse() const;
        const glm::mat4 getViewProjection() const;
        const glm::mat4 getViewProjectionInverse() const;

        const unsigned int sphereIntersectTest(const glm_vec3& pos, const float radius) const;
        const unsigned int pointIntersectTest(const glm_vec3& pos) const;

        const bool rayIntersectSphere(const Entity&) const;
};
#endif
