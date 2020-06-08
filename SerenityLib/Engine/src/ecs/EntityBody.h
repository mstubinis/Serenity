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
    private:

    public:
        EntityBody() = default;
        EntityBody(Scene&);
        EntityBody(const unsigned int entityID, const unsigned int sceneID, const unsigned int versionID);
        ~EntityBody() = default;

        virtual float mass() const;
        virtual glm_quat getRotation() const;
        virtual glm_vec3 getScale() const;
        virtual glm_vec3 getPosition() const;
        virtual glm_vec3 getLocalPosition() const;
        virtual glm::vec3 getPositionRender() const;
        virtual float mass(const EntityDataRequest& request) const;
        virtual glm_quat getRotation(const EntityDataRequest& request) const;
        virtual glm_vec3 getScale(const EntityDataRequest& request) const;
        virtual glm_vec3 getPosition(const EntityDataRequest& request) const;
        virtual glm_vec3 getLocalPosition(const EntityDataRequest& request) const;
        virtual glm::vec3 getPositionRender(const EntityDataRequest& request) const;


        const glm_vec3& forward() const;
        const glm_vec3& right() const;
        const glm_vec3& up() const;
        const glm_vec3& forward(const EntityDataRequest& request) const;
        const glm_vec3& right(const EntityDataRequest& request) const;
        const glm_vec3& up(const EntityDataRequest& request) const;

        virtual glm_vec3 getLinearVelocity() const;
        virtual glm_vec3 getAngularVelocity() const;
        virtual glm_vec3 getLinearVelocity(const EntityDataRequest& request) const;
        virtual glm_vec3 getAngularVelocity(const EntityDataRequest& request) const;

        virtual void translate(const glm_vec3& translation, const bool local = true);
        virtual void translate(const decimal& x, const decimal& y, const decimal& z, const bool local = true);
        virtual void translate(const decimal& t, const bool local = true);
        virtual void translate(const EntityDataRequest& request, const glm_vec3& translation, const bool local = true);
        virtual void translate(const EntityDataRequest& request, const decimal& x, const decimal& y, const decimal& z, const bool local = true);
        virtual void translate(const EntityDataRequest& request, const decimal& t, const bool local = true);


        virtual void rotate(const glm_vec3& rotation, const bool local = true);
        virtual void rotate(const decimal& pitch_radians, const decimal& yaw_radians, const decimal& roll_radians, const bool local = true);
        virtual void rotate(const EntityDataRequest& request, const glm_vec3& rotation, const bool local = true);
        virtual void rotate(const EntityDataRequest& request, const decimal& pitch_radians, const decimal& yaw_radians, const decimal& roll_radians, const bool local = true);


        virtual void scale(const glm_vec3& amount);
        virtual void scale(const decimal& x, const decimal& y, const decimal& z);
        virtual void scale(const decimal& s);
        virtual void scale(const EntityDataRequest& request, const glm_vec3& amount);
        virtual void scale(const EntityDataRequest& request, const decimal& x, const decimal& y, const decimal& z);
        virtual void scale(const EntityDataRequest& request, const decimal& s);

        virtual void setPosition(const glm_vec3& newPosition);
        virtual void setPosition(const decimal& x, const decimal& y, const decimal& z);
        virtual void setPosition(const decimal& p);
        virtual void setPosition(const EntityDataRequest& request, const glm_vec3& newPosition);
        virtual void setPosition(const EntityDataRequest& request, const decimal& x, const decimal& y, const decimal& z);
        virtual void setPosition(const EntityDataRequest& request, const decimal& p);

        virtual void setRotation(const glm_quat& newRotation);
        virtual void setRotation(const decimal& quat_x, const decimal& quat_y, const decimal& quat_z, const decimal& quat_w);
        virtual void setRotation(const EntityDataRequest& request, const glm_quat& newRotation);
        virtual void setRotation(const EntityDataRequest& request, const decimal& quat_x, const decimal& quat_y, const decimal& quat_z, const decimal& quat_w);

        virtual void setScale(const glm_vec3& newScale);
        virtual void setScale(const decimal& x, const decimal& y, const decimal& z);
        virtual void setScale(const decimal& s);
        virtual void setScale(const EntityDataRequest& request, const glm_vec3& newScale);
        virtual void setScale(const EntityDataRequest& request, const decimal& x, const decimal& y, const decimal& z);
        virtual void setScale(const EntityDataRequest& request, const decimal& s);

        virtual void setDamping(const decimal& linear, const decimal& angular);

        virtual void setDynamic(const bool dynamic);
        virtual void setMass(const float mass);
        virtual void setGravity(const decimal& x, const decimal& y, const decimal& z);

        virtual void clearLinearForces();
        virtual void clearAngularForces();
        virtual void clearAllForces();

        virtual void setLinearVelocity(const decimal& x, const decimal& y, const decimal& z, const bool local = true);
        virtual void setLinearVelocity(const glm_vec3& velocity, const bool local = true);
        virtual void setLinearVelocity(const EntityDataRequest& request, const decimal& x, const decimal& y, const decimal& z, const bool local = true);
        virtual void setLinearVelocity(const EntityDataRequest& request, const glm_vec3& velocity, const bool local = true);

        virtual void setAngularVelocity(const decimal& x, const decimal& y, const decimal& z, const bool local = true);
        virtual void setAngularVelocity(const glm_vec3& velocity, const bool local = true);
        virtual void setAngularVelocity(const EntityDataRequest& request, const decimal& x, const decimal& y, const decimal& z, const bool local = true);
        virtual void setAngularVelocity(const EntityDataRequest& request, const glm_vec3& velocity, const bool local = true);

        virtual void applyForce(const decimal& x, const decimal& y, const decimal& z, const bool local = true);
        virtual void applyForce(const glm_vec3& force, const glm_vec3& origin = glm_vec3(0.0f), const bool local = true);
        virtual void applyForce(const EntityDataRequest& request, const decimal& x, const decimal& y, const decimal& z, const bool local = true);
        virtual void applyForce(const EntityDataRequest& request, const glm_vec3& force, const glm_vec3& origin = glm_vec3(0.0f), const bool local = true);

        virtual void applyImpulse(const decimal& x, const decimal& y, const decimal& z, const bool local = true);
        virtual void applyImpulse(const glm_vec3& impulse, const glm_vec3& origin = glm_vec3(0.0f), const bool local = true);
        virtual void applyImpulse(const EntityDataRequest& request, const decimal& x, const decimal& y, const decimal& z, const bool local = true);
        virtual void applyImpulse(const EntityDataRequest& request, const glm_vec3& impulse, const glm_vec3& origin = glm_vec3(0.0f), const bool local = true);

        virtual void applyTorque(const decimal& x, const decimal& y, const decimal& z, const bool local = true);
        virtual void applyTorque(const glm_vec3& torque, const bool local = true);
        virtual void applyTorque(const EntityDataRequest& request, const decimal& x, const decimal& y, const decimal& z, const bool local = true);
        virtual void applyTorque(const EntityDataRequest& request, const glm_vec3& torque, const bool local = true);

        virtual void applyTorqueImpulse(const decimal& x, const decimal& y, const decimal& z, const bool local = true);
        virtual void applyTorqueImpulse(const glm_vec3& torqueImpulse, const bool local = true);
        virtual void applyTorqueImpulse(const EntityDataRequest& request, const decimal& x, const decimal& y, const decimal& z, const bool local = true);
        virtual void applyTorqueImpulse(const EntityDataRequest& request, const glm_vec3& torqueImpulse, const bool local = true);
};

#endif