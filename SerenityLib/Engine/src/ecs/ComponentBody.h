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
            PhysicsData(const PhysicsData& other);
            PhysicsData& operator=(const PhysicsData& other);
            PhysicsData& operator=(PhysicsData&& other) noexcept;
            PhysicsData(PhysicsData&& other) noexcept;
            ~PhysicsData();         
        };
        struct NormalData {
            glm::vec3 scale;
            glm::vec3 position;
            glm::quat rotation;
            glm::mat4 modelMatrix;

            NormalData();
            NormalData(const NormalData& other);
            NormalData& operator=(const NormalData& other);
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
        glm::vec3 m_Forward, m_Right, m_Up;

        boost::function<void(ComponentBody& owner, const glm::vec3& ownerHit, ComponentBody& other, const glm::vec3& otherHit, const glm::vec3& normal)> m_CollisionFunctor;

    public:
        BOOST_TYPE_INDEX_REGISTER_CLASS
        ComponentBody(const Entity&);
        ComponentBody(const Entity&, const CollisionType::Type);

        ComponentBody& operator=(const ComponentBody& other);
        ComponentBody(const ComponentBody& other);
        ComponentBody(ComponentBody&& other) noexcept;
        ComponentBody& operator=(ComponentBody&& other) noexcept;

        ~ComponentBody();

        template<typename T> void setCollisionFunctor(const T& functor) {
            m_CollisionFunctor = boost::bind<void>(functor, _1, _2, _3, _4, _5);
        }
        void collisionResponse(ComponentBody& owner, const glm::vec3& ownerHit, ComponentBody& other, const glm::vec3& otherHit, const glm::vec3& normal);

        void setInternalPhysicsUserPointer(void* userPtr);
        void setUserPointer(void* userPtr);
        void setUserPointer1(void* userPtr);
        void setUserPointer2(void* userPtr);
        void* getUserPointer();
        void* getUserPointer1();
        void* getUserPointer2();

        const ushort getCollisionGroup() const; //get the groups this body belongs to
        const ushort getCollisionMask() const;  //get the groups this body will register collisions with
        const ushort getCollisionFlags() const;

        void alignTo(const glm::vec3& direction, const float speed);

        void translate(const glm::vec3& translation, const bool local = true);
        void translate(const float x, const float y, const float z, const bool local = true);
        void translate(const float t, const bool local = true);

        void rotate(const glm::vec3& rotation, const bool local = true);
        void rotate(const float pitch, const float yaw, const float roll, const bool local = true);
		void rotate(const double pitch, const double yaw, const double roll, const bool local = true);

        void scale(const glm::vec3& amount);
        void scale(const float x, const float y, const float z);
        void scale(const float s);

        void setPosition(const glm::vec3& newPosition);
        void setPosition(const float x, const float y, const float z);
        void setPosition(const float p);

        void setRotation(const glm::quat& newRotation);
        void setRotation(const float x, const float y, const float z, const float w);

        void setScale(const glm::vec3& newScale);
        void setScale(const float x, const float y, const float z);
        void setScale(const float s);

		const float mass() const;
        float getDistance(const Entity& other);
        unsigned long long getDistanceLL(const Entity& other);
        glm::vec3 getScreenCoordinates(const bool clampToEdge = false);

        ScreenBoxCoordinates getScreenBoxCoordinates(const float minOffset = 10.0f);

		const glm::quat rotation() const;
		const glm::vec3 getScale() const;
		const glm::vec3 position() const;
        const glm::vec3 position_render() const;
		const glm::vec3 forward() const;
		const glm::vec3 right() const;
		const glm::vec3 up() const;
		const glm::vec3 getLinearVelocity() const;
		const glm::vec3 getAngularVelocity() const;
		const glm::mat4 modelMatrix() const;
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

        void setDamping(const float linear, const float angular);

        void setDynamic(const bool dynamic);
        void setMass(const float mass);
        void setGravity(const float& x, const float& y, const float& z);

        void clearLinearForces();
        void clearAngularForces();
        void clearAllForces();

        void setLinearVelocity(const float x, const float y, const float z, const bool local = true);
		void setLinearVelocity(const double x, const double y, const double z, const bool local = true);
        void setLinearVelocity(const glm::vec3& velocity, const bool local = true);

        void setAngularVelocity(const float x, const float y, const float z, const bool local = true);
		void setAngularVelocity(const double x, const double y, const double z, const bool local = true);
        void setAngularVelocity(const glm::vec3& velocity, bool local = true);

        void applyForce(const float x, const float y, const float z, const bool local = true);
		void applyForce(const double x, const double y, const double z, const bool local = true);
        void applyForce(const glm::vec3& force, const glm::vec3& origin = glm::vec3(0.0f), bool local = true);

        void applyImpulse(const float x, const float y, const float z, const bool local = true);
		void applyImpulse(const double x, const double y, const double z, const bool local = true);
        void applyImpulse(const glm::vec3& impulse, const glm::vec3& origin = glm::vec3(0.0f), bool local = true);

        void applyTorque(const float x, const float y, const float z, const bool local = true);
		void applyTorque(const double x, const double y, const double z, const bool local = true);
        void applyTorque(const glm::vec3& torque, const bool local = true);

        void applyTorqueImpulse(const float x, const float y, const float z, const bool local = true);
		void applyTorqueImpulse(const double x, const double y, const double z, const bool local = true);
        void applyTorqueImpulse(const glm::vec3& torqueImpulse, const bool local = true);
};

class ComponentBody_System : public Engine::epriv::ECSSystemCI {
    public:
        ComponentBody_System();
        ~ComponentBody_System() = default;
};

#endif