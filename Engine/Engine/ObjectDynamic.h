#ifndef OBJECTDYNAMIC_H
#define OBJECTDYNAMIC_H

#include "ObjectDisplay.h"

class Collision;
class btRigidBody;
struct btDefaultMotionState;
class btVector3;

class ObjectDynamic: public ObjectDisplay{
	protected:
		float m_Mass;
		Collision* m_Collision;
		btRigidBody* m_RigidBody;
		btDefaultMotionState* m_MotionState;
	public:
		ObjectDynamic( std::string = "",
					   std::string = "",
					   glm::v3 = glm::v3(0),            //Position
					   glm::vec3 = glm::vec3(1),        //Scale
					   std::string = "Dynamic Object",  //Object
					   Collision* = nullptr,            //Bullet Collision Shape
					   Scene* = nullptr
					 );
		virtual ~ObjectDynamic();

		virtual void setPosition(glm::nType,glm::nType,glm::nType); 
		virtual void setPosition(glm::v3);

		void scale(float,float,float);
		void scale(glm::vec3);

		void rotate(float,float,float,bool overTime = true); 
		void rotate(glm::vec3, bool overTime = true);

		void translate(glm::nType,glm::nType,glm::nType,bool local=true); 
		void translate(glm::v3,bool local=true);

		void applyForce(float,float,float,bool local=true);
		void applyForce(glm::vec3,glm::vec3 = glm::vec3(0),bool local=true);
		void applyForceX(float,bool local=true);
		void applyForceY(float,bool local=true);
		void applyForceZ(float,bool local=true);

		void applyImpulse(float,float,float,bool local=true); 
		void applyImpulse(glm::vec3,glm::vec3 = glm::vec3(0),bool local=true);
		void applyImpulseX(float,bool local=true);
		void applyImpulseY(float,bool local=true);
		void applyImpulseZ(float,bool local=true);

		void applyTorque(float,float,float,bool local=true); 
		void applyTorque(glm::vec3,bool local=true);
		void applyTorqueX(float,bool local=true);
		void applyTorqueY(float,bool local=true);
		void applyTorqueZ(float,bool local=true);

		void applyTorqueImpulse(float,float,float,bool local=true); 
		void applyTorqueImpulse(glm::vec3,bool local=true);
		void applyTorqueImpulseX(float,bool local=true);
		void applyTorqueImpulseY(float,bool local=true);
		void applyTorqueImpulseZ(float,bool local=true);

		void setLinearVelocity(float,float,float,bool local=true); 
		void setLinearVelocity(glm::vec3,bool local=true);
		void setLinearVelocityX(float,bool local=true);
		void setLinearVelocityY(float,bool local=true);
		void setLinearVelocityZ(float,bool local=true);

		void setAngularVelocity(float,float,float,bool local=true); 
		void setAngularVelocity(glm::vec3,bool local=true);
		void setAngularVelocityX(float,bool local=true);
		void setAngularVelocityY(float,bool local=true);
		void setAngularVelocityZ(float,bool local=true);

		const glm::v3 getPosition();
		const float getMass() const { return m_Mass; }
		btRigidBody* getRigidBody() const { return m_RigidBody; }
		void setMass(float);

		void clearLinearForces();
		void clearAngularForces();
		void clearAllForces();

		void alignTo(glm::v3,float speed=0, bool overTime=false);

		void update(float);
};
#endif