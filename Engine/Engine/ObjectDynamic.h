#ifndef OBJECTDYNAMIC_H
#define OBJECTDYNAMIC_H

#include "Object.h"

class btCollisionShape;
class btRigidBody;
struct btDefaultMotionState;
class btVector3;

class ObjectDynamic: public Object{
	private:
		glm::vec3 _Forward(); 
		glm::vec3 _Right(); 
		glm::vec3 _Up();
		float m_Mass;
	protected:
		btCollisionShape* m_Collision_Shape;
		btRigidBody* m_RigidBody;
		btDefaultMotionState* m_MotionState;
		btVector3* m_Inertia;
	public:
		ObjectDynamic( std::string = "",
					   std::string = "",
					   glm::vec3 = glm::vec3(0,0,0),   //Position
					   glm::vec3 = glm::vec3(1,1,1),   //Scale
					   std::string = "Dynamic Object", //Object
					   btCollisionShape* = nullptr     //Bullet Collision Shape
					 );
		~ObjectDynamic();

		virtual void Set_Position(float,float,float); virtual void Set_Position(glm::vec3&);

		void Translate(float,float,float,bool local = true); void Translate(glm::vec3&,bool local = true);
		void Apply_Force(float,float,float,bool local=true); void Apply_Force(glm::vec3&,glm::vec3& = glm::vec3(0,0,0),bool local=true);
		void Apply_Impulse(float,float,float); void Apply_Impulse(glm::vec3&,glm::vec3& = glm::vec3(0,0,0));
		void Apply_Torque(float,float,float); void Apply_Torque(glm::vec3&);
		void Apply_Torque_Impulse(float,float,float); void Apply_Torque_Impulse(glm::vec3&);

		void Set_Linear_Velocity(float,float,float,bool local=true); void Set_Linear_Velocity(glm::vec3&,bool local=true);
		void Set_Angular_Velocity(float,float,float); void Set_Angular_Velocity(glm::vec3&);

		void Set_Mass(float);

		void Update(float);

		const glm::vec3 Position() const;
		const float Mass() const { return m_Mass; }
};
#endif