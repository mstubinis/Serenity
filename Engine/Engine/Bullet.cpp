#include "Bullet.h"
using namespace glm;

Bullet::Bullet(){
	m_broadphase = new btDbvtBroadphase();
	m_collisionConfiguration = new btDefaultCollisionConfiguration();
	m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);
	m_solver = new btSequentialImpulseConstraintSolver;
	m_dynamicsWorld = new btDiscreteDynamicsWorld(m_dispatcher,m_broadphase,m_solver,m_collisionConfiguration);
	m_dynamicsWorld->setGravity(btVector3(0,-1,0));

	m_debugDrawer = new GLDebugDrawer();
	m_debugDrawer->setDebugMode(btIDebugDraw::DBG_MAX_DEBUG_DRAW_MODE );
	m_dynamicsWorld->setDebugDrawer(m_debugDrawer);
}
Bullet::~Bullet(){
	delete m_debugDrawer;
	delete m_dynamicsWorld;
	delete m_solver;
	delete m_dispatcher;
	delete m_collisionConfiguration;
	delete m_broadphase;
}
void Bullet::Set_Gravity(float x,float y,float z){ m_dynamicsWorld->setGravity(btVector3(x,y,z)); }
void Bullet::Set_Gravity(vec3 gravity){ Set_Gravity(gravity.x,gravity.y,gravity.z); }
void Bullet::Add_Rigid_Body(btRigidBody* rigidBody){ m_dynamicsWorld->addRigidBody(rigidBody); }
void Bullet::Update(float dt){ m_dynamicsWorld->stepSimulation(dt); }
void Bullet::Render(){ m_dynamicsWorld->debugDrawWorld(); }