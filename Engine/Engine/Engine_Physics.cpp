#include "Engine_Physics.h"
#include "Engine_Resources.h"
#include "Engine_Renderer.h"
#include "Camera.h"
#include "ObjectDynamic.h"
#include "Mesh.h"
#include "Scene.h"

#include <bullet/BulletCollision/CollisionShapes/btShapeHull.h>
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

using namespace Engine;
using namespace Engine::Physics;
using namespace std;

btBroadphaseInterface* Physics::Detail::PhysicsManagement::m_broadphase = nullptr;
btDefaultCollisionConfiguration* Physics::Detail::PhysicsManagement::m_collisionConfiguration = nullptr;
btCollisionDispatcher* Physics::Detail::PhysicsManagement::m_dispatcher = nullptr;
btSequentialImpulseConstraintSolver* Physics::Detail::PhysicsManagement::m_solver = nullptr;
btDiscreteDynamicsWorld* Physics::Detail::PhysicsManagement::m_world = nullptr;
GLDebugDrawer* Physics::Detail::PhysicsManagement::m_debugDrawer = nullptr;

std::vector<Collision*> Physics::Detail::PhysicsManagement::m_Collisions;

void Detail::PhysicsManagement::_preTicCallback(btDynamicsWorld* world, btScalar timeStep){
}
void Detail::PhysicsManagement::_postTicCallback(btDynamicsWorld* world, btScalar timeStep){
}

void Detail::PhysicsManagement::init(){
    m_broadphase = new btDbvtBroadphase();
    m_collisionConfiguration = new btDefaultCollisionConfiguration();
    m_dispatcher = new btCollisionDispatcher(m_collisionConfiguration);
    m_solver = new btSequentialImpulseConstraintSolver;
    m_world = new btDiscreteDynamicsWorld(m_dispatcher,m_broadphase,m_solver,m_collisionConfiguration);

    m_debugDrawer = new GLDebugDrawer();
    m_debugDrawer->setDebugMode(btIDebugDraw::DBG_MAX_DEBUG_DRAW_MODE );
    m_world->setDebugDrawer(m_debugDrawer);
    m_world->setGravity(btVector3(0.0f,0.0f,0.0f));

    btGImpactCollisionAlgorithm::registerAlgorithm(m_dispatcher);

    m_world->setInternalTickCallback(Detail::PhysicsManagement::_preTicCallback, static_cast<void*>(m_world),true);
    m_world->setInternalTickCallback(Detail::PhysicsManagement::_postTicCallback, static_cast<void*>(m_world),false);
}
void Detail::PhysicsManagement::destruct(){
    SAFE_DELETE(m_debugDrawer);
    SAFE_DELETE(m_world);
    SAFE_DELETE(m_solver);
    SAFE_DELETE(m_dispatcher);
    SAFE_DELETE(m_collisionConfiguration);
    SAFE_DELETE(m_broadphase);
    for(auto collision:m_Collisions)
        SAFE_DELETE(collision);
}

void Physics::setGravity(float x,float y,float z){ Physics::Detail::PhysicsManagement::m_world->setGravity(btVector3(x,y,z)); }
void Physics::setGravity(glm::vec3& gravity){ Physics::setGravity(gravity.x,gravity.y,gravity.z); }
void Physics::addRigidBody(btRigidBody* rigidBody, short group, short mask){ Physics::Detail::PhysicsManagement::m_world->addRigidBody(rigidBody,group,mask); }
void Physics::addRigidBody(ObjectDynamic* obj){ Physics::addRigidBody(obj->getRigidBody()); }
void Physics::addRigidBody(btRigidBody* body){ Physics::Detail::PhysicsManagement::m_world->addRigidBody(body); }
void Physics::removeRigidBody(btRigidBody* body){ Physics::Detail::PhysicsManagement::m_world->removeRigidBody(body); }
void Physics::removeRigidBody(ObjectDynamic* obj){ Physics::removeRigidBody(obj->getRigidBody()); }

void Detail::PhysicsManagement::update(float dt,int maxSteps,float other){ 
    m_world->stepSimulation(dt,maxSteps,other);
    uint numManifolds = m_world->getDispatcher()->getNumManifolds();
    for (uint i = 0; i < numManifolds; i++){
        btPersistentManifold* contactManifold =  m_world->getDispatcher()->getManifoldByIndexInternal(i);
        btCollisionObject* obA = const_cast<btCollisionObject*>(contactManifold->getBody0());
        btCollisionObject* obB = const_cast<btCollisionObject*>(contactManifold->getBody1());
        for (uint j = 0; j < uint(contactManifold->getNumContacts()); j++){
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
vector<glm::vec3> Physics::rayCast(const btVector3& s, const btVector3& e,btRigidBody* ignored){
    if(ignored != nullptr) Detail::PhysicsManagement::m_world->removeRigidBody(ignored);
    vector<glm::vec3> result = Detail::PhysicsManagement::rayCastInternal(s,e);
    if(ignored != nullptr) Detail::PhysicsManagement::m_world->addRigidBody(ignored);
    return result;
}
vector<glm::vec3> Physics::rayCast(const btVector3& s, const btVector3& e,vector<btRigidBody*>& ignored){
    for(auto object:ignored) Detail::PhysicsManagement::m_world->removeRigidBody(object);
    vector<glm::vec3> result = Detail::PhysicsManagement::rayCastInternal(s,e);
    for(auto object:ignored) Detail::PhysicsManagement::m_world->addRigidBody(object);
    return result;
 }
vector<glm::vec3> Physics::rayCast(const glm::vec3& s, const glm::vec3& e,Object* ignored){
    btVector3 _s = btVector3(btScalar(s.x),btScalar(s.y),btScalar(s.z));
    btVector3 _e = btVector3(btScalar(e.x),btScalar(e.y),btScalar(e.z));
    ObjectDynamic* b = dynamic_cast<ObjectDynamic*>(ignored);
    if(b != NULL) return Physics::rayCast(_s,_e,b->getRigidBody());
    return Physics::rayCast(_s,_e,nullptr);
 }
vector<glm::vec3> Physics::rayCast(const glm::vec3& s, const glm::vec3& e,vector<Object*>& ignored){
    btVector3 _s = btVector3(btScalar(s.x),btScalar(s.y),btScalar(s.z));
    btVector3 _e = btVector3(btScalar(e.x),btScalar(e.y),btScalar(e.z));
    vector<btRigidBody*> objs;
    for(auto o:ignored){
        ObjectDynamic* b = dynamic_cast<ObjectDynamic*>(o);
        if(b != NULL) objs.push_back(b->getRigidBody());
    }
    return Engine::Physics::rayCast(_s,_e,objs);
}
vector<glm::vec3> Physics::Detail::PhysicsManagement::rayCastInternal(const btVector3& start, const btVector3& end){
    btCollisionWorld::ClosestRayResultCallback RayCallback(start, end);
    Detail::PhysicsManagement::m_world->rayTest(start, end, RayCallback);
    vector<glm::vec3> result;
    if(RayCallback.hasHit()){
        glm::vec3 res1 = glm::vec3(RayCallback.m_hitPointWorld.x(),RayCallback.m_hitPointWorld.y(),RayCallback.m_hitPointWorld.z()); 
        glm::vec3 res2 = glm::vec3(RayCallback.m_hitNormalWorld.x(),RayCallback.m_hitNormalWorld.y(),RayCallback.m_hitNormalWorld.z());
        result.push_back(res1);
        result.push_back(res2);
    }
    return result;
}
void Detail::PhysicsManagement::render(){
    glMatrixMode(GL_PROJECTION); glPushMatrix();
	Camera* c = Resources::getCurrentScene()->getActiveCamera();
    glLoadMatrixf(glm::value_ptr(c->getProjection()));

    glMatrixMode(GL_MODELVIEW); glPushMatrix();
    glLoadMatrixf(glm::value_ptr(c->getView()));

    m_world->debugDrawWorld();

    glMatrixMode(GL_PROJECTION); glPopMatrix();
    glMatrixMode(GL_MODELVIEW); glPopMatrix();
}
Collision::Collision(btCollisionShape* shape,CollisionType type, float mass){
    m_CollisionShape = shape;
    m_CollisionType = type;
    _init(type,mass);
}
Collision::Collision(ImportedMeshData& data,CollisionType type, float mass){ 
    _load(data,type);
    _init(type,mass);
}
void Collision::_init(CollisionType type, float mass){
    if(m_Inertia == nullptr){
        m_Inertia = new btVector3(0.0f,0.0f,0.0f);
    }
    else{
        m_Inertia->setX(0.0f);m_Inertia->setY(0.0f);m_Inertia->setZ(0.0f);
    }
    setMass(mass);
    Detail::PhysicsManagement::m_Collisions.push_back(this);
}
Collision::~Collision(){ 
    SAFE_DELETE(m_Inertia);
    SAFE_DELETE(m_InternalMeshData);
    SAFE_DELETE(m_CollisionShape);
    m_CollisionType = CollisionType::None;
}
void Collision::_load(ImportedMeshData& data, CollisionType collisionType){
    m_InternalMeshData = nullptr;
    btCollisionShape* shape = nullptr;
    m_CollisionType = collisionType;
    switch(collisionType){
        case CollisionType::ConvexHull:{
            shape = new btConvexHullShape();
            for(auto vertex:data.points){ ((btConvexHullShape*)shape)->addPoint(btVector3(vertex.x,vertex.y,vertex.z)); }
            btShapeHull* hull =  new btShapeHull((btConvexHullShape*)shape);
            hull->buildHull(shape->getMargin());
            delete shape;
            const btVector3* ptsArray = hull->getVertexPointer();
            shape = new btConvexHullShape();
            for(int i = 0; i < hull->numVertices(); i++){
                ((btConvexHullShape*)shape)->addPoint(btVector3(ptsArray[i].x(),ptsArray[i].y(),ptsArray[i].z()));
            }
            m_CollisionShape = shape;
            delete hull;
            break;
        }
        case CollisionType::TriangleShape:{
            m_InternalMeshData = new btTriangleMesh();
            for(auto triangle:data.file_triangles){
                glm::vec3 v1,v2,v3;
                v1 = triangle.v1.position;
                v2 = triangle.v2.position;
                v3 = triangle.v3.position;
                btVector3 bv1 = btVector3(v1.x,v1.y,v1.z);
                btVector3 bv2 = btVector3(v2.x,v2.y,v2.z);
                btVector3 bv3 = btVector3(v3.x,v3.y,v3.z);
                m_InternalMeshData->addTriangle(bv1, bv2, bv3,true);
            }
            shape = new btGImpactMeshShape(m_InternalMeshData);
            ((btGImpactMeshShape*)shape)->setLocalScaling(btVector3(1.0f,1.0f,1.0f));
            ((btGImpactMeshShape*)shape)->setMargin(0.001f);
            ((btGImpactMeshShape*)shape)->updateBound();
            m_CollisionShape = shape;
            break;
        }
        case CollisionType::TriangleShapeStatic:{
            m_InternalMeshData = new btTriangleMesh();
            for(auto triangle:data.file_triangles){
                glm::vec3 v1Pos,v2Pos,v3Pos;
                v1Pos = triangle.v1.position;
                v2Pos = triangle.v2.position;
                v3Pos = triangle.v3.position;
                btVector3 v1 = btVector3(v1Pos.x,v1Pos.y,v1Pos.z);
                btVector3 v2 = btVector3(v2Pos.x,v2Pos.y,v2Pos.z);
                btVector3 v3 = btVector3(v3Pos.x,v3Pos.y,v3Pos.z);
                m_InternalMeshData->addTriangle(v1, v2, v3,true);
            }
            shape = new btBvhTriangleMeshShape(m_InternalMeshData,true);
            (shape)->setLocalScaling(btVector3(1.0f,1.0f,1.0f));
            (shape)->setMargin(0.001f);
            m_CollisionShape = shape;
            break;
        }
        case CollisionType::Box:{
            glm::vec3 max = glm::vec3(0.0f);
            for(auto vertex:data.file_points){
                float x = abs(vertex.x); float y = abs(vertex.y); float z = abs(vertex.z);
                if(x > max.x) max.x = x; if(y > max.y) max.y = y; if(z > max.z) max.z = z;
            }
            shape = new btBoxShape(btVector3(max.x,max.y,max.z));
            m_CollisionShape = shape;
            break;
        }
        default:{
            break;
        }
    }
}
void Collision::setMass(float mass){
    if(m_CollisionShape == nullptr || m_CollisionType == CollisionType::TriangleShapeStatic || m_CollisionType == CollisionType::None) return;

    if(m_CollisionType != CollisionType::TriangleShape){
        m_CollisionShape->calculateLocalInertia(mass,*m_Inertia);
    }
    else{
        ((btGImpactMeshShape*)m_CollisionShape)->calculateLocalInertia(mass,*m_Inertia);
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
        btScalar z0  = radius*glm::sin(lat0);
        btScalar zr0 =  radius*glm::cos(lat0);

        btScalar lat1 = SIMD_PI * (-btScalar(0.5) + (btScalar) i / lats);
        btScalar z1 = radius*glm::sin(lat1);
        btScalar zr1 = radius*glm::cos(lat1);

        glBegin(GL_QUAD_STRIP);
        for(j = 0; j <= longs; j++) {
            btScalar lng = 2 * SIMD_PI * (btScalar) (j - 1) / longs;
            btScalar x = glm::cos(lng);
            btScalar y = glm::sin(lng);

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
    const btVector3 n=btCross(b-a,c-a).normalized();
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
