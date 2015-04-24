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

PhysicsEngine::PhysicsEngine(){
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
PhysicsEngine::~PhysicsEngine(){
	delete m_debugDrawer;
	delete m_dynamicsWorld;
	delete m_solver;
	delete m_dispatcher;
	delete m_collisionConfiguration;
	delete m_broadphase;
}
void PhysicsEngine::setGravity(float x,float y,float z){ m_dynamicsWorld->setGravity(btVector3(x,y,z)); }
void PhysicsEngine::setGravity(glm::vec3 gravity){ setGravity(gravity.x,gravity.y,gravity.z); }
void PhysicsEngine::addRigidBody(btRigidBody* rigidBody){ m_dynamicsWorld->addRigidBody(rigidBody); }
void PhysicsEngine::update(float dt){
	m_dynamicsWorld->stepSimulation(1/60.0f);
}
void PhysicsEngine::render(){
	glm::mat4 model = glm::mat4(1.0);
	GLuint shader = Resources::getShader("Deferred")->getShaderProgram();
	glUseProgram(shader);
	glUniformMatrix4fv(glGetUniformLocation(shader, "VP"), 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(glGetUniformLocation(shader, "World"), 1, GL_FALSE, glm::value_ptr(model));
	m_dynamicsWorld->debugDrawWorld();
	glUseProgram(0);
}