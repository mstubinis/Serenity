#pragma once
#ifndef ENGINE_ECS_ENTITY_BODY_RIGID_H
#define ENGINE_ECS_ENTITY_BODY_RIGID_H

class btRigidBody;
class ComponentRigidBody;
class Scene;

#include <serenity/ecs/entity/Entity.h>
#include <serenity/physics/PhysicsIncludes.h>
#include <serenity/dependencies/glm.h>

class EntityBodyRigid : public Entity {
    public:
        EntityBodyRigid() = default;
        EntityBodyRigid(Scene& scene);
        EntityBodyRigid(const Entity& other);
        EntityBodyRigid(uint32_t entityID, uint32_t sceneID, uint32_t versionID);

        EntityBodyRigid(const EntityBodyRigid&)                = default;
        EntityBodyRigid& operator=(const EntityBodyRigid&)     = default;
        EntityBodyRigid(EntityBodyRigid&&) noexcept            = default;
        EntityBodyRigid& operator=(EntityBodyRigid&&) noexcept = default;

        virtual ~EntityBodyRigid() = default;

        virtual [[nodiscard]] glm::quat getRotation() const;
        virtual [[nodiscard]] glm::quat getLocalRotation() const;
        virtual [[nodiscard]] glm::quat getWorldRotation() const;
        virtual [[nodiscard]] glm::vec3 getScale() const;
        virtual [[nodiscard]] glm_vec3 getPosition() const;
        virtual [[nodiscard]] glm_vec3 getLocalPosition() const;
        virtual [[nodiscard]] glm_vec3 getWorldPosition() const;

        [[nodiscard]] const glm::vec3& getForward() const;
        [[nodiscard]] const glm::vec3& getRight() const;
        [[nodiscard]] const glm::vec3& getUp() const;

        virtual [[nodiscard]] glm_vec3 getLinearVelocity() const;
        virtual [[nodiscard]] glm_vec3 getAngularVelocity() const;

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

        virtual void setDamping(decimal linear, decimal angular);

        virtual void setDynamic(bool dynamic);
        virtual void setMass(float mass);
        virtual void setGravity(decimal x, decimal y, decimal z);

        virtual void clearLinearForces();
        virtual void clearAngularForces();
        virtual void clearAllForces();

        virtual void setLinearVelocity(decimal x, decimal y, decimal z, bool local = true);
        virtual void setLinearVelocity(const glm_vec3& velocity, bool local = true);

        virtual void setAngularVelocity(decimal x, decimal y, decimal z, bool local = true);
        virtual void setAngularVelocity(const glm_vec3& velocity, bool local = true);

        virtual void applyForce(decimal x, decimal y, decimal z, bool local = true);
        virtual void applyForce(const glm_vec3& force, const glm_vec3& origin = glm_vec3(0.0f), const bool local = true);

        virtual void applyImpulse(decimal x, decimal y, decimal z, bool local = true);
        virtual void applyImpulse(const glm_vec3& impulse, const glm_vec3& origin = glm_vec3(0.0f), bool local = true);

        virtual void applyTorque(decimal x, decimal y, decimal z, bool local = true);
        virtual void applyTorque(const glm_vec3& torque, bool local = true);

        virtual void applyTorqueImpulse(decimal x, decimal y, decimal z, bool local = true);
        virtual void applyTorqueImpulse(const glm_vec3& torqueImpulse, bool local = true);
};

#endif