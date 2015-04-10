#ifndef ENGINE_PHYSICS_H
#define ENGINE_PHYSICS_H

#include <glm\glm.hpp>

class btBroadphaseInterface;
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;
class btRigidBody;
class GLDebugDrawer;

class PhysicsEngine{
	private:
		btBroadphaseInterface* m_broadphase;					  // Build the broadphase
		btDefaultCollisionConfiguration* m_collisionConfiguration;// collision configuration
		btCollisionDispatcher* m_dispatcher;					  // dispatcher
		btSequentialImpulseConstraintSolver* m_solver;		      // The actual physics solver
		btDiscreteDynamicsWorld* m_dynamicsWorld;				  // The world.

		GLDebugDrawer* m_debugDrawer;

	public:
		PhysicsEngine();
		~PhysicsEngine();

		void Set_Gravity(float,float,float); void Set_Gravity(glm::vec3);
		void Add_Rigid_Body(btRigidBody*);

		void Update(float dt);
		void Render();
};
extern PhysicsEngine* physicsEngine;
#endif