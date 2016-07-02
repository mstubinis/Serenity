#pragma once
#ifndef ENGINE_OBJECTDYNAMIC_H
#define ENGINE_OBJECTDYNAMIC_H

#include "Object.h"

class Collision;
class btRigidBody;
struct btDefaultMotionState;
class btVector3;
struct DisplayItem;

/*
#define BIT(x) (1<<(x))
enum COLLISION_GROUPS {
    COLLISION_GROUP_NOTHING = 0,
    COLLISION_GROUP_EVERYTHING = BIT(0),
};
*/

class ObjectDynamic: public Object{
    protected:
        glm::v3 m_Forward, m_Right, m_Up;
        bool m_Visible;
        std::vector<DisplayItem*> m_DisplayItems;
        glm::vec4 m_Color;
		glm::vec3 m_GodsRaysColor;
        glm::vec3 m_BoundingBoxRadius;
        virtual void calculateRadius();
        glm::m4 m_Model;
        glm::v3 _prevPosition;

        float m_Mass;
        Collision* m_Collision;
        btRigidBody* m_RigidBody;
        btDefaultMotionState* m_MotionState;
    public:
        virtual void collisionResponse(ObjectDynamic* other);

        ObjectDynamic( std::string = "",
                       std::string = "",
                       glm::v3 = glm::v3(0),            //Position
                       glm::vec3 = glm::vec3(1),        //Scale
                       std::string = "Dynamic Object",  //Object
                       Collision* = nullptr,            //Bullet Collision Shape
                       Scene* = nullptr
                     );
        virtual ~ObjectDynamic();

        virtual void setDynamic(bool=true);

        std::vector<DisplayItem*>&  getDisplayItems(){ return m_DisplayItems; }

        virtual void setPosition(glm::num,glm::num,glm::num); 
        virtual void setPosition(glm::v3);
        virtual void setScale(float,float,float); 
        virtual void setScale(glm::vec3);
        virtual void setColor(float,float,float,float);
        virtual void setColor(glm::vec4);

        virtual void alignTo(glm::v3,float speed=0,bool overTime=false);

        virtual void scale(float,float,float);
        virtual void scale(glm::vec3);

        virtual void rotate(float,float,float,bool overTime = true); 
        virtual void rotate(glm::vec3, bool overTime = true);

        virtual void translate(glm::num,glm::num,glm::num,bool local=true); 
        virtual void translate(glm::v3,bool local=true);

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

        virtual glm::v3 getForward(){ return m_Forward; }
        virtual glm::v3 getRight(){ return m_Right; }
        virtual glm::v3 getUp(){ return m_Up; }
        virtual glm::v3 getPosition();
        virtual float getMass(){ return m_Mass; }
        virtual btRigidBody* getRigidBody(){ return m_RigidBody; }
        virtual glm::quat getOrientation();
        virtual void setOrientation(glm::quat);
        virtual glm::vec3 getScale();
        virtual glm::m4 getModel();
        virtual glm::v3 getMotionVector(){ return getPosition() - _prevPosition; }

        virtual void setMass(float);

        virtual void clearLinearForces();
        virtual void clearAngularForces();
        virtual void clearAllForces();

        virtual void update(float);
        virtual void render(GLuint=0,bool=false);
        virtual void draw(GLuint shader,bool=false,bool=false);

        virtual bool rayIntersectSphere(Camera*);
        virtual bool rayIntersectSphere(glm::v3 origin, glm::vec3 vector);
};
#endif