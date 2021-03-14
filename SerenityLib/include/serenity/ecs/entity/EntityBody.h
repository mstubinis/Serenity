#pragma once
#ifndef ENGINE_ECS_ENTITY_BODY_H
#define ENGINE_ECS_ENTITY_BODY_H

class Collision;
class btRigidBody;
class ComponentBody;
class Scene;

#include <serenity/ecs/entity/Entity.h>
#include <serenity/physics/PhysicsIncludes.h>
#include <serenity/dependencies/glm.h>

class EntityBody : public Entity {
    public:
        EntityBody() = default;
        EntityBody(Scene& scene);
        EntityBody(const Entity& other);
        EntityBody(uint32_t entityID, uint32_t sceneID, uint32_t versionID);

        EntityBody(const EntityBody&)                = default;
        EntityBody& operator=(const EntityBody&)     = default;
        EntityBody(EntityBody&&) noexcept            = default;
        EntityBody& operator=(EntityBody&&) noexcept = default;

        virtual ~EntityBody() = default;

        virtual [[nodiscard]] glm_quat getRotation() const;
        virtual [[nodiscard]] glm_vec3 getScale() const;
        virtual [[nodiscard]] glm_vec3 getPosition() const;
        virtual [[nodiscard]] glm_vec3 getLocalPosition() const;

        const [[nodiscard]]glm_vec3& forward() const;
        const [[nodiscard]]glm_vec3& right() const;
        const [[nodiscard]]glm_vec3& up() const;

        virtual [[nodiscard]] glm_vec3 getLinearVelocity() const;

        virtual void translate(const glm_vec3& translation, bool local = true);
        virtual void translate(decimal x, decimal y, decimal z, bool local = true);
        virtual void translate(decimal t, bool local = true);

        virtual void rotate(const glm_vec3& rotation, bool local = true);
        virtual void rotate(decimal pitch_radians, decimal yaw_radians, decimal roll_radians, bool local = true);

        virtual void scale(const glm_vec3& amount);
        virtual void scale(decimal x, decimal y, decimal z);
        virtual void scale(decimal s);

        virtual void setPosition(const glm_vec3& newPosition);
        virtual void setPosition(decimal x, decimal y, decimal z);
        virtual void setPosition(decimal p);

        virtual void setRotation(const glm_quat& newRotation);
        virtual void setRotation(decimal quat_x, decimal quat_y, decimal quat_z, decimal quat_w);

        virtual void setScale(const glm_vec3& newScale);
        virtual void setScale(decimal x, decimal y, decimal z);
        virtual void setScale(decimal s);

        virtual void setLinearVelocity(decimal x, decimal y, decimal z, bool local = true);
        virtual void setLinearVelocity(const glm_vec3& velocity, bool local = true);
};

#endif