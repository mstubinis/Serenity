#ifndef ENGINE_PHYSICS_H
#define ENGINE_PHYSICS_H

#include <glm/glm.hpp>
#include <string>
#include <vector>

class btBroadphaseInterface;
class btDefaultCollisionConfiguration;
class btCollisionDispatcher;
class btSequentialImpulseConstraintSolver;
class btDiscreteDynamicsWorld;
class btRigidBody;
class GLDebugDrawer;

#include <bullet/btBulletCollisionCommon.h>
#include <bullet/BulletCollision/Gimpact/btGImpactShape.h>

enum COLLISION_TYPE { 
	COLLISION_TYPE_CONVEXHULL, 
	COLLISION_TYPE_TRIANGLESHAPE,
	COLLISION_TYPE_COMPOUND,
	COLLISION_TYPE_BOXSHAPE,
	COLLISION_TYPE_NONE
};
class MeshCollision final{
	private:
		unsigned int m_CollisionType;
		btCollisionShape* m_CollisionShape;
	public:
		MeshCollision(btCollisionShape* shape = nullptr,COLLISION_TYPE = COLLISION_TYPE_NONE);
		MeshCollision(std::string filename,COLLISION_TYPE);
		~MeshCollision();

		void load(std::string filename, COLLISION_TYPE);

		btCollisionShape* getCollisionShape() const { return m_CollisionShape; }
		const unsigned int getCollisionType() const { return m_CollisionType; }
};
class Collision final{
	private:
		MeshCollision* m_MeshCollision;
		btVector3* m_Inertia;
	public:
		Collision(MeshCollision*,float mass);
		~Collision();

		void recalculate(float mass);
		btVector3* getInertia() const { return m_Inertia; }
		MeshCollision* getMeshCollision() const { return m_MeshCollision; }
};

namespace Engine{
	namespace Physics{
		namespace Detail{
			class PhysicsManagement final{
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

					static std::vector<MeshCollision*> m_MeshCollisions;
			};
		};
		void setGravity(float,float,float); 
		void setGravity(glm::vec3);
		void addRigidBody(btRigidBody*);
	};
};
#endif