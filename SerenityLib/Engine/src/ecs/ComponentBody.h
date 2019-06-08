#pragma once
#ifndef ENGINE_ECS_COMPONENT_BODY_H
#define ENGINE_ECS_COMPONENT_BODY_H

#include <core/engine/Engine_Physics.h>
#include <ecs/ComponentBaseClass.h>
#include <ecs/ECSSystem.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include <LinearMath/btDefaultMotionState.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>

#include <iostream>

class Collision;
class ComponentModel;

struct ScreenBoxCoordinates {
    bool inBounds;
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
    };
};

class ComponentBody : public ComponentBaseClass {
    friend struct Engine::epriv::ComponentBody_UpdateFunction;
    friend struct Engine::epriv::ComponentBody_ComponentAddedToEntityFunction;
    friend struct Engine::epriv::ComponentBody_EntityAddedToSceneFunction;
    friend struct Engine::epriv::ComponentBody_SceneEnteredFunction;
    friend struct Engine::epriv::ComponentBody_SceneLeftFunction;
    friend class  ::ComponentModel;
    private:
        struct PhysicsData {
            Collision* collision;
            btRigidBody* rigidBody;
            btDefaultMotionState motionState;
            float mass;

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
            NormalData* n;
            PhysicsData* p;
        } data;
        bool _physics;
        glm::vec3 _forward, _right, _up;
    public:
        BOOST_TYPE_INDEX_REGISTER_CLASS
        ComponentBody(Entity&);
        ComponentBody(Entity&, CollisionType::Type);

        ComponentBody& operator=(const ComponentBody& other);
        ComponentBody(const ComponentBody& other);
        ComponentBody(ComponentBody&& other) noexcept;
        ComponentBody& operator=(ComponentBody&& other) noexcept;

        ~ComponentBody();

        void alignTo(glm::vec3& direction, float speed);

        void translate(const glm::vec3& translation, bool local = true);
        void translate(float x, float y, float z, bool local = true);
        void translate(float t, bool local = true);
        void rotate(const glm::vec3& rotation, bool local = true);
        void rotate(float pitch, float yaw, float roll, bool local = true);
        void scale(const glm::vec3& amount);
        void scale(float x, float y, float z);
        inline void scale(float s);

        void setPosition(const glm::vec3& newPosition);
        void setPosition(float x, float y, float z);
        void setPosition(float p);
        void setRotation(const glm::quat& newRotation);
        void setRotation(float x, float y, float z, float w);
        void setScale(const glm::vec3& newScale);
        void setScale(float x, float y, float z);
        inline void setScale(float s);

        float mass();
        float getDistance(Entity& other);
        unsigned long long getDistanceLL(Entity& other);
        glm::vec3 getScreenCoordinates(bool clampToEdge = false);

        ScreenBoxCoordinates getScreenBoxCoordinates(float minOffset = 10.0f);

        glm::quat rotation();
        glm::vec3 getScale();
        glm::vec3 position();
        glm::vec3 forward();
        glm::vec3 right();
        glm::vec3 up();
        glm::vec3 getLinearVelocity();
        glm::vec3 getAngularVelocity();
        glm::mat4 modelMatrix();
        btRigidBody& getBody();

        void setCollision(CollisionType::Type, float mass);
        void setCollision(Collision*);
        void setDamping(float linear, float angular);

        void setDynamic(bool dynamic);
        void setMass(float mass);
        void setGravity(const float& x, const float& y, const float& z);

        void clearLinearForces();
        void clearAngularForces();
        void clearAllForces();

        void setLinearVelocity(float x, float y, float z, bool local = true);
        void setLinearVelocity(const glm::vec3& velocity, bool local = true);
        void setAngularVelocity(float x, float y, float z, bool local = true);
        void setAngularVelocity(const glm::vec3& velocity, bool local = true);
        void applyForce(float x, float y, float z, bool local = true);
        void applyForce(const glm::vec3& force, glm::vec3 origin = glm::vec3(0.0f), bool local = true);
        void applyImpulse(float x, float y, float z, bool local = true);
        void applyImpulse(const glm::vec3& impulse, glm::vec3 origin = glm::vec3(0.0f), bool local = true);
        void applyTorque(float x, float y, float z, bool local = true);
        void applyTorque(const glm::vec3& torque, bool local = true);
        void applyTorqueImpulse(float x, float y, float z, bool local = true);
        void applyTorqueImpulse(const glm::vec3& torqueImpulse, bool local = true);
};

class ComponentBody_System : public Engine::epriv::ECSSystemCI {
    public:
        ComponentBody_System();
        ~ComponentBody_System() = default;
};

#endif