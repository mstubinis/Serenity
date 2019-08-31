#define BT_THREADSAFE 1
#include <core/engine/physics/Engine_Physics.h>
#include <core/engine/physics/World.h>
#include <core/engine/physics/DebugDrawer.h>

#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/mesh/Mesh.h>
#include <core/ModelInstance.h>
#include <core/engine/scene/Scene.h>

// ecs
#include <ecs/ComponentModel.h>
#include <ecs/ComponentBody.h>

#include <BulletCollision/CollisionShapes/btShapeHull.h>
#include <BulletCollision/CollisionShapes/btHeightfieldTerrainShape.h>
#include <btBulletCollisionCommon.h>
//#include <LinearMath/btIDebugDraw.h>

//Multi-threading
#include <core/engine/threading/Engine_ThreadManager.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorldMt.h>
#include <BulletDynamics/Dynamics/btSimulationIslandManagerMt.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolverMt.h>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

//yes, this code is needed or stuff breaks. find out why
#include <SFML/OpenGL.hpp>
#include <GL/freeglut.h>


using namespace Engine;
using namespace std;

class epriv::PhysicsManager::impl final{
    public:
        PhysicsWorld* data;
        bool m_Paused;

        void _init(){
            m_Paused = false;      
        }
        void _postInit(const char* name, const uint& w, const uint& h, const uint& numCores){
            data = new epriv::PhysicsWorld(numCores);
            data->debugDrawer->initRenderingContext();
        }
        void _destructWorldObjectsOnly() {
            auto& world = *data->world;
            int collisionObjCount = world.getNumCollisionObjects();
            for (int i = 0; i < collisionObjCount; ++i) {
                btCollisionObject* obj = world.getCollisionObjectArray()[i];
                if (obj) {
                    //btRigidBody* body = btRigidBody::upcast(obj);
                    //if (body) {
                        //auto* motionState = body->getMotionState();
                        //SAFE_DELETE(motionState);
                    //}
                    world.removeCollisionObject(obj);
                    SAFE_DELETE(obj);
                }
            }
        }
        void _destruct(){
            _destructWorldObjectsOnly();
            SAFE_DELETE(data);
        }
        void _update(const double& dt, int& maxSteps, float& other){
            if(m_Paused) 
                return;
            data->world->stepSimulation(static_cast<btScalar>(dt),maxSteps,other);

            uint numManifolds = data->dispatcher->getNumManifolds();
            for (uint i = 0; i < numManifolds; ++i){
                btPersistentManifold* contactManifold = data->dispatcher->getManifoldByIndexInternal(i);
                btCollisionObject* collisionObjectA = const_cast<btCollisionObject*>(contactManifold->getBody0());
                btCollisionObject* collisionObjectB = const_cast<btCollisionObject*>(contactManifold->getBody1());
                for (int j = 0; j < contactManifold->getNumContacts(); ++j){
                    btManifoldPoint& pt = contactManifold->getContactPoint(j);
                    if (pt.getDistance() < 0.0f){
                        glm::vec3 ptA = Math::btVectorToGLM(pt.getPositionWorldOnA());
                        glm::vec3 ptB = Math::btVectorToGLM(pt.getPositionWorldOnB());
                        glm::vec3 normalOnB = Math::btVectorToGLM(pt.m_normalWorldOnB);

                        auto aPtr = collisionObjectA->getUserPointer();
                        auto bPtr = collisionObjectB->getUserPointer();

                        ComponentBody* _a = static_cast<ComponentBody*>(aPtr);
                        ComponentBody* _b = static_cast<ComponentBody*>(bPtr);
                        if (_a && _b) {
                            ComponentBody& a = *_a;
                            ComponentBody& b = *_b;

                            a.collisionResponse(a, ptA, b, ptB, normalOnB);
                            b.collisionResponse(b, ptB, a, ptA, normalOnB);
                        }
                    }
                }
            }
        }
        void _render(Camera& camera){
            data->world->debugDrawWorld();
            const glm::vec3 camPos = camera.getPosition();
            const glm::mat4 model = glm::mat4(1.0f);
            Renderer::sendUniformMatrix4("Model", model);
            Renderer::sendUniformMatrix4("VP", camera.getViewProjection());
            data->debugDrawer->drawAccumulatedLines();
            data->debugDrawer->postRender();
        }
        void _addRigidBody(btRigidBody* _rigidBody) {
            int collisionObjCount = data->world->getNumCollisionObjects();
            for (int i = 0; i < collisionObjCount; ++i) {
                btRigidBody* body = btRigidBody::upcast(data->world->getCollisionObjectArray()[i]);
                if (body) {
                    if (body == _rigidBody) {
                        return;
                    }
                }
            }
            data->world->addRigidBody(_rigidBody);
        }
        void _addRigidBody(btRigidBody* _rigidBody, short group, short mask) {
            int collisionObjCount = data->world->getNumCollisionObjects();
            for (int i = 0; i < collisionObjCount; ++i) {
                btRigidBody* body = btRigidBody::upcast(data->world->getCollisionObjectArray()[i]);
                if (body) {
                    if (body == _rigidBody) {
                        return;
                    }
                }
            }
            data->world->addRigidBody(_rigidBody, group, mask);
        }
        void _removeRigidBody(btRigidBody* _rigidBody) {
            int collisionObjCount = data->world->getNumCollisionObjects();
            for (int i = 0; i < collisionObjCount; ++i) {
                btRigidBody* body = btRigidBody::upcast(data->world->getCollisionObjectArray()[i]);
                if (body) {
                    if (body == _rigidBody) {
                        for (int i = body->getNumConstraintRefs() - 1; i >= 0; i--){
                            btTypedConstraint* con = body->getConstraintRef(i);
                            data->world->removeConstraint(con);
                        }
                        data->world->removeRigidBody(_rigidBody);
                        return;
                    }
                }
            }
        }
        void _updateRigidBody(btRigidBody* rigidBody) {
            data->world->updateSingleAabb(rigidBody);
        }
};
epriv::PhysicsManager::impl* physicsManager;

epriv::PhysicsManager::PhysicsManager():m_i(new impl){ 
    m_i->_init();
    physicsManager = m_i.get(); 
}
epriv::PhysicsManager::~PhysicsManager(){ 
    m_i->_destruct(); 
}
void epriv::PhysicsManager::_init(const char* name, const uint& w, const uint& h, const uint& numCores){ 
    m_i->_postInit(name,w,h,numCores); 
}
void epriv::PhysicsManager::_update(const double& dt,int maxsteps,float other){ 
    m_i->_update(dt,maxsteps,other); 
}
void epriv::PhysicsManager::_render(Camera& camera){
    m_i->_render(camera);
}

void Physics::pause(bool b){ 
    physicsManager->m_Paused = b; 
}
void Physics::unpause(){ 
    physicsManager->m_Paused = false; 
}
void Physics::setGravity(const float x, const float y, const float z){ 
    physicsManager->data->world->setGravity(btVector3(x,y,z)); 
}
void Physics::setGravity(const glm::vec3& gravity){ 
    Physics::setGravity(gravity.x,gravity.y,gravity.z); 
}
void Physics::addRigidBody(btRigidBody* rigidBody, short group, short mask){ 
    physicsManager->_addRigidBody(rigidBody,group,mask); 
}
void Physics::addRigidBody(btRigidBody* rigidBody){ 
    physicsManager->_addRigidBody(rigidBody); 
}
void Physics::removeRigidBody(btRigidBody* rigidBody){ 
    physicsManager->_removeRigidBody(rigidBody); 
}
void Physics::updateRigidBody(btRigidBody* rigidBody){ 
    physicsManager->_updateRigidBody(rigidBody); 
}
vector<glm::vec3> _rayCastInternal(const btVector3& start, const btVector3& end) {
    btCollisionWorld::ClosestRayResultCallback RayCallback(start, end);
    physicsManager->data->world->rayTest(start, end, RayCallback);
    vector<glm::vec3> result;
    if (RayCallback.hasHit()) {
        result.reserve(2); //is this needed performance wise?
        glm::vec3 res1 = glm::vec3(RayCallback.m_hitPointWorld.x(), RayCallback.m_hitPointWorld.y(), RayCallback.m_hitPointWorld.z());
        glm::vec3 res2 = glm::vec3(RayCallback.m_hitNormalWorld.x(), RayCallback.m_hitNormalWorld.y(), RayCallback.m_hitNormalWorld.z());
        result.push_back(res1);
        result.push_back(res2);
    }
    return result;
}
vector<glm::vec3> Physics::rayCast(const btVector3& s, const btVector3& e,btRigidBody* ignored){
    if(ignored){
        physicsManager->data->world->removeRigidBody(ignored);
    }
    vector<glm::vec3> result = _rayCastInternal(s,e);
    if(ignored){
        physicsManager->data->world->addRigidBody(ignored);
    }
    return result;
}
vector<glm::vec3> Physics::rayCast(const btVector3& s, const btVector3& e,vector<btRigidBody*>& ignored){
    for(auto& object:ignored){
        physicsManager->data->world->removeRigidBody(object);
    }
    vector<glm::vec3> result = _rayCastInternal(s,e);
    for(auto& object:ignored){
        physicsManager->data->world->addRigidBody(object);
    }
    return result;
 }
vector<glm::vec3> Physics::rayCast(const glm::vec3& s, const glm::vec3& e, Entity* ignored){
    btVector3 _s = Math::btVectorFromGLM(s);
    btVector3 _e = Math::btVectorFromGLM(e);
    if (ignored) {
        ComponentBody* body = ignored->getComponent<ComponentBody>();
        if (body) {
			const auto& rigid = body->getBtBody();
            return Physics::rayCast(_s, _e, &const_cast<btRigidBody&>(rigid));
        }
    }
    return Physics::rayCast(_s, _e, nullptr);
 }
vector<glm::vec3> Physics::rayCast(const glm::vec3& s, const glm::vec3& e,vector<Entity>& ignored){
    btVector3 _s = Math::btVectorFromGLM(s);
    btVector3 _e = Math::btVectorFromGLM(e);
    vector<btRigidBody*> objs;
    for(auto& o : ignored){
        ComponentBody* body = o.getComponent<ComponentBody>();
        if(body){
			const auto& rigid = body->getBtBody();
            objs.push_back(&const_cast<btRigidBody&>(rigid));
        }
    }
    return Physics::rayCast(_s, _e, objs);
}