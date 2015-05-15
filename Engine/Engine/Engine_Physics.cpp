#include "Engine_Physics.h"

#include <bullet/btBulletCollisionCommon.h>
#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/LinearMath/btIDebugDraw.h>
#include "GLDebugDrawer.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Engine_Resources.h"
#include "ShaderProgram.h"
#include "Camera.h"

using namespace Engine;

btBroadphaseInterface* Engine::Physics::Detail::PhysicsManagement::m_broadphase = nullptr;
btDefaultCollisionConfiguration* Engine::Physics::Detail::PhysicsManagement::m_collisionConfiguration = nullptr;
btCollisionDispatcher* Engine::Physics::Detail::PhysicsManagement::m_dispatcher = nullptr;
btSequentialImpulseConstraintSolver* Engine::Physics::Detail::PhysicsManagement::m_solver = nullptr;
btDiscreteDynamicsWorld* Engine::Physics::Detail::PhysicsManagement::m_dynamicsWorld = nullptr;
GLDebugDrawer* Engine::Physics::Detail::PhysicsManagement::m_debugDrawer = nullptr;

void Engine::Physics::Detail::PhysicsManagement::init(){
	m_broadphase = new btDbvtBroadphase();
	m_collisionConfiguration = new btDefaultCollisionConfiguration();
	m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);
	m_solver = new btSequentialImpulseConstraintSolver;
	m_dynamicsWorld = new btDiscreteDynamicsWorld(m_dispatcher,m_broadphase,m_solver,m_collisionConfiguration);

	m_debugDrawer = new GLDebugDrawer();
	m_debugDrawer->setDebugMode(btIDebugDraw::DBG_MAX_DEBUG_DRAW_MODE );
	m_dynamicsWorld->setDebugDrawer(m_debugDrawer);

	m_dynamicsWorld->setGravity(btVector3(0,0,0));
}
void Engine::Physics::Detail::PhysicsManagement::destruct(){
	delete m_debugDrawer;
	delete m_dynamicsWorld;
	delete m_solver;
	delete m_dispatcher;
	delete m_collisionConfiguration;
	delete m_broadphase;
}
void Engine::Physics::Detail::PhysicsManagement::_setGravity(float x, float y, float z){ 
	Engine::Physics::Detail::PhysicsManagement::m_dynamicsWorld->setGravity(btVector3(x,y,z)); 
}
void Engine::Physics::Detail::PhysicsManagement::_addRigidBody(btRigidBody* rigidBody){ 
	Engine::Physics::Detail::PhysicsManagement::m_dynamicsWorld->addRigidBody(rigidBody); 
}

void Engine::Physics::setGravity(float x,float y,float z){ 
	Engine::Physics::Detail::PhysicsManagement::_setGravity(x,y,z);
}
void Engine::Physics::setGravity(glm::vec3 gravity){ setGravity(gravity.x,gravity.y,gravity.z); }
void Engine::Physics::addRigidBody(btRigidBody* rigidBody){ 
	Engine::Physics::Detail::PhysicsManagement::_addRigidBody(rigidBody);
}
void Engine::Physics::Detail::PhysicsManagement::update(float dt,unsigned int maxSteps,float other){
	m_dynamicsWorld->stepSimulation(dt,maxSteps,other);
}
void Engine::Physics::Detail::PhysicsManagement::render(){
	glm::mat4 model = glm::mat4(1.0);
	GLuint shader = Resources::getShader("Deferred")->getShaderProgram();
	glUseProgram(shader);
	glUniformMatrix4fv(glGetUniformLocation(shader, "VP"), 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(glGetUniformLocation(shader, "World"), 1, GL_FALSE, glm::value_ptr(model));
	m_dynamicsWorld->debugDrawWorld();
	glUseProgram(0);
}