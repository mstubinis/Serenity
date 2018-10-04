#pragma once
#ifndef ENGINE_ECS_COMPONENT_BODY_H
#define ENGINE_ECS_COMPONENT_BODY_H

#include "../Engine_Physics.h"
#include "ComponentBaseClass.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

class Collision;
class btRigidBody;
struct btDefaultMotionState;

class ComponentModel;
class ComponentBody : public ComponentBaseClass {
    friend class ::ComponentModel;
    private:
        struct PhysicsData {
            Collision* collision;
            btRigidBody* rigidBody;
            btDefaultMotionState* motionState;
            float mass;
            PhysicsData() {
                collision = 0; rigidBody = 0; motionState = 0; mass = 0;
            }
        };
        struct NormalData {
            glm::vec3* scale;
            glm::vec3* position;
            glm::quat* rotation;
            glm::mat4* modelMatrix;
            NormalData() {
                scale = 0; position = 0; rotation = 0; modelMatrix = 0;
            }
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
        ComponentBody(Entity&,CollisionType::Type);
        ~ComponentBody();

        void alignTo(glm::vec3 direction, float speed);

        void translate(glm::vec3 translation, bool local = true);   void translate(float x, float y, float z, bool local = true);
        void rotate(glm::vec3 rotation, bool local = true);         void rotate(float pitch, float yaw, float roll, bool local = true);
        void scale(glm::vec3 amount);                               void scale(float x, float y, float z);

        void setPosition(glm::vec3 newPosition);                    void setPosition(float x, float y, float z);
        void setRotation(glm::quat newRotation);                    void setRotation(float x, float y, float z, float w);
        void setScale(glm::vec3 newScale);                          void setScale(float x, float y, float z);

        float mass();
        glm::vec3 getScreenCoordinates();
        glm::quat rotation();
        glm::vec3 getScale();
        glm::vec3 position();
        glm::vec3 forward();
        glm::vec3 right();
        glm::vec3 up();
        glm::vec3 getLinearVelocity();
        glm::vec3 getAngularVelocity();
        glm::mat4 modelMatrix();
        const btRigidBody* getBody() const;

        void setCollision(CollisionType::Type, float mass);
        void setDamping(float linear, float angular);

        void setDynamic(bool dynamic);
        void setMass(float mass);

        void clearLinearForces();
        void clearAngularForces();
        void clearAllForces();

        void setLinearVelocity(float x, float y, float z, bool local = true);     void setLinearVelocity(glm::vec3 velocity, bool local = true);
        void setAngularVelocity(float x, float y, float z, bool local = true);    void setAngularVelocity(glm::vec3 velocity, bool local = true);
        void applyForce(float x, float y, float z, bool local = true);            void applyForce(glm::vec3 force, glm::vec3 origin = glm::vec3(0.0f), bool local = true);
        void applyImpulse(float x, float y, float z, bool local = true);          void applyImpulse(glm::vec3 impulse, glm::vec3 origin = glm::vec3(0.0f), bool local = true);
        void applyTorque(float x, float y, float z, bool local = true);           void applyTorque(glm::vec3 torque, bool local = true);
        void applyTorqueImpulse(float x, float y, float z, bool local = true);    void applyTorqueImpulse(glm::vec3 torqueImpulse, bool local = true);
};

#endif