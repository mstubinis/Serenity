#ifndef OBJECTDYNAMIC_H
#define OBJECTDYNAMIC_H

#include "Object.h"
#include "Bullet.h"

class ObjectDynamic: public Object{
	protected:
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

		void Set_Position(float,float,float); void Set_Position(glm::vec3);
		void Translate(float,float,float); void Translate(glm::vec3);

		void Rotate(float,float,float); void Rotate(glm::vec3);
		void Pitch(float); void Yaw(float); void Roll(float);

		void Update(float);
		void Render(Mesh*,Material*,RENDER_TYPE);
};
#endif