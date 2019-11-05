#pragma once
#ifndef ENGINE_ECS_COMPONENT_BODY_H
#define ENGINE_ECS_COMPONENT_BODY_H

#include <ecs/ComponentBaseClass.h>
#include <ecs/ECSSystem.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <LinearMath/btDefaultMotionState.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <core/engine/physics/Collision.h>

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <iostream>

class Collision;
class ComponentModel;
class ComponentBody;
struct ScreenBoxCoordinates {
    bool      inBounds;
    glm::vec2 topLeft;
    glm::vec2 topRight;
    glm::vec2 bottomLeft;
    glm::vec2 bottomRight;
};

namespace Engine {
    namespace epriv {
        struct ComponentBody_UpdateFunction;
        struct ComponentBody_EntityAddedToSceneFunction;
        struct ComponentBody_ComponentAddedToEntityFunction;
        struct ComponentBody_SceneEnteredFunction;
        struct ComponentBody_SceneLeftFunction;
        struct ComponentBody_EmptyCollisionFunctor final { 
            void operator()(ComponentBody& owner, const glm::vec3& ownerHit, ComponentBody& other, const glm::vec3& otherHit, const glm::vec3& normal) const {
            } 
        };
    };
};

class ComponentBody : public ComponentBaseClass {
    friend struct Engine::epriv::ComponentBody_UpdateFunction;
    friend struct Engine::epriv::ComponentBody_ComponentAddedToEntityFunction;
    friend struct Engine::epriv::ComponentBody_EntityAddedToSceneFunction;
    friend struct Engine::epriv::ComponentBody_SceneEnteredFunction;
    friend struct Engine::epriv::ComponentBody_SceneLeftFunction;
    friend class  ComponentModel;
    private:
        struct PhysicsData {
            Collision*           collision;
            btRigidBody*         bullet_rigidBody;
            btDefaultMotionState bullet_motionState;
            float                mass;
            ushort               group;
            ushort               mask;

            PhysicsData();
            PhysicsData(const PhysicsData& other)            = delete;
            PhysicsData& operator=(const PhysicsData& other) = delete;
            PhysicsData& operator=(PhysicsData&& other) noexcept;
            PhysicsData(PhysicsData&& other) noexcept;
            ~PhysicsData();         
        };
        struct NormalData {
            glm_vec3 scale;
            glm_vec3 position;
            glm_quat rotation;
            glm_mat4 modelMatrix;

            NormalData();
            NormalData(const NormalData& other)            = delete;
            NormalData& operator=(const NormalData& other) = delete;
            NormalData& operator=(NormalData&& other) noexcept;
            NormalData(NormalData&& other) noexcept;
            ~NormalData();
        };
        union {
            NormalData*  n;
            PhysicsData* p;
        } data;
        bool  m_Physics;
        void* m_UserPointer;
        void* m_UserPointer1;
        void* m_UserPointer2;
        glm_vec3 m_Forward, m_Right, m_Up, m_Goal, m_GoalVelocity;
        decimal m_GoalSpeed;

        boost::function<void(ComponentBody& owner, const glm::vec3& ownerHit, ComponentBody& other, const glm::vec3& otherHit, const glm::vec3& normal)> m_CollisionFunctor;

    public:
        BOOST_TYPE_INDEX_REGISTER_CLASS
        ComponentBody(const Entity&);
        ComponentBody(const Entity&, const CollisionType::Type);

        ComponentBody& operator=(const ComponentBody& other) = delete;
        ComponentBody(const ComponentBody& other)            = delete;
        ComponentBody(ComponentBody&& other) noexcept;
        ComponentBody& operator=(ComponentBody&& other) noexcept;

        ~ComponentBody();

        template<typename T> void setCollisionFunctor(const T& functor) {
            m_CollisionFunctor = boost::bind<void>(functor, _1, _2, _3, _4, _5);
        }
        void collisionResponse(ComponentBody& owner, const glm::vec3& ownerHit, ComponentBody& other, const glm::vec3& otherHit, const glm::vec3& normal);

        void rebuildRigidBody(const bool addBodyToPhysicsWorld = true);

        const glm_vec3& getGoal() const;
        const decimal& getGoalSpeed() const;
        void setGoal(const glm_vec3& _goal, const decimal& speed = 1.0f);
        void setGoal(const decimal& x, const decimal& y, const decimal& z, const decimal& speed = 1.0f);

        void setInternalPhysicsUserPointer(void* userPtr);
        void setUserPointer(void* userPtr);
        void setUserPointer1(void* userPtr);
        void setUserPointer2(void* userPtr);
        void* getUserPointer();
        void* getUserPointer1();
        void* getUserPointer2();

        const bool&  hasPhysics() const;
        const ushort getCollisionGroup() const; //get the groups this body belongs to
        const ushort getCollisionMask() const;  //get the groups this body will register collisions with
        const ushort getCollisionFlags() const;

        void alignTo(const glm_vec3& direction, const decimal speed);

        void translate(const glm_vec3& translation, const bool local = true);
        void translate(const decimal& x, const decimal& y, const decimal& z, const bool local = true);
        void translate(const decimal& t, const bool local = true);

        void rotate(const glm_vec3& rotation, const bool local = true);
        void rotate(const decimal& pitch, const decimal& yaw, const decimal& roll, const bool local = true);

        void scale(const glm_vec3& amount);
        void scale(const decimal& x, const decimal& y, const decimal& z);
        void scale(const decimal& s);

        void setPosition(const glm_vec3& newPosition);
        void setPosition(const decimal& x, const decimal& y, const decimal& z);
        void setPosition(const decimal& p);

        void setRotation(const glm_quat& newRotation);
        void setRotation(const decimal& x, const decimal& y, const decimal& z, const decimal& w);

        void setScale(const glm_vec3& newScale);
        void setScale(const decimal& x, const decimal& y, const decimal& z);
        void setScale(const decimal& s);

		const float mass() const;
        const decimal getDistance(const Entity& other);
        const unsigned long long getDistanceLL(const Entity& other);
        glm::vec3 getScreenCoordinates(const bool clampToEdge = false);

        ScreenBoxCoordinates getScreenBoxCoordinates(const float minOffset = 10.0f);

		const glm_quat rotation() const;
		const glm_vec3 getScale() const;
		const glm_vec3 position() const;
        const glm::vec3 position_render() const;
		const glm_vec3& forward() const;
		const glm_vec3& right() const;
		const glm_vec3& up() const;
		const glm_vec3 getLinearVelocity() const;
		const glm_vec3 getAngularVelocity() const;
		const glm_mat4 modelMatrix() const;
        const glm::mat4 modelMatrixRendering() const;
		const btRigidBody& getBtBody() const;

        void setCollision(const CollisionType::Type collisionType, const float mass);
        void setCollision(Collision* collision);
        Collision* getCollision();

        void setCollisionGroup(const short& group);  //set the groups this body belongs to
        void setCollisionMask(const short& mask); //set the groups this body will register collisions with
        void setCollisionFlag(const short& flag);
        void setCollisionGroup(const CollisionFilter::Filter& group);  //set the groups this body belongs to
        void setCollisionMask(const CollisionFilter::Filter& mask); //set the groups this body will register collisions with
        void setCollisionFlag(const CollisionFlag::Flag& flag);
        void addCollisionGroup(const short& group);  //add to the groups this body belongs to
        void addCollisionMask(const short& mask); //add to the groups this body will register collisions with
        void addCollisionFlag(const short& flag);
        void addCollisionGroup(const CollisionFilter::Filter& group); //add to the groups this body belongs to
        void addCollisionMask(const CollisionFilter::Filter& mask); //add to the groups this body will register collisions with
        void addCollisionFlag(const CollisionFlag::Flag& flag);
        void removeCollisionGroup(const short& group);
        void removeCollisionMask(const short& mask);
        void removeCollisionFlag(const short& flag);
        void removeCollisionGroup(const CollisionFilter::Filter& group);
        void removeCollisionMask(const CollisionFilter::Filter& mask);
        void removeCollisionFlag(const CollisionFlag::Flag& flag);

        void setDamping(const decimal& linear, const decimal& angular);

        void setDynamic(const bool dynamic);
        void setMass(const float mass);
        void setGravity(const decimal& x, const decimal& y, const decimal& z);

        void clearLinearForces();
        void clearAngularForces();
        void clearAllForces();

        void setLinearVelocity(const decimal& x, const decimal& y, const decimal& z, const bool local = true);
        void setLinearVelocity(const glm_vec3& velocity, const bool local = true);

        void setAngularVelocity(const decimal& x, const decimal& y, const decimal& z, const bool local = true);
        void setAngularVelocity(const glm_vec3& velocity, bool local = true);

        void applyForce(const decimal& x, const decimal& y, const decimal& z, const bool local = true);
        void applyForce(const glm_vec3& force, const glm_vec3& origin = glm_vec3(0.0f), bool local = true);

        void applyImpulse(const decimal& x, const decimal& y, const decimal& z, const bool local = true);
        void applyImpulse(const glm_vec3& impulse, const glm_vec3& origin = glm_vec3(0.0f), bool local = true);

        void applyTorque(const decimal& x, const decimal& y, const decimal& z, const bool local = true);
        void applyTorque(const glm_vec3& torque, const bool local = true);

        void applyTorqueImpulse(const decimal& x, const decimal& y, const decimal& z, const bool local = true);
        void applyTorqueImpulse(const glm_vec3& torqueImpulse, const bool local = true);
};

class ComponentBody_System : public Engine::epriv::ECSSystemCI {
    public:
        ComponentBody_System();
        ~ComponentBody_System() = default;
};

#endif