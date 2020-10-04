#pragma once
#ifndef ENGINE_CAMERA_H
#define ENGINE_CAMERA_H

#include <ecs/Components.h>

class  Scene;
enum class CameraType : unsigned char {
    Perspective, 
    Orthographic,
};
class Camera: public Entity {
    friend struct Engine::priv::ComponentCamera_Functions;
    public:
        Camera(float angle, float aspectRatio, float nearPlane, float farPlane, Scene* = nullptr);
        Camera(float left, float right, float bottom, float top, float nearPlane, float farPlane, Scene* = nullptr);
        virtual ~Camera();

        void lookAt(const glm_vec3& eye, const glm_vec3& center, const glm_vec3& up) noexcept;

        glm_vec3 getPosition() const noexcept;
        glm_vec3 forward() const noexcept;
        glm_vec3 right() const noexcept;
        glm_vec3 up() const noexcept;
        glm::quat getOrientation() const noexcept;

        decimal getDistance(Entity) const noexcept;
        decimal getDistance(const glm_vec3&) const noexcept;
        decimal getDistanceSquared(Entity) const noexcept;
        decimal getDistanceSquared(const glm_vec3&) const noexcept;
        decimal getDistanceSquared(Entity e, const glm_vec3& this_pos) const noexcept;
        decimal getDistanceSquared(const glm_vec3& objPos, const glm_vec3& this_pos) const noexcept;

        float getAngle() const noexcept;    void setAngle(float angle) noexcept;
        float getAspect() const noexcept;   void setAspect(float aspectRatio) noexcept;
        float getNear() const noexcept;     void setNear(float nearPlane) noexcept;
        float getFar() const noexcept;      void setFar(float farPlane) noexcept;

        void setProjectionMatrix(const glm::mat4& perspectiveMatrix) noexcept;
        void setViewMatrix(const glm::mat4& viewMatrix) noexcept;

        glm::mat4 getProjection() const noexcept;
        glm::mat4 getProjectionInverse() const noexcept;
        glm::vec3 getViewVector() const noexcept;

        glm::mat4 getView() const noexcept;
        glm::mat4 getViewInverse() const noexcept;
        glm::mat4 getViewProjection() const noexcept;
        glm::mat4 getViewProjectionInverse() const noexcept;

        unsigned int sphereIntersectTest(const glm_vec3& pos, float radius) const noexcept;
        unsigned int pointIntersectTest(const glm_vec3& pos) const noexcept;

        bool rayIntersectSphere(Entity) const noexcept;
};
#endif
