#ifndef OBJECTDYNAMIC_H
#define OBJECTDYNAMIC_H

#include "Object.h"
#include "Bullet.h"

class ObjectDynamic: public Object{
	protected:
		glm::vec3 m_Rotation;

		btCollisionShape* m_Collision_Shape;
		btRigidBody* m_RigidBody;
		btDefaultMotionState* m_MotionState;
	public:
		ObjectDynamic( std::string = "",
					   std::string = "",
					   glm::vec3 = glm::vec3(0,0,0),   //Position
					   glm::vec3 = glm::vec3(1,1,1),   //Scale
					   glm::vec3 = glm::vec3(0,0,0),   //Rotation
					   std::string = "Dynamic Object", //Object
					   btCollisionShape* = nullptr     //Bullet Collision Shape
					 );
		~ObjectDynamic();

		glm::vec3 Forward(btQuaternion&) const; glm::vec3 Right(btQuaternion&) const; glm::vec3 Up(btQuaternion&) const;

		virtual void Set_Position(float,float,float); virtual void Set_Position(glm::vec3&);
		virtual void Translate(float,float,float); virtual void Translate(glm::vec3&);

		virtual void Rotate(float,float,float); virtual void Rotate(glm::vec3&);
		virtual void Pitch(float); virtual void Yaw(float); virtual void Roll(float);

		void Apply_Force(float,float,float,bool local=true); void Apply_Force(glm::vec3&,glm::vec3& = glm::vec3(0,0,0),bool local=true);
		void Apply_Impulse(float,float,float); void Apply_Impulse(glm::vec3&,glm::vec3& = glm::vec3(0,0,0));
		void Apply_Torque(float,float,float); void Apply_Torque(glm::vec3&);
		void Apply_Torque_Impulse(float,float,float); void Apply_Torque_Impulse(glm::vec3&);

		void Set_Linear_Velocity(float,float,float,bool local=true); void Set_Linear_Velocity(glm::vec3&,bool local=true);
		void Set_Angular_Velocity(float,float,float); void Set_Angular_Velocity(glm::vec3&);

		void Update(float);
		void Render(Mesh*,Material*,RENDER_TYPE);

		glm::vec3 Position(); 
		glm::vec3 Forward(); 
		glm::vec3 Right(); 
		glm::vec3 Up();
};
#endif