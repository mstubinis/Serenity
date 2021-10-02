#pragma once
#ifndef ENGINE_ECS_ENTITY_BODY_H
#define ENGINE_ECS_ENTITY_BODY_H

class btRigidBody;
class ComponentTransform;
class Scene;

#include <serenity/ecs/entity/Entity.h>
#include <serenity/physics/PhysicsIncludes.h>
#include <serenity/dependencies/glm.h>

class EntityBody : public Entity {
    public:
        EntityBody() = default;
        EntityBody(Scene& scene);
        EntityBody(const Entity& other);
        EntityBody(EntityID entityID, EntityID sceneID, EntityID versionID);

        EntityBody(const EntityBody&)                = default;
        EntityBody& operator=(const EntityBody&)     = default;
        EntityBody(EntityBody&&) noexcept            = default;
        EntityBody& operator=(EntityBody&&) noexcept = default;

        virtual ~EntityBody() = default;

        virtual [[nodiscard]] glm::quat getRotation() const;
        virtual [[nodiscard]] glm::quat getLocalRotation() const;
        virtual [[nodiscard]] glm::quat getWorldRotation() const;
        virtual [[nodiscard]] glm::vec3 getScale() const;
        virtual [[nodiscard]] glm_vec3 getPosition() const;
        virtual [[nodiscard]] glm_vec3 getWorldPosition() const;
        virtual [[nodiscard]] glm_vec3 getLocalPosition() const;

        [[nodiscard]] const glm::vec3& getForward() const;
        [[nodiscard]] const glm::vec3& getRight() const;
        [[nodiscard]] const glm::vec3& getUp() const;

        //virtual [[nodiscard]] glm_vec3 getLinearVelocity() const;

        virtual void translate(const glm_vec3& translation, bool local = true);
        virtual void translate(decimal x, decimal y, decimal z, bool local = true);
        virtual void translate(decimal t, bool local = true);

        virtual void rotate(const glm::vec3& rotation, bool local = true);
        virtual void rotate(float pitch_radians, float yaw_radians, float roll_radians, bool local = true);

        virtual void scale(const glm::vec3& amount);
        virtual void scale(float x, float y, float z);
        virtual void scale(float s);

        virtual void setPosition(const glm_vec3& newPosition);
        virtual void setPosition(decimal x, decimal y, decimal z);
        virtual void setPosition(decimal p);

        virtual void setRotation(const glm::quat& newRotation);
        virtual void setRotation(float quat_x, float quat_y, float quat_z, float quat_w);

        virtual void setScale(const glm::vec3& newScale);
        virtual void setScale(float x, float y, float z);
        virtual void setScale(float s);

        //virtual void setLinearVelocity(decimal x, decimal y, decimal z, bool local = true);
        //virtual void setLinearVelocity(const glm_vec3& velocity, bool local = true);
};

#endif