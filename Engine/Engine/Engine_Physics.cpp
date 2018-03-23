#include "Engine.h"
#include "Engine_Physics.h"
#include "Engine_Resources.h"
#include "Engine_Renderer.h"
#include "Camera.h"
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
using namespace std;


void _preTicCallback(btDynamicsWorld* world, btScalar timeStep){
}
void _postTicCallback(btDynamicsWorld* world, btScalar timeStep){
}

class epriv::PhysicsManager::impl final{
    public:
        btBroadphaseInterface* m_Broadphase;
        btDefaultCollisionConfiguration* m_CollisionConfiguration;
        btCollisionDispatcher* m_Dispatcher;
        btSequentialImpulseConstraintSolver* m_Solver;
        btDiscreteDynamicsWorld* m_World;
		GLDebugDrawer* m_DebugDrawer;

		vector<Collision*> m_CollisionObjects;

		void _init(const char* name,uint& w,uint& h){
			m_Broadphase = new btDbvtBroadphase();
			m_CollisionConfiguration = new btDefaultCollisionConfiguration();
			m_Dispatcher = new btCollisionDispatcher(m_CollisionConfiguration);
			m_Solver = new btSequentialImpulseConstraintSolver;
			m_World = new btDiscreteDynamicsWorld(m_Dispatcher,m_Broadphase,m_Solver,m_CollisionConfiguration);

			m_DebugDrawer = new GLDebugDrawer();
			m_DebugDrawer->setDebugMode(btIDebugDraw::DBG_MAX_DEBUG_DRAW_MODE);
			m_World->setDebugDrawer(m_DebugDrawer);
			m_World->setGravity(btVector3(0.0f,0.0f,0.0f));

			btGImpactCollisionAlgorithm::registerAlgorithm(m_Dispatcher);

			m_World->setInternalTickCallback(_preTicCallback,(void*)m_World,true);
			m_World->setInternalTickCallback(_postTicCallback,(void*)m_World,false);
		}
		void _postInit(const char* name,uint w,uint h){
		}
		void _destruct(){
			SAFE_DELETE(m_DebugDrawer);
			SAFE_DELETE(m_World);
			SAFE_DELETE(m_Solver);
			SAFE_DELETE(m_Dispatcher);
			SAFE_DELETE(m_CollisionConfiguration);
			SAFE_DELETE(m_Broadphase);
			for(auto collision:m_CollisionObjects)
				SAFE_DELETE(collision);
		}
		void _update(float& dt, int& maxSteps, float& other){
			m_World->stepSimulation(dt,maxSteps,other);
			uint numManifolds = m_World->getDispatcher()->getNumManifolds();
			for (uint i = 0; i < numManifolds; ++i){
				btPersistentManifold* contactManifold =  m_World->getDispatcher()->getManifoldByIndexInternal(i);
				btCollisionObject* obA = const_cast<btCollisionObject*>(contactManifold->getBody0());
				btCollisionObject* obB = const_cast<btCollisionObject*>(contactManifold->getBody1());
				for (int j = 0; j < contactManifold->getNumContacts(); ++j){
					btManifoldPoint& pt = contactManifold->getContactPoint(j);
					if (pt.getDistance() < 0.0f){
						const btVector3& ptA = pt.getPositionWorldOnA();
						const btVector3& ptB = pt.getPositionWorldOnB();
						const btVector3& normalOnB = pt.m_normalWorldOnB;

						ComponentRigidBody* a = (ComponentRigidBody*)(obA->getUserPointer());
						ComponentRigidBody* b = (ComponentRigidBody*)(obB->getUserPointer());

						//a->collisionResponse(b);
						//b->collisionResponse(a);
					}
				}
			}
		}
		void _render(){
			glUseProgram(0); //this is important
			glMatrixMode(GL_PROJECTION); glPushMatrix();
			Camera* c = Resources::getCurrentScene()->getActiveCamera();
			glLoadMatrixf(glm::value_ptr(c->getProjection()));
			glMatrixMode(GL_MODELVIEW); glPushMatrix();
			glLoadMatrixf(glm::value_ptr(c->getView()));
			m_World->debugDrawWorld();
			glMatrixMode(GL_PROJECTION); glPopMatrix();
			glMatrixMode(GL_MODELVIEW); glPopMatrix();
		}
		void _removeCollision(Collision* collisionObject){
			m_CollisionObjects.erase(std::remove(m_CollisionObjects.begin(), m_CollisionObjects.end(), collisionObject), m_CollisionObjects.end());
            SAFE_DELETE(collisionObject);
		}
};
vector<glm::vec3> _rayCastInternal(const btVector3& start, const btVector3& end){
    btCollisionWorld::ClosestRayResultCallback RayCallback(start, end);
    epriv::Core::m_Engine->m_PhysicsManager->m_i->m_World->rayTest(start, end, RayCallback);
    vector<glm::vec3> result;
    if(RayCallback.hasHit()){
        glm::vec3 res1 = glm::vec3(RayCallback.m_hitPointWorld.x(),RayCallback.m_hitPointWorld.y(),RayCallback.m_hitPointWorld.z()); 
        glm::vec3 res2 = glm::vec3(RayCallback.m_hitNormalWorld.x(),RayCallback.m_hitNormalWorld.y(),RayCallback.m_hitNormalWorld.z());
        result.push_back(res1);
        result.push_back(res2);
    }
    return result;
}

epriv::PhysicsManager::PhysicsManager(const char* name,uint w,uint h):m_i(new impl){ m_i->_init(name,w,h); }
epriv::PhysicsManager::~PhysicsManager(){ m_i->_destruct(); }
void epriv::PhysicsManager::_init(const char* name,uint w,uint h){ m_i->_postInit(name,w,h); }
void epriv::PhysicsManager::_update(float dt,int maxsteps,float other){ m_i->_update(dt,maxsteps,other); }
void epriv::PhysicsManager::_render(){ m_i->_render(); }
void epriv::PhysicsManager::_removeCollision(Collision* collisionObject){ m_i->_removeCollision(collisionObject); }
const btDiscreteDynamicsWorld* epriv::PhysicsManager::_world() const{ return m_i->m_World; }

void Physics::setGravity(float x,float y,float z){ epriv::Core::m_Engine->m_PhysicsManager->m_i->m_World->setGravity(btVector3(x,y,z)); }
void Physics::setGravity(glm::vec3& gravity){ Physics::setGravity(gravity.x,gravity.y,gravity.z); }
void Physics::addRigidBody(btRigidBody* rigidBody, short group, short mask){ epriv::Core::m_Engine->m_PhysicsManager->m_i->m_World->addRigidBody(rigidBody,group,mask); }
void Physics::addRigidBody(btRigidBody* body){ epriv::Core::m_Engine->m_PhysicsManager->m_i->m_World->addRigidBody(body); }
void Physics::removeRigidBody(btRigidBody* body){ epriv::Core::m_Engine->m_PhysicsManager->m_i->m_World->removeRigidBody(body); }

vector<glm::vec3> Physics::rayCast(const btVector3& s, const btVector3& e,btRigidBody* ignored){
    if(ignored != nullptr) epriv::Core::m_Engine->m_PhysicsManager->m_i->m_World->removeRigidBody(ignored);
    vector<glm::vec3> result = _rayCastInternal(s,e);
    if(ignored != nullptr) epriv::Core::m_Engine->m_PhysicsManager->m_i->m_World->addRigidBody(ignored);
    return result;
}
vector<glm::vec3> Physics::rayCast(const btVector3& s, const btVector3& e,vector<btRigidBody*>& ignored){
    for(auto object:ignored) epriv::Core::m_Engine->m_PhysicsManager->m_i->m_World->removeRigidBody(object);
    vector<glm::vec3> result = _rayCastInternal(s,e);
    for(auto object:ignored) epriv::Core::m_Engine->m_PhysicsManager->m_i->m_World->addRigidBody(object);
    return result;
 }
vector<glm::vec3> Physics::rayCast(const glm::vec3& s, const glm::vec3& e,Entity* ignored){
    btVector3 _s = btVector3(btScalar(s.x),btScalar(s.y),btScalar(s.z));
    btVector3 _e = btVector3(btScalar(e.x),btScalar(e.y),btScalar(e.z));
	
	ComponentRigidBody* body = ignored->getComponent<ComponentRigidBody>();


    if(body) return Physics::rayCast(_s,_e,const_cast<btRigidBody*>(body->getBody()));

    return Physics::rayCast(_s,_e,nullptr);
 }
vector<glm::vec3> Physics::rayCast(const glm::vec3& s, const glm::vec3& e,vector<Entity*>& ignored){
    btVector3 _s = btVector3(btScalar(s.x),btScalar(s.y),btScalar(s.z));
    btVector3 _e = btVector3(btScalar(e.x),btScalar(e.y),btScalar(e.z));
    vector<btRigidBody*> objs;
    for(auto o:ignored){
		ComponentRigidBody* body = o->getComponent<ComponentRigidBody>();
        if(body) objs.push_back(const_cast<btRigidBody*>(body->getBody()));
    }
    return Engine::Physics::rayCast(_s,_e,objs);
}
Collision::Collision(btCollisionShape* shape,CollisionType::Type type, float mass){
    m_CollisionShape = shape;
    m_CollisionType = type;
    _init(type,mass);
}
Collision::Collision(epriv::ImportedMeshData& data,CollisionType::Type type, float mass){ 
    _load(data,type);
    _init(type,mass);
}
void Collision::_init(CollisionType::Type type, float mass){
    if(m_Inertia == nullptr){
        m_Inertia = new btVector3(0.0f,0.0f,0.0f);
    }
    else{
        m_Inertia->setX(0.0f);m_Inertia->setY(0.0f);m_Inertia->setZ(0.0f);
    }
    setMass(mass);
    epriv::Core::m_Engine->m_PhysicsManager->m_i->m_CollisionObjects.push_back(this);
}
Collision::~Collision(){ 
    SAFE_DELETE(m_Inertia);
    SAFE_DELETE(m_InternalMeshData);
    SAFE_DELETE(m_CollisionShape);
    m_CollisionType = CollisionType::None;
}
void Collision::_load(epriv::ImportedMeshData& data, CollisionType::Type collisionType){
    m_InternalMeshData = nullptr;
    btCollisionShape* shape = nullptr;
    m_CollisionType = collisionType;
    switch(collisionType){
        case CollisionType::ConvexHull:{
            shape = new btConvexHullShape();
            for(auto vertex:data.points){ ((btConvexHullShape*)shape)->addPoint(btVector3(vertex.x,vertex.y,vertex.z)); }
            btShapeHull* hull =  new btShapeHull((btConvexHullShape*)shape);
            hull->buildHull(shape->getMargin());
            SAFE_DELETE(shape);
            const btVector3* ptsArray = hull->getVertexPointer();
            shape = new btConvexHullShape();
            for(int i = 0; i < hull->numVertices(); ++i){
                ((btConvexHullShape*)shape)->addPoint(btVector3(ptsArray[i].x(),ptsArray[i].y(),ptsArray[i].z()));
            }
            m_CollisionShape = shape;
            SAFE_DELETE(hull);
            break;
        }
        case CollisionType::TriangleShape:{
            m_InternalMeshData = new btTriangleMesh();
            for(auto triangle:data.file_triangles){
				btVector3 v1 = Engine::Math::btVectorFromGLM(triangle.v1.position);
				btVector3 v2 = Engine::Math::btVectorFromGLM(triangle.v2.position);
				btVector3 v3 = Engine::Math::btVectorFromGLM(triangle.v3.position);
                m_InternalMeshData->addTriangle(v1, v2, v3,true);
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
				btVector3 v1 = Engine::Math::btVectorFromGLM(triangle.v1.position);
				btVector3 v2 = Engine::Math::btVectorFromGLM(triangle.v2.position);
				btVector3 v3 = Engine::Math::btVectorFromGLM(triangle.v3.position);
                m_InternalMeshData->addTriangle(v1, v2, v3,true);
            }
            shape = new btBvhTriangleMeshShape(m_InternalMeshData,true);
            (shape)->setLocalScaling(btVector3(1.0f,1.0f,1.0f));
            (shape)->setMargin(0.001f);
            m_CollisionShape = shape;
            break;
        }
		case CollisionType::Sphere:{
			float radius = 0;
			for(auto vertex:data.points){
				float length = glm::length(vertex);
				if(length > radius){
					radius = length;
				}
			}
			shape = new btSphereShape(radius);
			m_CollisionShape = shape;
			break;
		};
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
    for(i = 0; i <= lats; ++i) {
        btScalar lat0 = SIMD_PI * (-btScalar(0.5) + (btScalar) (i - 1) / lats);
        btScalar z0  = radius*glm::sin(lat0);
        btScalar zr0 =  radius*glm::cos(lat0);

        btScalar lat1 = SIMD_PI * (-btScalar(0.5) + (btScalar) i / lats);
        btScalar z1 = radius*glm::sin(lat1);
        btScalar zr1 = radius*glm::cos(lat1);

        glBegin(GL_QUAD_STRIP);
        for(j = 0; j <= longs; ++j) {
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

void GLDebugResetFont(int screenWidth,int screenHeight){}
#define USE_ARRAYS 1
void GLDebugDrawStringInternal(int x,int y,const char* string, const btVector3& rgb){GLDebugDrawStringInternal(x,y,string,rgb,true,10);}
void GLDebugDrawStringInternal(int x,int y,const char* string, const btVector3& rgb, bool enableBlend, int spacing){}
void GLDebugDrawString(int x,int y,const char* string){}
