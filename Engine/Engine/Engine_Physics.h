#ifndef ENGINE_PHYSICS_H
#define ENGINE_PHYSICS_H

#include <glm/glm.hpp>

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

		void setGravity(float,float,float); 
		void setGravity(glm::vec3);
		void addRigidBody(btRigidBody*);

		void update(float dt,unsigned int maxSteps = 1,float = 1/60.0f);
		void render();
};
extern PhysicsEngine* physicsEngine;
#endif