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

//yes, this code is needed or stuff breaks. find out why
#include <SFML/OpenGL.hpp>
#include <GL/freeglut.h>
////////////////////////////////////////////

using namespace Engine;
using namespace std;


namespace Engine{
    namespace epriv{
        class GLDebugDrawer: public btIDebugDraw,public EventObserver {
            friend class ::Engine::epriv::PhysicsManager;
            private:
                GLuint m_Mode, m_VAO, m_VertexBuffer, C_MAX_POINTS;
                struct LineVertex{
                    glm::vec3 position;
                    glm::vec3 color;
                    LineVertex(){ position = glm::vec3(0.0f); color = glm::vec3(1.0f); }
                };
                vector<LineVertex> vertices;

                void init(){
                    C_MAX_POINTS = 262144;
                    m_VAO = m_VertexBuffer = 0;
					registerEvent(EventType::WindowFullscreenChanged);
                }
                void destruct(){
					unregisterEvent(EventType::WindowFullscreenChanged);
                    glDeleteBuffers(1, &m_VertexBuffer);
					Renderer::deleteVAO(m_VAO);
                    vector_clear(vertices);
                }
				void bindDataToGPU() {
					glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
					glEnableVertexAttribArray(0);
					glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex), (void*)0);
					glEnableVertexAttribArray(1);
					glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(LineVertex), (void*)(offsetof(LineVertex, color)));
				}
                void renderLines(){
                    if(m_VAO){
                        Renderer::bindVAO(m_VAO);
                        glDrawArrays(GL_LINES, 0,vertices.size());
                        Renderer::bindVAO(0);
                    }else{
						bindDataToGPU();
                        glDrawArrays(GL_LINES, 0,vertices.size());
                        glDisableVertexAttribArray(0);
                        glDisableVertexAttribArray(1);
                    }
                }
				void buildVAO() {
					Renderer::deleteVAO(m_VAO);
					if (epriv::RenderManager::OPENGL_VERSION >= 30) {
						Renderer::genAndBindVAO(m_VAO);
						bindDataToGPU();
						Renderer::bindVAO(0);
					}
				}
                void postRender(){
                    vector_clear(vertices);
                }
            public:
                void initRenderingContext(){
                    vector<LineVertex> temp1;
                    temp1.resize(C_MAX_POINTS,LineVertex());

                    glGenBuffers(1, &m_VertexBuffer);
                    glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer);
                    glBufferData(GL_ARRAY_BUFFER, sizeof(LineVertex) * C_MAX_POINTS, &temp1[0], GL_DYNAMIC_DRAW);

                    //support vao's
					buildVAO();
                }
                virtual void drawLine(const btVector3& from, const btVector3& to, const btVector3& color){
                    if(vertices.size() >= (C_MAX_POINTS)) return;
                    LineVertex v1, v2;
                    glm::vec3 _color = glm::vec3(color.x(),color.y(),color.z());
                    v1.color = _color;  v2.color = _color;
                    v1.position = glm::vec3(from.x(),from.y(),from.z());  v2.position = glm::vec3(to.x(),to.y(),to.z());
                    vertices.push_back(v1);  vertices.push_back(v2);
                }
                void drawAccumulatedLines(){
                    glBindBuffer(   GL_ARRAY_BUFFER, m_VertexBuffer);
                    glBufferSubData(GL_ARRAY_BUFFER,0, sizeof(LineVertex) * vertices.size(), &vertices[0]);
                    renderLines();
                }
				void onEvent(const Event& e) {
					if (e.type == EventType::WindowFullscreenChanged) {
						buildVAO();
					}
				}
                virtual void drawAabb(const btVector3 &from, const btVector3 &to, const btVector3 &color){
                }
                virtual void drawCylinder(btScalar radius, btScalar halfHeight, int upAxis, const btTransform &transform, const btVector3 &color){
                }
                virtual void drawContactPoint(const btVector3& PointOnB, const btVector3& normalOnB,btScalar distance,int lifeTime, const btVector3& color){
                }
                virtual void reportErrorWarning(const char* errWarning){
                }
                virtual void draw3dText(const btVector3& location, const char* text){
                }
                virtual void setDebugMode(int _mode){ m_Mode = _mode; }
                int getDebugMode() const { return m_Mode; }
                //int getDebugMode() const { return 3; }
                GLDebugDrawer(){ init(); }
                ~GLDebugDrawer(){ destruct(); }
        };
    };
};


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
        bool m_Paused;
        vector<Collision*> m_CollisionObjects;

        void _init(const char* name,uint& w,uint& h){
            m_Broadphase = new btDbvtBroadphase();
            m_CollisionConfiguration = new btDefaultCollisionConfiguration();
            m_Dispatcher = new btCollisionDispatcher(m_CollisionConfiguration);
            m_Solver = new btSequentialImpulseConstraintSolver;
            m_World = new btDiscreteDynamicsWorld(m_Dispatcher,m_Broadphase,m_Solver,m_CollisionConfiguration);
            m_Paused = false;
            m_DebugDrawer = new GLDebugDrawer();
            m_DebugDrawer->setDebugMode(btIDebugDraw::DBG_MAX_DEBUG_DRAW_MODE);
            m_World->setDebugDrawer(m_DebugDrawer);
            m_World->setGravity(btVector3(0.0f,0.0f,0.0f));

            btGImpactCollisionAlgorithm::registerAlgorithm(m_Dispatcher);

            m_World->setInternalTickCallback(_preTicCallback,(void*)m_World,true);
            m_World->setInternalTickCallback(_postTicCallback,(void*)m_World,false);
        }
        void _postInit(const char* name,uint w,uint h){
            m_DebugDrawer->initRenderingContext();
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
            if(m_Paused) return;
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

                        ComponentBody* a = (ComponentBody*)(obA->getUserPointer());
                        ComponentBody* b = (ComponentBody*)(obB->getUserPointer());

                        //a->collisionResponse(b);
                        //b->collisionResponse(a);
                    }
                }
            }
        }
        void _render(){
            m_World->debugDrawWorld();

            Camera* c = Resources::getCurrentScene()->getActiveCamera();
            glm::vec3 camPos = c->getPosition();
            glm::mat4 model = glm::mat4(1.0f);
            model[3][0] -= camPos.x;
            model[3][1] -= camPos.y;
            model[3][2] -= camPos.z;
            Renderer::sendUniformMatrix4f("Model",model);
            Renderer::sendUniformMatrix4f("VP",c->getViewProjection());

            m_DebugDrawer->drawAccumulatedLines();
            m_DebugDrawer->postRender();
        }
        void _removeCollision(Collision* collisionObject){
            removeFromVector(m_CollisionObjects,collisionObject);
            SAFE_DELETE(collisionObject);
        }
};
epriv::PhysicsManager::impl* physicsManager;

vector<glm::vec3> _rayCastInternal(const btVector3& start, const btVector3& end){
    btCollisionWorld::ClosestRayResultCallback RayCallback(start, end);
    physicsManager->m_World->rayTest(start, end, RayCallback);
    vector<glm::vec3> result;
    if(RayCallback.hasHit()){
        glm::vec3 res1 = glm::vec3(RayCallback.m_hitPointWorld.x(),RayCallback.m_hitPointWorld.y(),RayCallback.m_hitPointWorld.z()); 
        glm::vec3 res2 = glm::vec3(RayCallback.m_hitNormalWorld.x(),RayCallback.m_hitNormalWorld.y(),RayCallback.m_hitNormalWorld.z());
        result.push_back(res1);
        result.push_back(res2);
    }
    return result;
}

epriv::PhysicsManager::PhysicsManager(const char* name,uint w,uint h):m_i(new impl){ m_i->_init(name,w,h); physicsManager = m_i.get(); }
epriv::PhysicsManager::~PhysicsManager(){ m_i->_destruct(); }
void epriv::PhysicsManager::_init(const char* name,uint w,uint h){ m_i->_postInit(name,w,h); }
void epriv::PhysicsManager::_update(float dt,int maxsteps,float other){ m_i->_update(dt,maxsteps,other); }
void epriv::PhysicsManager::_render(){ m_i->_render(); }
void epriv::PhysicsManager::_removeCollision(Collision* collisionObject){ m_i->_removeCollision(collisionObject); }
const btDiscreteDynamicsWorld* epriv::PhysicsManager::_world() const{ return m_i->m_World; }

void Physics::pause(bool b){ physicsManager->m_Paused = b; }
void Physics::unpause(){ physicsManager->m_Paused = false; }
void Physics::setGravity(float x,float y,float z){ physicsManager->m_World->setGravity(btVector3(x,y,z)); }
void Physics::setGravity(glm::vec3& gravity){ Physics::setGravity(gravity.x,gravity.y,gravity.z); }
void Physics::addRigidBody(btRigidBody* rigidBody, short group, short mask){ physicsManager->m_World->addRigidBody(rigidBody,group,mask); }
void Physics::addRigidBody(btRigidBody* body){ physicsManager->m_World->addRigidBody(body); }
void Physics::removeRigidBody(btRigidBody* body){ physicsManager->m_World->removeRigidBody(body); }
void Physics::updateRigidBody(btRigidBody* body){ 
	physicsManager->m_World->updateSingleAabb(body);
}

vector<glm::vec3> Physics::rayCast(const btVector3& s, const btVector3& e,btRigidBody* ignored){
    if(ignored){
        physicsManager->m_World->removeRigidBody(ignored);
    }
    vector<glm::vec3> result = _rayCastInternal(s,e);
    if(ignored){
        physicsManager->m_World->addRigidBody(ignored);
    }
    return result;
}
vector<glm::vec3> Physics::rayCast(const btVector3& s, const btVector3& e,vector<btRigidBody*>& ignored){
    for(auto object:ignored){
        physicsManager->m_World->removeRigidBody(object);
    }
    vector<glm::vec3> result = _rayCastInternal(s,e);
    for(auto object:ignored){
        physicsManager->m_World->addRigidBody(object);
    }
    return result;
 }
vector<glm::vec3> Physics::rayCast(const glm::vec3& s, const glm::vec3& e,Entity* ignored){
    btVector3 _s = btVector3(btScalar(s.x),btScalar(s.y),btScalar(s.z));
    btVector3 _e = btVector3(btScalar(e.x),btScalar(e.y),btScalar(e.z));
    
    ComponentBody* body = ignored->getComponent<ComponentBody>();

    if(body){
        return Physics::rayCast(_s,_e,const_cast<btRigidBody*>(body->getBody()));
    }
    return Physics::rayCast(_s,_e,nullptr);
 }
vector<glm::vec3> Physics::rayCast(const glm::vec3& s, const glm::vec3& e,vector<Entity*>& ignored){
    btVector3 _s = btVector3(btScalar(s.x),btScalar(s.y),btScalar(s.z));
    btVector3 _e = btVector3(btScalar(e.x),btScalar(e.y),btScalar(e.z));
    vector<btRigidBody*> objs;
    for(auto o:ignored){
        ComponentBody* body = o->getComponent<ComponentBody>();
        if(body){
            objs.push_back(const_cast<btRigidBody*>(body->getBody()));
        }
    }
    return Engine::Physics::rayCast(_s,_e,objs);
}
Collision::Collision(btCollisionShape* shape,CollisionType::Type type, float mass){
    m_CollisionShape = shape;
    m_CollisionType = type;
    _init(type,mass);
}
Collision::Collision(epriv::ImportedMeshData& data,CollisionType::Type type, float mass,glm::vec3 scale){ 
    _load(data,type,scale);
    _init(type,mass);
}
void Collision::_init(CollisionType::Type type, float mass){
    if(!m_Inertia){
        m_Inertia = new btVector3(0.0f,0.0f,0.0f);
    }
    else{
        m_Inertia->setX(0.0f);m_Inertia->setY(0.0f);m_Inertia->setZ(0.0f);
    }
    setMass(mass);
    physicsManager->m_CollisionObjects.push_back(this);
}
Collision::~Collision(){ 
    SAFE_DELETE(m_Inertia);
    SAFE_DELETE(m_InternalMeshData);
    SAFE_DELETE(m_CollisionShape);
    m_CollisionType = CollisionType::None;
}
void Collision::_load(epriv::ImportedMeshData& data, CollisionType::Type collisionType,glm::vec3 _scale){
	if(m_CollisionShape){
		SAFE_DELETE(m_CollisionShape);
	}
	if(m_InternalMeshData){
		SAFE_DELETE(m_InternalMeshData);
	}

    m_InternalMeshData = nullptr;
    btCollisionShape* shape = nullptr;
    m_CollisionType = collisionType;
    switch(collisionType){
        case CollisionType::ConvexHull:{
            shape = new btConvexHullShape();
			btConvexHullShape& convexCast = *((btConvexHullShape*)shape);
            for(auto vertex:data.points){ 
				convexCast.addPoint(btVector3(vertex.x * _scale.x,vertex.y * _scale.y,vertex.z * _scale.z)); 
			}
            btShapeHull hull = btShapeHull(&convexCast);
            hull.buildHull(convexCast.getMargin());
            SAFE_DELETE(shape);
            const btVector3* ptsArray = hull.getVertexPointer();
            shape = new btConvexHullShape();
			btConvexHullShape& convexShape = *((btConvexHullShape*)shape);
            for(int i = 0; i < hull.numVertices(); ++i){
                convexShape.addPoint(btVector3(ptsArray[i].x(),ptsArray[i].y(),ptsArray[i].z()));
            }
			convexShape.setLocalScaling(btVector3(1.0f,1.0f,1.0f));
			convexShape.setMargin(0.001f);
			convexShape.recalcLocalAabb();
            m_CollisionShape = shape;
            break;
        }
        case CollisionType::TriangleShape:{
            m_InternalMeshData = new btTriangleMesh();
            for(auto triangle:data.file_triangles){
                btVector3 v1 = Math::btVectorFromGLM(triangle.v1.position * _scale);
                btVector3 v2 = Math::btVectorFromGLM(triangle.v2.position * _scale);
                btVector3 v3 = Math::btVectorFromGLM(triangle.v3.position * _scale);
                m_InternalMeshData->addTriangle(v1, v2, v3,true);
            }
            shape = new btGImpactMeshShape(m_InternalMeshData);
            btGImpactMeshShape& giShape = *((btGImpactMeshShape*)shape);
            giShape.setLocalScaling(btVector3(1.0f,1.0f,1.0f));
            giShape.setMargin(0.001f);
            giShape.updateBound();
            m_CollisionShape = shape;
            break;
        }
        case CollisionType::TriangleShapeStatic:{
            m_InternalMeshData = new btTriangleMesh();
            for(auto triangle:data.file_triangles){
                btVector3 v1 = Math::btVectorFromGLM(triangle.v1.position * _scale);
                btVector3 v2 = Math::btVectorFromGLM(triangle.v2.position * _scale);
                btVector3 v3 = Math::btVectorFromGLM(triangle.v3.position * _scale);
                m_InternalMeshData->addTriangle(v1, v2, v3,true);
            }
            shape = new btBvhTriangleMeshShape(m_InternalMeshData,true);
			btBvhTriangleMeshShape& triShape = *((btBvhTriangleMeshShape*)shape);
            triShape.setLocalScaling(btVector3(1.0f,1.0f,1.0f));
            triShape.setMargin(0.001f);
			triShape.recalcLocalAabb();
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
            shape = new btSphereShape(radius * _scale.x);
			btSphereShape& sphere = *((btSphereShape*)shape);
			sphere.setLocalScaling(btVector3(1.0f,1.0f,1.0f));
			sphere.setMargin(0.001f);
            m_CollisionShape = shape;
            break;
        };
        case CollisionType::Box:{
            glm::vec3 max = glm::vec3(0.0f);
            for(auto vertex:data.file_points){
                float x = abs(vertex.x); float y = abs(vertex.y); float z = abs(vertex.z);
                if(x > max.x) max.x = x; if(y > max.y) max.y = y; if(z > max.z) max.z = z;
            }
            shape = new btBoxShape(btVector3(max.x * _scale.x,max.y * _scale.y,max.z * _scale.z));
			btBoxShape& boxShape = *((btBoxShape*)shape);
			boxShape.setLocalScaling(btVector3(1.0f,1.0f,1.0f));
			boxShape.setMargin(0.001f);
            m_CollisionShape = shape;
            break;
        }
        default:{
            break;
        }
    }
}
void Collision::setMass(float mass){
    if(!m_CollisionShape || m_CollisionType == CollisionType::TriangleShapeStatic || m_CollisionType == CollisionType::None) return;

    if(m_CollisionType != CollisionType::TriangleShape){
        m_CollisionShape->calculateLocalInertia(mass,*m_Inertia);
    }
    else{
        ((btGImpactMeshShape*)m_CollisionShape)->calculateLocalInertia(mass,*m_Inertia);
    }
}
