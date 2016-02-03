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

enum COLLISION_TYPE { COLLISION_TYPE_CONVEXHULL, COLLISION_TYPE_TRIANGLESHAPE,COLLISION_TYPE_BOXSHAPE,COLLISION_TYPE_NONE };

namespace Engine{
	namespace Physics{
		struct Collision{
			public:
				unsigned int m_CollisionType;
				btCollisionShape* m_Collision;
				btVector3* m_Inertia;
				
				Collision(){ 
					m_Inertia = nullptr;
					m_Collision = nullptr;
					setCollision(nullptr,COLLISION_TYPE_NONE,0); 
				}
				Collision(btCollisionShape* shape,unsigned int type){ 
					m_Inertia = nullptr;
					m_Collision = nullptr;
					setCollision(shape,type,0); 
				}
				~Collision(){ 
					delete m_Inertia; 
					delete m_Collision; 
					m_CollisionType = COLLISION_TYPE_NONE; 
				}
				void recalculate(float mass){
					if(m_Collision != nullptr){
						if(m_CollisionType != COLLISION_TYPE_TRIANGLESHAPE){
							m_Collision->calculateLocalInertia(mass,*m_Inertia);
						}
						else{
							((btGImpactMeshShape*)m_Collision)->calculateLocalInertia(mass,*m_Inertia);
						}
					}
				}
				void setCollision(btCollisionShape* shape,unsigned int type, float mass){
					if(m_Inertia == nullptr){
						m_Inertia = new btVector3(0,0,0);
					}
					else{
						m_Inertia->setX(0);m_Inertia->setY(0);m_Inertia->setZ(0);
					}
					m_Collision = shape;
					m_CollisionType = type;
					if(shape != nullptr){
						if(mass != 0){
							if(type != COLLISION_TYPE_TRIANGLESHAPE){
								m_Collision->calculateLocalInertia(mass,*m_Inertia);
							}
							else{
								((btGImpactMeshShape*)m_Collision)->calculateLocalInertia(mass,*m_Inertia);
							}
						}
					}
				}
				btCollisionShape* getCollision() const { return m_Collision; }
				btVector3* getInertia() const { return m_Inertia; }
				const unsigned int getCollisionType() const { return m_CollisionType; }
		};
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

					static Collision* loadCollision(std::string filename, COLLISION_TYPE);
			};
		};
		void setGravity(float,float,float); 
		void setGravity(glm::vec3);
		void addRigidBody(btRigidBody*);
	};
};
#endif