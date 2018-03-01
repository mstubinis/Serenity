#pragma once
#ifndef ENGINE_OBJECTDYNAMIC_H
#define ENGINE_OBJECTDYNAMIC_H

#include "Object.h"
#include "MeshInstance.h"

struct Handle;

class Collision;
class btRigidBody;
struct btDefaultMotionState;
class btVector3;

/*
#define BIT(x) (1<<(x))
enum COLLISION_GROUPS {
    COLLISION_GROUP_NOTHING = 0,
    COLLISION_GROUP_EVERYTHING = BIT(0),
};
*/
class ObjectDynamic: public Object{
    protected:
        glm::vec3 m_Forward, m_Right, m_Up;
        bool m_Visible;
		bool m_PassedRenderCheck;
        std::vector<MeshInstance*> m_MeshInstances;
        glm::vec4 m_Color;
        glm::vec3 m_GodsRaysColor;
        glm::vec3 m_BoundingBoxRadius;
        virtual void calculateRadius();
        glm::mat4 m_Model;
        glm::vec3 _prevPosition;

        float m_Mass;
        Collision* m_Collision;
        btRigidBody* m_RigidBody;
        btDefaultMotionState* m_MotionState;
    public:
        virtual void collisionResponse(ObjectDynamic* other);

        ObjectDynamic( 
            Handle meshHandle,
            Handle materialHandle,
            glm::vec3 = glm::vec3(0),        //Position
            glm::vec3 = glm::vec3(1),        //Scale
            std::string = "Dynamic Object",  //Object
            Collision* = nullptr,            //Bullet Collision Shape
            Scene* = nullptr
        );
        ObjectDynamic( 
            Mesh*,
            Material*,
            glm::vec3 = glm::vec3(0),        //Position
            glm::vec3 = glm::vec3(1),        //Scale
            std::string = "Dynamic Object",  //Object
            Collision* = nullptr,            //Bullet Collision Shape
            Scene* = nullptr
        );
        virtual ~ObjectDynamic();

        virtual void setDynamic(bool=true);

        virtual void suspend();
        virtual void resume();

		bool passedRenderCheck(){return m_PassedRenderCheck;}
        void checkRender(Camera*);

        std::vector<MeshInstance*>&  getMeshInstances(){ return m_MeshInstances; }

        virtual void setPosition(float,float,float); 
        virtual void setPosition(glm::vec3);
        virtual void setScale(float,float,float); 
        virtual void setScale(glm::vec3);
        virtual void setColor(float,float,float,float);
        virtual void setColor(glm::vec4);

        virtual void lookAt(glm::vec3,glm::vec3,glm::vec3);
        virtual void lookAt(Object*);

        virtual void alignTo(glm::vec3,float speed=0);
        virtual void alignTo(Object*,float speed=0);
        virtual void alignToX(Object*,float speed=0);
        virtual void alignToY(Object*,float speed=0);
        virtual void alignToZ(Object*,float speed=0);

        virtual void scale(float,float,float);
        virtual void scale(glm::vec3);

        virtual void rotate(float,float,float,bool overTime = true); 
        virtual void rotate(glm::vec3, bool overTime = true);

        virtual void translate(float,float,float,bool local=true); 
        virtual void translate(glm::vec3,bool local=true);

        virtual void applyForce(float,float,float,bool local=true);
        virtual void applyForce(glm::vec3,glm::vec3 = glm::vec3(0),bool local=true);
        virtual void applyForceX(float,bool local=true);
        virtual void applyForceY(float,bool local=true);
        virtual void applyForceZ(float,bool local=true);

        virtual void applyImpulse(float,float,float,bool local=true); 
        virtual void applyImpulse(glm::vec3,glm::vec3 = glm::vec3(0),bool local=true);
        virtual void applyImpulseX(float,bool local=true);
        virtual void applyImpulseY(float,bool local=true);
        virtual void applyImpulseZ(float,bool local=true);

        virtual void applyTorque(float,float,float,bool local=true); 
        virtual void applyTorque(glm::vec3,bool local=true);
        virtual void applyTorqueX(float,bool local=true);
        virtual void applyTorqueY(float,bool local=true);
        virtual void applyTorqueZ(float,bool local=true);

        virtual void applyTorqueImpulse(float,float,float,bool local=true); 
        virtual void applyTorqueImpulse(glm::vec3,bool local=true);
        virtual void applyTorqueImpulseX(float,bool local=true);
        virtual void applyTorqueImpulseY(float,bool local=true);
        virtual void applyTorqueImpulseZ(float,bool local=true);

        virtual void setLinearVelocity(float,float,float,bool local=true); 
        virtual void setLinearVelocity(glm::vec3,bool local=true);
        virtual void setLinearVelocityX(float,bool local=true);
        virtual void setLinearVelocityY(float,bool local=true);
        virtual void setLinearVelocityZ(float,bool local=true);

        virtual void setAngularVelocity(float,float,float,bool local=true); 
        virtual void setAngularVelocity(glm::vec3,bool local=true);
        virtual void setAngularVelocityX(float,bool local=true);
        virtual void setAngularVelocityY(float,bool local=true);
        virtual void setAngularVelocityZ(float,bool local=true);

        virtual glm::vec4 getColor(){ return m_Color; }
        virtual glm::vec3 getGodsRaysColor(){ return m_GodsRaysColor; }
        virtual glm::vec3 getForward();
        virtual glm::vec3 getRight();
        virtual glm::vec3 getUp();
        virtual glm::vec3 getPosition();
        virtual float getMass(){ return m_Mass; }
        virtual btRigidBody* getRigidBody(){ return m_RigidBody; }
        virtual glm::quat& getOrientation();
        virtual void setOrientation(glm::quat q);
        virtual glm::vec3 getScale();
        virtual glm::mat4& getModel();
        virtual glm::vec3 getMotionVector(){ return getPosition() - _prevPosition; }
        virtual bool visible() { return m_Visible; }

        virtual void setMass(float);

        virtual void clearLinearForces();
        virtual void clearAngularForces();
        virtual void clearAllForces();

        void setMesh(Mesh*);
        void setMesh(Handle& meshHandle);

        void setMaterial(Material*);
        void setMaterial(Handle& materialHandle);

        virtual void update(float);

        virtual bool rayIntersectSphere(Camera* = nullptr);
        virtual bool rayIntersectSphere(glm::vec3 origin, glm::vec3 vector);
};
#endif
