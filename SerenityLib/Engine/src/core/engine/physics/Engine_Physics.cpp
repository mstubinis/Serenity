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
#include <BulletCollision/NarrowPhaseCollision/btRaycastCallback.h>

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
vector<RayCastResult> _rayCastInternal(const btVector3& start, const btVector3& end, const unsigned short group, const unsigned short mask) {
    btCollisionWorld::AllHitsRayResultCallback RayCallback(start, end);
    RayCallback.m_collisionFilterMask = mask;
    RayCallback.m_collisionFilterGroup = group;

    physicsManager->data->world->rayTest(start, end, RayCallback);
    //physicsManager->data->world->getDebugDrawer()->drawLine(start, end, btVector4(1, 1, 0, 1));

    vector<RayCastResult> result;
    if (RayCallback.hasHit()) {
        auto& pts = RayCallback.m_hitPointWorld;
        auto& normals = RayCallback.m_hitNormalWorld;

        for (uint i = 0; i < pts.size(); ++i) {
            const glm::vec3 hitPoint = Math::btVectorToGLM(RayCallback.m_hitPointWorld[i]);
            const glm::vec3 hitNormal = Math::btVectorToGLM(RayCallback.m_hitNormalWorld[i]);

            RayCastResult res;
            res.hitPosition = hitPoint;
            res.hitNormal = hitNormal;
            result.push_back(res);
        }
    }
    return result;
}
vector<RayCastResult> Physics::rayCast(const btVector3& start, const btVector3& end, btRigidBody* ignored, const unsigned short group, const unsigned short mask){
    if(ignored){
        physicsManager->data->world->removeRigidBody(ignored);
    }
    vector<RayCastResult> result = _rayCastInternal(start, end, group, mask);
    if(ignored){
        physicsManager->data->world->addRigidBody(ignored);
    }
    return result;
}
vector<RayCastResult> Physics::rayCast(const btVector3& start, const btVector3& end, vector<btRigidBody*>& ignored, const unsigned short group, const unsigned short mask){
    for(auto& object:ignored){
        physicsManager->data->world->removeRigidBody(object);
    }
    vector<RayCastResult> result = _rayCastInternal(start, end, group, mask);
    for(auto& object:ignored){
        physicsManager->data->world->addRigidBody(object);
    }
    return result;
 }
vector<RayCastResult> Physics::rayCast(const glm::vec3& start, const glm::vec3& end, Entity* ignored, const unsigned short group, const unsigned short mask){
    const btVector3 start_ = Math::btVectorFromGLM(start);
    const btVector3 end_ = Math::btVectorFromGLM(end);
    if (ignored) {
        ComponentBody* body = ignored->getComponent<ComponentBody>();
        if (body) {
            if (body->hasPhysics()) {
                const auto& rigid = body->getBtBody();
                return Physics::rayCast(start_, end_, &const_cast<btRigidBody&>(rigid), group, mask);
            }
        }
    }
    return Physics::rayCast(start_, end_, nullptr, group, mask);
 }
vector<RayCastResult> Physics::rayCast(const glm::vec3& start, const glm::vec3& end ,vector<Entity>& ignored, const unsigned short group, const unsigned short mask){
    const btVector3 start_ = Math::btVectorFromGLM(start);
    const btVector3 end_ = Math::btVectorFromGLM(end);
    vector<btRigidBody*> objs;
    for(auto& o : ignored){
        ComponentBody* body = o.getComponent<ComponentBody>();
        if(body){
            if (body->hasPhysics()) {
                const auto& rigid = body->getBtBody();
                objs.push_back(&const_cast<btRigidBody&>(rigid));
            }
        }
    }
    return Physics::rayCast(start_, end_, objs, group, mask);
}