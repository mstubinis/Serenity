#include "Engine_Physics.h"

#include <bullet/btBulletDynamicsCommon.h>
#include <bullet/btBulletCollisionCommon.h>
#include <bullet/LinearMath/btIDebugDraw.h>
#include <bullet/BulletCollision/Gimpact/btCompoundFromGimpact.h>
#include <bullet/BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h>
#include <bullet/BulletCollision/Gimpact/btGImpactShape.h>

#include <boost/filesystem.hpp>
#include <boost/iostreams/device/mapped_file.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "Engine_Resources.h"
#include "Camera.h"
#include "GLDebugDrawer.h"

using namespace Engine;

btBroadphaseInterface* Engine::Physics::Detail::PhysicsManagement::m_broadphase = nullptr;
btDefaultCollisionConfiguration* Engine::Physics::Detail::PhysicsManagement::m_collisionConfiguration = nullptr;
btCollisionDispatcher* Engine::Physics::Detail::PhysicsManagement::m_dispatcher = nullptr;
btSequentialImpulseConstraintSolver* Engine::Physics::Detail::PhysicsManagement::m_solver = nullptr;
btDiscreteDynamicsWorld* Engine::Physics::Detail::PhysicsManagement::m_dynamicsWorld = nullptr;
GLDebugDrawer* Engine::Physics::Detail::PhysicsManagement::m_debugDrawer = nullptr;

std::vector<MeshCollision*> Engine::Physics::Detail::PhysicsManagement::m_MeshCollisions;

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

	btGImpactCollisionAlgorithm::registerAlgorithm(m_dispatcher);
}
void Engine::Physics::Detail::PhysicsManagement::destruct(){
	SAFE_DELETE(m_debugDrawer);
	SAFE_DELETE(m_dynamicsWorld);
	SAFE_DELETE(m_solver);
	SAFE_DELETE(m_dispatcher);
	SAFE_DELETE(m_collisionConfiguration);
	SAFE_DELETE(m_broadphase);
	for(auto collision:m_MeshCollisions)
		SAFE_DELETE(collision);
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
	glUseProgram(0);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixf(glm::value_ptr(Resources::getActiveCamera()->getProjection()));

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixf(glm::value_ptr(Resources::getActiveCamera()->getView()));

	m_dynamicsWorld->debugDrawWorld();

	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

MeshCollision::MeshCollision(btCollisionShape* shape,COLLISION_TYPE type){ 
	m_CollisionShape = shape;
	m_CollisionType = type;
	Engine::Physics::Detail::PhysicsManagement::m_MeshCollisions.push_back(this);
}
MeshCollision::MeshCollision(std::string filename,COLLISION_TYPE type){ 
	m_CollisionShape = nullptr;
	load(filename,type);
	Engine::Physics::Detail::PhysicsManagement::m_MeshCollisions.push_back(this);
}
MeshCollision::~MeshCollision(){ 
	SAFE_DELETE(m_CollisionShape); 
	m_CollisionType = COLLISION_TYPE_NONE;
}
void MeshCollision::load(std::string filename, COLLISION_TYPE collisionType){
	btCollisionShape* shape = nullptr;
	std::string extention;
	for(unsigned int i = filename.length() - 4; i < filename.length(); i++) extention += tolower(filename.at(i));
	boost::iostreams::stream<boost::iostreams::mapped_file_source> str(filename);
	switch(collisionType){
		case COLLISION_TYPE_CONVEXHULL:{
			shape = new btConvexHullShape();
			if(extention == ".obj"){
				#pragma region OBJ
				for(std::string line; std::getline(str, line, '\n');){
					std::string x; std::string y; std::string z;
					unsigned int spaceCount = 0;
					if(line[0] == 'v' && line[1] == ' '){
						for(auto c:line){
							if(c == ' ')                 spaceCount++;
							else{
								if(spaceCount == 1)      x += c;
								else if(spaceCount == 2) y += c;
								else if(spaceCount == 3) z += c;
							}
						}
						float x1 = static_cast<float>(::atof(x.c_str()));
						float y1 = static_cast<float>(::atof(y.c_str()));
						float z1 = static_cast<float>(::atof(z.c_str()));

						btVector3 pos = btVector3(x1,y1,z1);
						((btConvexHullShape*)shape)->addPoint(pos);
					}
				}
				this->m_CollisionShape = shape;
				this->m_CollisionType = COLLISION_TYPE_CONVEXHULL;
				return;
				#pragma endregion
			}
			break;
		}
		case COLLISION_TYPE_TRIANGLESHAPE:{
		   std::vector <int> indices;
		   std::vector <float> positions;
		   btTriangleMesh* mesh = new btTriangleMesh();
			if(extention == ".obj"){
				#pragma region OBJ
				int index = 1;
				std::vector<glm::vec3> pointData;
				std::vector<std::vector<glm::vec3>> listOfVerts;
				for(std::string line; std::getline(str, line, '\n');){
					std::string x; std::string y; std::string z;
					unsigned int spaceCount = 0;
					unsigned int slashCount = 0;
					if(line[0] == 'v'){ 
						for(auto c:line){
							if(c == ' ')                 spaceCount++;
							else{
								if(spaceCount == 1)      x += c;
								else if(spaceCount == 2) y += c;
								else if(spaceCount == 3) z += c;
							}
						}
						if(line[1] == ' '){//vertex point
							float x1 = static_cast<float>(::atof(x.c_str()));
							float y1 = static_cast<float>(::atof(y.c_str()));
							float z1 = static_cast<float>(::atof(z.c_str()));
							pointData.push_back(glm::vec3(x1,y1,z1));
						}
						index++;
					}
					//faces
					else if(line[0] == 'f' && line[1] == ' '){
						std::vector<glm::vec3> vertices;
						unsigned int count = 0;
						for(auto c:line){
							if(c == '/') {
								slashCount++;
							}
							else if(c == ' '){ 
								//global listOfVerts
								if(spaceCount != 0){
									glm::vec3 vertex = glm::vec3(static_cast<float>(::atof(x.c_str())),static_cast<float>(::atof(y.c_str())),static_cast<float>(::atof(z.c_str())));
									vertices.push_back(vertex);
									x = ""; y = ""; z = "";
									slashCount = 0;
								}
								spaceCount++;
							}
							else{
								if(spaceCount > 0){
									if(slashCount == 0)      x += c;
									else if(slashCount == 1) y += c;
									else if(slashCount == 2) z += c;
								}
							}
							count++;
						}
						glm::vec3 vertex = glm::vec3(static_cast<float>(::atof(x.c_str())),static_cast<float>(::atof(y.c_str())),static_cast<float>(::atof(z.c_str())));
						vertices.push_back(vertex);
						listOfVerts.push_back(vertices);
					}
				}
				for(auto face:listOfVerts){
					glm::vec3 v1,v2,v3,v4;

					v1 = pointData.at(static_cast<unsigned int>(face.at(0).x-1));
					v2 = pointData.at(static_cast<unsigned int>(face.at(1).x-1));
					v3 = pointData.at(static_cast<unsigned int>(face.at(2).x-1));

					btVector3 bv1 = btVector3(v1.x,v1.y,v1.z);
					btVector3 bv2 = btVector3(v2.x,v2.y,v2.z);
					btVector3 bv3 = btVector3(v3.x,v3.y,v3.z);

					if(face.size() == 4){//quad
						v4 = pointData.at(static_cast<unsigned int>(face.at(3).x-1));
						btVector3 bv4 = btVector3(v4.x,v4.y,v4.z);
	
						mesh->addTriangle(bv1, bv2, bv3,true);
						mesh->addTriangle(bv1, bv3, bv4,true);
					}
					else{//triangle
						mesh->addTriangle(bv1, bv2, bv3,true);
					}
				}
				#pragma endregion
			}

			shape = new btGImpactMeshShape(mesh);
			((btGImpactMeshShape*)shape)->setLocalScaling(btVector3(1.0f,1.0f,1.0f));
			((btGImpactMeshShape*)shape)->setMargin(0.001f);
			((btGImpactMeshShape*)shape)->updateBound();
			
			this->m_CollisionShape = shape;
			this->m_CollisionType = COLLISION_TYPE_TRIANGLESHAPE;
			return;
			break;
		}
		case COLLISION_TYPE_BOXSHAPE:{
			glm::vec3 max = glm::vec3(0);
			if(extention == ".obj"){
				#pragma region OBJ
				std::vector<glm::vec3> pointData;
				std::vector<std::vector<glm::vec3>> listOfVerts;
				for(std::string line; std::getline(str, line, '\n');){
					std::string x; std::string y; std::string z;
					unsigned int spaceCount = 0;
					if(line[0] == 'v'){ 
						for(auto c:line){
							if(c == ' ')                 spaceCount++;
							else{
								if(spaceCount == 1)      x += c;
								else if(spaceCount == 2) y += c;
								else if(spaceCount == 3) z += c;
							}
						}
						if(line[1] == ' '){//vertex point
							float x1 = static_cast<float>(::atof(x.c_str()));
							float y1 = static_cast<float>(::atof(y.c_str()));
							float z1 = static_cast<float>(::atof(z.c_str()));
							pointData.push_back(glm::vec3(x1,y1,z1));
						}
					}
				}
				for(auto point:pointData){
					float x = abs(point.x);
					float y = abs(point.y);
					float z = abs(point.z);

					if(x > max.x) max.x = x;
					if(y > max.y) max.y = y;
					if(z > max.z) max.z = z;
				}
				#pragma endregion
			}
			shape = new btBoxShape(btVector3(max.x,max.y,max.z));
			this->m_CollisionShape = shape;
			this->m_CollisionType = COLLISION_TYPE_BOXSHAPE;
			return;
			break;
		}
	}
}
Collision::Collision(MeshCollision* _meshCollision,float mass){ 
	if(m_Inertia == nullptr){
		m_Inertia = new btVector3(0,0,0);
	}
	else{
		m_Inertia->setX(0);m_Inertia->setY(0);m_Inertia->setZ(0);
	}
	m_MeshCollision = _meshCollision;
	recalculate(mass);
}
Collision::~Collision(){ 
	SAFE_DELETE(m_Inertia);
}
void Collision::recalculate(float mass){
	if(m_MeshCollision->getCollisionShape() != nullptr){
		if(m_MeshCollision->getCollisionType() != COLLISION_TYPE_TRIANGLESHAPE){
			m_MeshCollision->getCollisionShape()->calculateLocalInertia(mass,*m_Inertia);
		}
		else{
			((btGImpactMeshShape*)m_MeshCollision->getCollisionShape())->calculateLocalInertia(mass,*m_Inertia);
		}
	}
}