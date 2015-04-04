#ifndef BULLET_H
#define BULLET_H

#include <Bullet\btBulletCollisionCommon.h>
#include <Bullet\btBulletDynamicsCommon.h>

#include "Engine_Resources.h"

#include <Bullet\LinearMath\btIDebugDraw.h>
#include "GLDebugDrawer.h"

class Bullet{
	private:
		btBroadphaseInterface* m_broadphase;					  // Build the broadphase
		btDefaultCollisionConfiguration* m_collisionConfiguration;// collision configuration
		btCollisionDispatcher* m_dispatcher;					  // dispatcher
		btSequentialImpulseConstraintSolver* m_solver;		      // The actual physics solver
		btDiscreteDynamicsWorld* m_dynamicsWorld;				  // The world.

		GLDebugDrawer* m_debugDrawer;

	public:
		Bullet();
		~Bullet();

		void Set_Gravity(float,float,float); void Set_Gravity(glm::vec3);
		void Add_Rigid_Body(btRigidBody*);

		void Update(float dt);
		void Render();
};
extern Bullet* bullet;
#endif