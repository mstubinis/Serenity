#ifndef OBJECTDYNAMIC_H
#define OBJECTDYNAMIC_H

#include "Object.h"
#include "Bullet.h"

class ObjectDynamic: public Object{
	protected:
		glm::vec3 m_Velocity, m_Velocity_Rotation;
		float m_Friction, m_Mass, m_Inertia;

		btCollisionShape* m_Collision_Shape;
		btRigidBody* m_RigidBody;
		btDefaultMotionState* m_MotionState;
	public:
		ObjectDynamic( Mesh* = nullptr,
					   Material* = nullptr,
					   glm::vec3 = glm::vec3(0,0,0),   //Position
					   glm::vec3 = glm::vec3(1,1,1),   //Scale
					   glm::vec3 = glm::vec3(0,0,0),   //Rotation
					   std::string = "Dynamic Object", //Object
					   btCollisionShape* = nullptr     //Bullet Collision Shape
					 );
		~ObjectDynamic();

		glm::vec3 Forward(btQuaternion&) const;
		glm::vec3 Right(btQuaternion&) const;
		glm::vec3 Up(btQuaternion&) const;

		void Update(float);
		void Render(Mesh*,Material*,RENDER_TYPE);
};
#endif