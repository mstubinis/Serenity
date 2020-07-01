#pragma once
#ifndef ENGINE_ECS_ENTITY_BODY_H
#define ENGINE_ECS_ENTITY_BODY_H

class Collision;
class btRigidBody;
class ComponentBody;
class Scene;

#include <ecs/Entity.h>
#include <core/engine/physics/PhysicsIncludes.h>

class EntityBody : public Entity {
    public:
        EntityBody() = default;
        EntityBody(Scene& scene);
        EntityBody(const Entity& other);
        EntityBody(std::uint32_t entityID, std::uint32_t sceneID, std::uint32_t versionID);
        virtual ~EntityBody() = default;

        virtual float mass() const;
        virtual glm_quat getRotation() const;
        virtual glm_vec3 getScale() const;
        virtual glm_vec3 getPosition() const;
        virtual glm_vec3 getLocalPosition() const;
        virtual glm::vec3 getPositionRender() const;

        const glm_vec3& forward() const;
        const glm_vec3& right() const;
        const glm_vec3& up() const;

        virtual glm_vec3 getLinearVelocity() const;
        virtual glm_vec3 getAngularVelocity() const;

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