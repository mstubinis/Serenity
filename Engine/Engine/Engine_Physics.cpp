#include "Engine_Physics.h"

#include <Bullet/btBulletCollisionCommon.h>
#include <Bullet/btBulletDynamicsCommon.h>
#include <Bullet/LinearMath/btIDebugDraw.h>
#include "GLDebugDrawer.h"

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Engine_Resources.h"
#include "ShaderProgram.h"
#include "Camera.h"

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
void PhysicsEngine::Set_Gravity(float x,float y,float z){ m_dynamicsWorld->setGravity(btVector3(x,y,z)); }
void PhysicsEngine::Set_Gravity(glm::vec3 gravity){ Set_Gravity(gravity.x,gravity.y,gravity.z); }
void PhysicsEngine::Add_Rigid_Body(btRigidBody* rigidBody){ m_dynamicsWorld->addRigidBody(rigidBody); }
void PhysicsEngine::Update(float dt){
	m_dynamicsWorld->stepSimulation(glm::max(dt,1/60.0f)); 
}
void PhysicsEngine::Render(){
	glm::mat4 model = glm::mat4();
	GLuint shaderProgram = Resources->Get_Shader_Program("Deferred")->Get_Shader_Program();
	glm::mat4 world = Resources->Current_Camera()->Calculate_Projection(model);
	glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "MVP"), 1, GL_FALSE, glm::value_ptr(world));
	m_dynamicsWorld->debugDrawWorld(); 
}