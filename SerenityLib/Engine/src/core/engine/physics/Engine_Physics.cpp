#define BT_THREADSAFE 1
#include <core/engine/physics/Engine_Physics.h>
#include <core/engine/physics/World.h>
#include <core/engine/physics/DebugDrawer.h>

#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/scene/Camera.h>
#include <core/engine/mesh/Mesh.h>
#include <core/engine/model/ModelInstance.h>
#include <core/engine/scene/Scene.h>

// ecs
#include <ecs/ComponentModel.h>
#include <ecs/ComponentBody.h>

#include <BulletCollision/CollisionShapes/btShapeHull.h>
#include <BulletCollision/CollisionShapes/btCollisionShape.h>
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

bool CustomMaterialContactAddedCallback(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0, const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1) {
    btCollisionShape* childShapeA = nullptr;
    btCollisionShape* childShapeB = nullptr;

    btCollisionObjectWrapper* ARoot = const_cast<btCollisionObjectWrapper*>(colObj0Wrap);
    btCollisionObjectWrapper* BRoot = const_cast<btCollisionObjectWrapper*>(colObj1Wrap);
    btCollisionObjectWrapper* ARootPrev = nullptr;
    btCollisionObjectWrapper* BRootPrev = nullptr;
    while (ARoot->m_parent) {
        ARootPrev = ARoot;
        ARoot = const_cast<btCollisionObjectWrapper*>(ARoot->m_parent);
    }
    while (BRoot->m_parent) {
        BRootPrev = BRoot;
        BRoot = const_cast<btCollisionObjectWrapper*>(BRoot->m_parent);
    }
    if (ARoot->getCollisionShape()->getShapeType() == BroadphaseNativeTypes::COMPOUND_SHAPE_PROXYTYPE) {
        if(ARootPrev)
            childShapeA = const_cast<btCollisionShape*>(ARootPrev->getCollisionShape());
    }
    if (BRoot->getCollisionShape()->getShapeType() == BroadphaseNativeTypes::COMPOUND_SHAPE_PROXYTYPE) {
        if(BRootPrev)
            childShapeB = const_cast<btCollisionShape*>(BRootPrev->getCollisionShape());
    }
    if (cp.getDistance() < 0.0f) {
        btCollisionObject* collisionObjectA = const_cast<btCollisionObject*>(colObj0Wrap->getCollisionObject());
        btCollisionObject* collisionObjectB = const_cast<btCollisionObject*>(colObj1Wrap->getCollisionObject());
        
        ComponentBody* _a = static_cast<ComponentBody*>(collisionObjectA->getUserPointer());
        ComponentBody* _b = static_cast<ComponentBody*>(collisionObjectB->getUserPointer());
        if (_a && _b) {
            glm::vec3 ptA       = Math::btVectorToGLM(cp.getPositionWorldOnA());
            glm::vec3 ptB       = Math::btVectorToGLM(cp.getPositionWorldOnB());
            glm::vec3 normalOnB = Math::btVectorToGLM(cp.m_normalWorldOnB);

            glm::vec3 localA = Math::btVectorToGLM(cp.m_localPointA);
            glm::vec3 localB = Math::btVectorToGLM(cp.m_localPointB);

            glm::vec3 normalA = glm::normalize(ptB - ptA);
            glm::vec3 normalB = glm::normalize(ptA - ptB);

            ComponentBody& a = *_a;
            ComponentBody& b = *_b;

            CollisionCallbackEventData dataA(a, b, ptA, ptB, normalOnB, localA, localB, normalA);
            dataA.ownerCollisionObj = collisionObjectA;
            dataA.otherCollisionObj = collisionObjectB;
            CollisionCallbackEventData dataB(b, a, ptB, ptA, normalOnB, localB, localA, normalB);
            dataB.ownerCollisionObj = collisionObjectB;
            dataB.otherCollisionObj = collisionObjectA;

            if (childShapeA) {
                void* usrPtr = childShapeA->getUserPointer();
                auto& modelInstance = *static_cast<ModelInstance*>(usrPtr);
                const size_t modelIndex = modelInstance.index();
                dataA.ownerModelInstanceIndex = modelIndex;
                dataB.otherModelInstanceIndex = modelIndex;
            }
            if (childShapeB) {
                void* usrPtr = childShapeB->getUserPointer();
                auto& modelInstance = *static_cast<ModelInstance*>(usrPtr);
                const size_t modelIndex = modelInstance.index();
                dataA.otherModelInstanceIndex = modelIndex;
                dataB.ownerModelInstanceIndex = modelIndex;
            }

            a.collisionResponse(dataA);
            b.collisionResponse(dataB);
            cp.setDistance((btScalar)9999999999999.0); //hacky way of saying "dont process this again"
        }
    }
    return true;
}

epriv::PhysicsManager* physicsManager;

epriv::PhysicsManager::PhysicsManager(){ 
    m_Paused = false;
    m_Data = nullptr;
    physicsManager = this;
    m_NumberOfStepsPerFrame = 1;
}
epriv::PhysicsManager::~PhysicsManager(){ 
    cleanup();
}
void epriv::PhysicsManager::cleanup() {
    if (!m_Data)
        return;
    auto& world = *m_Data->world;
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
    SAFE_DELETE(m_Data);
}
void epriv::PhysicsManager::_init(){
    m_Data = NEW epriv::PhysicsWorld();
    m_Data->debugDrawer->initRenderingContext();

    gContactAddedCallback = CustomMaterialContactAddedCallback;
}
void epriv::PhysicsManager::_update(const double& dt,int maxsteps, float other){ 
    if (m_Paused)
        return;
    m_Data->world->stepSimulation(static_cast<btScalar>(dt), maxsteps, static_cast<btScalar>(other));

    auto& dispatcher = *m_Data->dispatcher;
    for (int i = 0; i < dispatcher.getNumManifolds(); ++i) {
        btPersistentManifold& contactManifold = *dispatcher.getManifoldByIndexInternal(i);

        for (int j = 0; j < contactManifold.getNumContacts(); ++j) {
            btManifoldPoint& cp = contactManifold.getContactPoint(j);
            if (cp.getDistance() < 0.0f) {
                btCollisionObject* collisionObjectA = const_cast<btCollisionObject*>(contactManifold.getBody0());
                btCollisionObject* collisionObjectB = const_cast<btCollisionObject*>(contactManifold.getBody1());
                auto aPtr = collisionObjectA->getUserPointer();
                auto bPtr = collisionObjectB->getUserPointer();
                ComponentBody* _a = static_cast<ComponentBody*>(aPtr);
                ComponentBody* _b = static_cast<ComponentBody*>(bPtr);
                if (_a && _b) {
                    ComponentBody& a = *_a;
                    ComponentBody& b = *_b;

                    glm::vec3 ptA = Math::btVectorToGLM(cp.getPositionWorldOnA());
                    glm::vec3 ptB = Math::btVectorToGLM(cp.getPositionWorldOnB());
                    glm::vec3 normalOnB = Math::btVectorToGLM(cp.m_normalWorldOnB);

                    glm::vec3 localA = Math::btVectorToGLM(cp.m_localPointA);
                    glm::vec3 localB = Math::btVectorToGLM(cp.m_localPointB);

                    glm::vec3 normalA = glm::normalize(ptB - ptA);
                    glm::vec3 normalB = glm::normalize(ptA - ptB);

                    CollisionCallbackEventData dataA(a, b, ptA, ptB, normalOnB, localA, localB, normalA);
                    dataA.ownerCollisionObj = collisionObjectA;
                    dataA.otherCollisionObj = collisionObjectB;
                    CollisionCallbackEventData dataB(b, a, ptB, ptA, normalOnB, localB, localA, normalB);
                    dataB.ownerCollisionObj = collisionObjectB;
                    dataB.otherCollisionObj = collisionObjectA;

                    a.collisionResponse(dataA);
                    b.collisionResponse(dataB);

                    cp.setDistance((btScalar)9999999999999.0); //hacky way of saying "dont process this again"
                }
            }
        }
    }
}
void epriv::PhysicsManager::_render(Camera& camera){
    m_Data->world->debugDrawWorld();
    const glm::vec3 camPos = camera.getPosition();
    const glm::mat4 model = glm::mat4(1.0f);
    Renderer::sendUniformMatrix4("Model", model);
    Renderer::sendUniformMatrix4("VP", camera.getViewProjection());
    m_Data->debugDrawer->drawAccumulatedLines();
    m_Data->debugDrawer->postRender();
}
void Physics::setNumberOfStepsPerFrame(const unsigned int numSteps) {
    physicsManager->m_NumberOfStepsPerFrame = glm::max(1U, numSteps);
}
const unsigned int Physics::getNumberOfStepsPerFrame() {
    return physicsManager->m_NumberOfStepsPerFrame;
}
void Physics::pause(bool b){ 
    physicsManager->m_Paused = b; 
}
void Physics::unpause(){ 
    physicsManager->m_Paused = false; 
}
void Physics::setGravity(const float x, const float y, const float z){ 
    physicsManager->m_Data->world->setGravity(btVector3(x,y,z));
}
void Physics::setGravity(const glm::vec3& gravity){ 
    Physics::setGravity(gravity.x,gravity.y,gravity.z); 
}
void Physics::addRigidBody(btRigidBody* rigidBody, short group, short mask){ 
    auto& data = *physicsManager->m_Data;
    int collisionObjCount = data.world->getNumCollisionObjects();
    for (int i = 0; i < collisionObjCount; ++i) {
        btRigidBody* body = btRigidBody::upcast(data.world->getCollisionObjectArray()[i]);
        if (body) {
            if (body == rigidBody) {
                return;
            }
        }
    }
    data.world->addRigidBody(rigidBody, group, mask);
}
void Physics::addRigidBody(btRigidBody* rigidBody){ 
    auto& data = *physicsManager->m_Data;
    int collisionObjCount = data.world->getNumCollisionObjects();
    for (int i = 0; i < collisionObjCount; ++i) {
        btRigidBody* body = btRigidBody::upcast(data.world->getCollisionObjectArray()[i]);
        if (body) {
            if (body == rigidBody) {
                return;
            }
        }
    }
    data.world->addRigidBody(rigidBody);
}
void Physics::removeRigidBody(btRigidBody* rigidBody){ 
    auto& data = *physicsManager->m_Data;
    int collisionObjCount = data.world->getNumCollisionObjects();
    for (int i = 0; i < collisionObjCount; ++i) {
        btRigidBody* body = btRigidBody::upcast(data.world->getCollisionObjectArray()[i]);
        if (body) {
            if (body == rigidBody) {
                for (int i = body->getNumConstraintRefs() - 1; i >= 0; i--) {
                    btTypedConstraint* con = body->getConstraintRef(i);
                    data.world->removeConstraint(con);
                }
                data.world->removeRigidBody(rigidBody);
                return;
            }
        }
    }
}
void Physics::updateRigidBody(btRigidBody* rigidBody){ 
    physicsManager->m_Data->world->updateSingleAabb(rigidBody);
}
void Physics::addRigidBody(ComponentBody& body) {
    Physics::addRigidBody(&const_cast<btRigidBody&>(body.getBtBody()), body.getCollisionGroup(), body.getCollisionMask());
}
void Physics::removeRigidBody(ComponentBody& body) {
    Physics::removeRigidBody(&const_cast<btRigidBody&>(body.getBtBody()));
}


RayCastResult _rayCastInternal_Nearest(const btVector3& start, const btVector3& end, const unsigned short group, const unsigned short mask) {
    btCollisionWorld::ClosestRayResultCallback RayCallback(start, end);
    RayCallback.m_collisionFilterMask = mask;
    RayCallback.m_collisionFilterGroup = group;

    physicsManager->m_Data->world->rayTest(start, end, RayCallback);
    //physicsManager->m_Data->world->getDebugDrawer()->drawLine(start, end, btVector4(1, 1, 0, 1));

    RayCastResult result;
    if (RayCallback.hasHit()) {
        auto& pts = RayCallback.m_hitPointWorld;
        auto& normals = RayCallback.m_hitNormalWorld;

        const glm::vec3 hitPoint = Math::btVectorToGLM(RayCallback.m_hitPointWorld);
        const glm::vec3 hitNormal = Math::btVectorToGLM(RayCallback.m_hitNormalWorld);

        result.hitPosition = hitPoint;
        result.hitNormal = hitNormal;
    }
    return result;
}



vector<RayCastResult> _rayCastInternal(const btVector3& start, const btVector3& end, const unsigned short group, const unsigned short mask) {
    btCollisionWorld::AllHitsRayResultCallback RayCallback(start, end);
    RayCallback.m_collisionFilterMask = mask;
    RayCallback.m_collisionFilterGroup = group;

    physicsManager->m_Data->world->rayTest(start, end, RayCallback);
    //physicsManager->m_Data->world->getDebugDrawer()->drawLine(start, end, btVector4(1, 1, 0, 1));

    vector<RayCastResult> result;
    if (RayCallback.hasHit()) {
        auto& pts = RayCallback.m_hitPointWorld;
        auto& normals = RayCallback.m_hitNormalWorld;

        for (int i = 0; i < pts.size(); ++i) {
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
vector<RayCastResult> Physics::rayCast(const btVector3& start, const btVector3& end, ComponentBody* ignored, const unsigned short group, const unsigned short mask){
    if(ignored){
        Physics::removeRigidBody(*ignored);
    }
    vector<RayCastResult> result = _rayCastInternal(start, end, group, mask);
    if(ignored){
        Physics::addRigidBody(*ignored);
    }
    return result;
}
vector<RayCastResult> Physics::rayCast(const btVector3& start, const btVector3& end, vector<ComponentBody*>& ignored, const unsigned short group, const unsigned short mask){
    for(auto& object:ignored){
        Physics::removeRigidBody(*object);
    }
    vector<RayCastResult> result = _rayCastInternal(start, end, group, mask);
    for(auto& object:ignored){
        Physics::addRigidBody(*object);
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
                return Physics::rayCast(start_, end_, body, group, mask);
            }
        }
    }
    return Physics::rayCast(start_, end_, nullptr, group, mask);
 }
vector<RayCastResult> Physics::rayCast(const glm::vec3& start, const glm::vec3& end ,vector<Entity>& ignored, const unsigned short group, const unsigned short mask){
    const btVector3 start_ = Math::btVectorFromGLM(start);
    const btVector3 end_ = Math::btVectorFromGLM(end);
    vector<ComponentBody*> objs;
    for(auto& o : ignored){
        ComponentBody* body = o.getComponent<ComponentBody>();
        if(body){
            if (body->hasPhysics()) {
                objs.push_back(body);
            }
        }
    }
    return Physics::rayCast(start_, end_, objs, group, mask);
}





RayCastResult Physics::rayCastNearest(const btVector3& start, const btVector3& end, ComponentBody* ignored, const unsigned short group, const unsigned short mask) {
    if (ignored) {
        Physics::removeRigidBody(*ignored);
    }
    RayCastResult result = _rayCastInternal_Nearest(start, end, group, mask);
    if (ignored) {
        Physics::addRigidBody(*ignored);
    }
    return result;
}
RayCastResult Physics::rayCastNearest(const btVector3& start, const btVector3& end, vector<ComponentBody*>& ignored, const unsigned short group, const unsigned short mask) {
    for (auto& object : ignored) {
        Physics::removeRigidBody(*object);
    }
    RayCastResult result = _rayCastInternal_Nearest(start, end, group, mask);
    for (auto& object : ignored) {
        Physics::addRigidBody(*object);
    }
    return result;
}
RayCastResult Physics::rayCastNearest(const glm::vec3& start, const glm::vec3& end, Entity* ignored, const unsigned short group, const unsigned short mask) {
    const btVector3 start_ = Math::btVectorFromGLM(start);
    const btVector3 end_ = Math::btVectorFromGLM(end);
    if (ignored) {
        ComponentBody* body = ignored->getComponent<ComponentBody>();
        if (body) {
            if (body->hasPhysics()) {
                return Physics::rayCastNearest(start_, end_, body, group, mask);
            }
        }
    }
    return Physics::rayCastNearest(start_, end_, nullptr, group, mask);
}
RayCastResult Physics::rayCastNearest(const glm::vec3& start, const glm::vec3& end, vector<Entity>& ignored, const unsigned short group, const unsigned short mask) {
    const btVector3 start_ = Math::btVectorFromGLM(start);
    const btVector3 end_ = Math::btVectorFromGLM(end);
    vector<ComponentBody*> objs;
    for (auto& o : ignored) {
        ComponentBody* body = o.getComponent<ComponentBody>();
        if (body) {
            if (body->hasPhysics()) {
                objs.push_back(body);
            }
        }
    }
    return Physics::rayCastNearest(start_, end_, objs, group, mask);
}