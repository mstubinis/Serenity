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

        virtual void translate(const glm_vec3& translation, const bool local = true);
        virtual void translate(const decimal& x, const decimal& y, const decimal& z, const bool local = true);
        virtual void translate(const decimal& t, const bool local = true);


        virtual void rotate(const glm_vec3& rotation, const bool local = true);
        virtual void rotate(const decimal& pitch_radians, const decimal& yaw_radians, const decimal& roll_radians, const bool local = true);


        virtual void scale(const glm_vec3& amount);
        virtual void scale(const decimal& x, const decimal& y, const decimal& z);
        virtual void scale(const decimal& s);

        virtual void setPosition(const glm_vec3& newPosition);
        virtual void setPosition(const decimal& x, const decimal& y, const decimal& z);
        virtual void setPosition(const decimal& p);

        virtual void setRotation(const glm_quat& newRotation);
        virtual void setRotation(const decimal& quat_x, const decimal& quat_y, const decimal& quat_z, const decimal& quat_w);

        virtual void setScale(const glm_vec3& newScale);
        virtual void setScale(const decimal& x, const decimal& y, const decimal& z);
        virtual void setScale(const decimal& s);

        virtual void setDamping(const decimal& linear, const decimal& angular);

        virtual void setDynamic(const bool dynamic);
        virtual void setMass(const float mass);
        virtual void setGravity(const decimal& x, const decimal& y, const decimal& z);

        virtual void clearLinearForces();
        virtual void clearAngularForces();
        virtual void clearAllForces();

        virtual void setLinearVelocity(const decimal& x, const decimal& y, const decimal& z, const bool local = true);
        virtual void setLinearVelocity(const glm_vec3& velocity, const bool local = true);

        virtual void setAngularVelocity(const decimal& x, const decimal& y, const decimal& z, const bool local = true);
        virtual void setAngularVelocity(const glm_vec3& velocity, const bool local = true);

        virtual void applyForce(const decimal& x, const decimal& y, const decimal& z, const bool local = true);
        virtual void applyForce(const glm_vec3& force, const glm_vec3& origin = glm_vec3(0.0f), const bool local = true);

        virtual void applyImpulse(const decimal& x, const decimal& y, const decimal& z, const bool local = true);
        virtual void applyImpulse(const glm_vec3& impulse, const glm_vec3& origin = glm_vec3(0.0f), const bool local = true);

        virtual void applyTorque(const decimal& x, const decimal& y, const decimal& z, const bool local = true);
        virtual void applyTorque(const glm_vec3& torque, const bool local = true);

        virtual void applyTorqueImpulse(const decimal& x, const decimal& y, const decimal& z, const bool local = true);
        virtual void applyTorqueImpulse(const glm_vec3& torqueImpulse, const bool local = true);
};

#endif