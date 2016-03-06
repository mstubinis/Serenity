#ifndef OBJECTDYNAMIC_H
#define OBJECTDYNAMIC_H

#include "ObjectDisplay.h"

class Collision;
class btRigidBody;
struct btDefaultMotionState;
class btVector3;

class ObjectDynamic: public ObjectDisplay{
	private:
		glm::vec3 _calculateForward(); 
		glm::vec3 _calculateRight(); 
		glm::vec3 _calculateUp();
		float m_Mass;
	protected:
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

		void translate(glm::nType,glm::nType,glm::nType,bool local=true); 
		void translate(glm::v3,bool local=true);
		void applyForce(float,float,float,bool local=true); 
		void applyForce(glm::vec3,glm::vec3 = glm::vec3(0),bool local=true);
		void applyImpulse(float,float,float); 
		void applyImpulse(glm::vec3,glm::vec3 = glm::vec3(0));
		void applyTorque(float,float,float); 
		void applyTorque(glm::vec3);
		void applyTorqueImpulse(float,float,float); 
		void applyTorqueImpulse(glm::vec3);

		void setLinearVelocity(float,float,float,bool local=true); 
		void setLinearVelocity(glm::vec3,bool local=true);
		void setAngularVelocity(float,float,float); 
		void setAngularVelocity(glm::vec3);

		const float getMass() const { return m_Mass; }
		void setMass(float);

		void update(float);
};
#endif