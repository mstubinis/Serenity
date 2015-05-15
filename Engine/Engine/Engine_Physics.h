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

namespace Engine{
	namespace Physics{
		namespace Detail{
			class PhysicsManagement{
				private:
					static btBroadphaseInterface* m_broadphase;
					static btDefaultCollisionConfiguration* m_collisionConfiguration;
					static btCollisionDispatcher* m_dispatcher;
					static btSequentialImpulseConstraintSolver* m_solver;
					static btDiscreteDynamicsWorld* m_dynamicsWorld;

					static GLDebugDrawer* m_debugDrawer;
				public:
					static void _addRigidBody(btRigidBody*);
					static void _setGravity(float,float,float);

					static void init();
					static void destruct();
					static void update(float dt,unsigned int maxSteps = 1,float = 1/60.0f);
					static void render();
			};
		};
		void setGravity(float,float,float); 
		void setGravity(glm::vec3);
		void addRigidBody(btRigidBody*);
	};
};
#endif