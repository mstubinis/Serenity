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
#include "ObjectDynamic.h"

using namespace Engine;

btBroadphaseInterface* Engine::Physics::Detail::PhysicsManagement::m_broadphase = nullptr;
btDefaultCollisionConfiguration* Engine::Physics::Detail::PhysicsManagement::m_collisionConfiguration = nullptr;
btCollisionDispatcher* Engine::Physics::Detail::PhysicsManagement::m_dispatcher = nullptr;
btSequentialImpulseConstraintSolver* Engine::Physics::Detail::PhysicsManagement::m_solver = nullptr;
btDiscreteDynamicsWorld* Engine::Physics::Detail::PhysicsManagement::m_dynamicsWorld = nullptr;
GLDebugDrawer* Engine::Physics::Detail::PhysicsManagement::m_debugDrawer = nullptr;

std::vector<Collision*> Engine::Physics::Detail::PhysicsManagement::m_Collisions;

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
    for(auto collision:m_Collisions)
        SAFE_DELETE(collision);
}

void Engine::Physics::setGravity(float x,float y,float z){ Engine::Physics::Detail::PhysicsManagement::m_dynamicsWorld->setGravity(btVector3(x,y,z)); }
void Engine::Physics::setGravity(glm::vec3 gravity){ Engine::Physics::setGravity(gravity.x,gravity.y,gravity.z); }
void Engine::Physics::addRigidBody(btRigidBody* rigidBody, short group, short mask){ 
    Engine::Physics::Detail::PhysicsManagement::m_dynamicsWorld->addRigidBody(rigidBody,group,mask); 
}

void Engine::Physics::addRigidBody(ObjectDynamic* obj){ Engine::Physics::addRigidBody(obj->getRigidBody()); }
void Engine::Physics::addRigidBody(btRigidBody* body){
    Engine::Physics::Detail::PhysicsManagement::m_dynamicsWorld->addRigidBody(body); 
}

void Engine::Physics::removeRigidBody(btRigidBody* body){
    Engine::Physics::Detail::PhysicsManagement::m_dynamicsWorld->removeRigidBody(body);
}
void Engine::Physics::removeRigidBody(ObjectDynamic* obj){ Engine::Physics::removeRigidBody(obj->getRigidBody()); }

void Engine::Physics::Detail::PhysicsManagement::update(float dt,unsigned int maxSteps,float other){ 
	m_dynamicsWorld->stepSimulation(dt,maxSteps,other); 
    unsigned int numManifolds = m_dynamicsWorld->getDispatcher()->getNumManifolds();
    for (unsigned int i = 0; i < numManifolds; i++){
        btPersistentManifold* contactManifold =  m_dynamicsWorld->getDispatcher()->getManifoldByIndexInternal(i);
        btCollisionObject* obA = const_cast<btCollisionObject*>(contactManifold->getBody0());
        btCollisionObject* obB = const_cast<btCollisionObject*>(contactManifold->getBody1());
        unsigned int numContacts = contactManifold->getNumContacts();
        for (unsigned int j = 0; j<numContacts; j++){
            btManifoldPoint& pt = contactManifold->getContactPoint(j);
            if (pt.getDistance() < 0.0f){
                const btVector3& ptA = pt.getPositionWorldOnA();
                const btVector3& ptB = pt.getPositionWorldOnB();
                const btVector3& normalOnB = pt.m_normalWorldOnB;

				ObjectDynamic* a = static_cast<ObjectDynamic*>(obA->getUserPointer());
				ObjectDynamic* b = static_cast<ObjectDynamic*>(obB->getUserPointer());

				a->collisionResponse(b);
				b->collisionResponse(a);
            }
        }
    }
}
void Engine::Physics::Detail::PhysicsManagement::render(){
	if(Engine::Resources::Detail::ResourceManagement::m_RenderingAPI == ENGINE_RENDERING_API_OPENGL){
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
}


Collision::Collision(btCollisionShape* shape,COLLISION_TYPE type, float mass){ 
    m_CollisionShape = shape;
	m_CollisionType = type;
    _init(type,mass);
}
Collision::Collision(std::string file,COLLISION_TYPE type, float mass){ 
	_load(file,type);
    _init(type,mass);
}
void Collision::_init(COLLISION_TYPE type, float mass){
    if(m_Inertia == nullptr){
        m_Inertia = new btVector3(0,0,0);
    }
    else{
        m_Inertia->setX(0);m_Inertia->setY(0);m_Inertia->setZ(0);
    }
	setMass(mass);
	Engine::Physics::Detail::PhysicsManagement::m_Collisions.push_back(this);
}
Collision::~Collision(){ 
    SAFE_DELETE(m_Inertia);
    SAFE_DELETE(m_CollisionShape); 
    m_CollisionType = COLLISION_TYPE_NONE;
}
void Collision::_load(std::string file, COLLISION_TYPE collisionType){
    btCollisionShape* shape = nullptr;
    std::string extention; for(unsigned int i = file.length() - 4; i < file.length(); i++) extention += tolower(file.at(i));
    boost::iostreams::stream<boost::iostreams::mapped_file_source> str(file);
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
                        ((btConvexHullShape*)shape)->addPoint(btVector3(x1,y1,z1));
                    }
                }
                #pragma endregion
            }
            this->m_CollisionShape = shape;
            this->m_CollisionType = COLLISION_TYPE_CONVEXHULL;
            return;
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
void Collision::setMass(float mass){
    if(m_CollisionShape != nullptr){
		if(m_CollisionType != COLLISION_TYPE_TRIANGLESHAPE){
            m_CollisionShape->calculateLocalInertia(mass,*m_Inertia);
        }
        else{
            ((btGImpactMeshShape*)m_CollisionShape)->calculateLocalInertia(mass,*m_Inertia);
        }
    }
}

GLDebugDrawer::GLDebugDrawer():m_debugMode(0){}
GLDebugDrawer::~GLDebugDrawer(){}
void GLDebugDrawer::drawLine(const btVector3& from,const btVector3& to,const btVector3& fromColor, const btVector3& toColor){
    glBegin(GL_LINES);
        glColor3f(fromColor.getX(), fromColor.getY(), fromColor.getZ());
        glVertex3d(from.getX(), from.getY(), from.getZ());
        glColor3f(toColor.getX(), toColor.getY(), toColor.getZ());
        glVertex3d(to.getX(), to.getY(), to.getZ());
    glEnd();
}
void GLDebugDrawer::drawLine(const btVector3& from,const btVector3& to,const btVector3& color){ 
    drawLine(from,to,color,color);
}
void GLDebugDrawer::drawSphere (const btVector3& p, btScalar radius, const btVector3& color){
    glColor4f (color.getX(), color.getY(), color.getZ(), btScalar(1.0f));
    glPushMatrix ();
    glTranslatef (p.getX(), p.getY(), p.getZ());

    int lats = 5;
    int longs = 5;

    int i, j;
    for(i = 0; i <= lats; i++) {
        btScalar lat0 = SIMD_PI * (-btScalar(0.5) + (btScalar) (i - 1) / lats);
        btScalar z0  = radius*sin(lat0);
        btScalar zr0 =  radius*cos(lat0);

        btScalar lat1 = SIMD_PI * (-btScalar(0.5) + (btScalar) i / lats);
        btScalar z1 = radius*sin(lat1);
        btScalar zr1 = radius*cos(lat1);

        glBegin(GL_QUAD_STRIP);
        for(j = 0; j <= longs; j++) {
            btScalar lng = 2 * SIMD_PI * (btScalar) (j - 1) / longs;
            btScalar x = cos(lng);
            btScalar y = sin(lng);

            glNormal3f(x * zr0, y * zr0, z0);
            glVertex3f(x * zr0, y * zr0, z0);
            glNormal3f(x * zr1, y * zr1, z1);
            glVertex3f(x * zr1, y * zr1, z1);
        }
        glEnd();
    }
    glPopMatrix();
}
void GLDebugDrawer::drawTriangle(const btVector3& a,const btVector3& b,const btVector3& c,const btVector3& color,btScalar alpha){
    const btVector3	n=btCross(b-a,c-a).normalized();
    glBegin(GL_TRIANGLES);		
        glColor4f(color.getX(), color.getY(), color.getZ(),alpha);
        glNormal3d(n.getX(),n.getY(),n.getZ());
        glVertex3d(a.getX(),a.getY(),a.getZ());
        glVertex3d(b.getX(),b.getY(),b.getZ());
        glVertex3d(c.getX(),c.getY(),c.getZ());
    glEnd();
}
void GLDebugDrawer::drawContactPoint(const btVector3& pointOnB,const btVector3& normalOnB,btScalar distance,int lifeTime,const btVector3& color){
    btVector3 to=pointOnB+normalOnB*1;//distance;
    const btVector3&from = pointOnB;
    glColor4f(color.getX(), color.getY(), color.getZ(),1.f);
    glBegin(GL_LINES);
        glVertex3d(from.getX(), from.getY(), from.getZ());
        glVertex3d(to.getX(), to.getY(), to.getZ());
    glEnd();
}
void GLDebugDrawer::reportErrorWarning(const char* warningString){}
void GLDebugDrawer::draw3dText(const btVector3& location,const char* textString){
    glRasterPos3f(location.x(),location.y(),location.z());
}
void GLDebugDrawer::setDebugMode(int debugMode){ m_debugMode = debugMode; }

#ifdef _WIN32//for glut.h
#include <windows.h>
#endif

//think different
#if defined(__APPLE__) && !defined (VMDMESA)
#include <TargetConditionals.h>
#if (defined (TARGET_OS_IPHONE) && TARGET_OS_IPHONE) || (defined (TARGET_IPHONE_SIMULATOR) && TARGET_IPHONE_SIMULATOR)
#import <OpenGLES/ES1/gl.h>
#define glOrtho glOrthof
#else
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#include <GLUT/glut.h>
#endif
#else

#ifdef _WINDOWS
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glut.h>
#endif
#endif

#include <stdio.h>
#include <string.h> //for memset

void GLDebugResetFont(int screenWidth,int screenHeight){}
#define USE_ARRAYS 1
void GLDebugDrawStringInternal(int x,int y,const char* string, const btVector3& rgb){GLDebugDrawStringInternal(x,y,string,rgb,true,10);}
void GLDebugDrawStringInternal(int x,int y,const char* string, const btVector3& rgb, bool enableBlend, int spacing){}
void GLDebugDrawString(int x,int y,const char* string){}