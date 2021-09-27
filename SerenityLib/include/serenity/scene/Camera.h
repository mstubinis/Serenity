#pragma once
#ifndef ENGINE_CAMERA_H
#define ENGINE_CAMERA_H

class  Scene;

#include <serenity/ecs/components/Components.h>
#include <serenity/system/Macros.h>

enum class CameraType : uint8_t {
    Perspective, 
    Orthographic,
};
class CameraLogicType {
    public:
        enum Type : uint8_t {
            FPS_Mouselook,
            SpaceSimulator,
        };
        BUILD_ENUM_CLASS_MEMBERS(CameraLogicType, Type)
};

using CameraLogicComponent   = ComponentLogic2;
using CameraLogicFunctionPtr = void(*)(const CameraLogicComponent*, const float);

class Camera: public Entity {
    friend struct Engine::priv::ComponentCamera_Functions;
    public:
        glm::vec3  m_UserFloats = glm::vec3{ 0.0f };
    public:
        Camera(Scene*, float angle, float aspectRatio, float nearPlane, float farPlane);
        Camera(Scene*, float left, float right, float bottom, float top, float nearPlane, float farPlane);
        virtual ~Camera();

        void lookAt(const glm_vec3& eye, const glm_vec3& center, const glm_vec3& up) noexcept;

        [[nodiscard]] glm_vec3 getPosition() const noexcept;
        [[nodiscard]] glm::vec3 getRight() const noexcept;
        [[nodiscard]] glm::quat getRotation() const noexcept;

        [[nodiscard]] decimal getDistance(Entity) const noexcept;
        [[nodiscard]] decimal getDistance(const glm_vec3&) const noexcept;
        [[nodiscard]] decimal getDistanceSquared(Entity) const noexcept;
        [[nodiscard]] decimal getDistanceSquared(const glm_vec3&) const noexcept;
        [[nodiscard]] decimal getDistanceSquared(Entity, const glm_vec3& this_pos) const noexcept;
        [[nodiscard]] decimal getDistanceSquared(const glm_vec3& objPos, const glm_vec3& this_pos) const noexcept;

        [[nodiscard]] float getAngle() const noexcept;
        [[nodiscard]] float getAspectRatio() const noexcept;
        [[nodiscard]] float getNear() const noexcept;
        [[nodiscard]] float getFar() const noexcept;

        inline void setUpdateFunction(const CameraLogicFunctionPtr& func) noexcept { getComponent<CameraLogicComponent>()->setFunctor(func); };
        void setUpdateFunction(CameraLogicType) noexcept;

        void setAngle(float angle) noexcept;
        void setAspectRatio(float aspectRatio) noexcept;
        void setNear(float nearPlane) noexcept;
        void setFar(float farPlane) noexcept;

        void setProjectionMatrix(const glm::mat4&) noexcept;
        void setViewMatrix(const glm::mat4&) noexcept;

        [[nodiscard]] glm::mat4 getProjection() const noexcept;
        [[nodiscard]] glm::mat4 getProjectionInverse() const noexcept;
        [[nodiscard]] glm::vec3 getViewVector() const noexcept;

        [[nodiscard]] glm::mat4 getView() const noexcept;
        [[nodiscard]] glm::mat4 getViewInverse() const noexcept;
        [[nodiscard]] glm::mat4 getViewProjection() const noexcept;
        [[nodiscard]] glm::mat4 getViewProjectionInverse() const noexcept;

        [[nodiscard]] bool rayIntersectSphere(Entity) const noexcept;
        [[nodiscard]] bool rayIntersectSphere(const glm::vec3& position, float radius) const noexcept;
};
#endif
