#define BT_THREADSAFE 1
#include <core/engine/physics/Engine_Physics.h>
//#include <core/engine/physics/World.h>
#include <core/engine/physics/DebugDrawer.h>

#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/renderer/Renderer.h>
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
#include <BulletCollision/CollisionShapes/btTriangleShape.h>
#include <BulletCollision/CollisionShapes/btTriangleInfoMap.h>
#include <btBulletCollisionCommon.h>
#include <BulletCollision/NarrowPhaseCollision/btRaycastCallback.h>
#include <BulletCollision/CollisionDispatch/btInternalEdgeUtility.h>

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


void processManifoldContact(btManifoldPoint& cp, btCollisionObject* A, btCollisionObject* B, btCollisionShape* childA, btCollisionShape* childB, const btCollisionShape* shapeA, const btCollisionShape* shapeB) {
    ComponentBody* a_               = static_cast<ComponentBody*>(A->getUserPointer());
    ComponentBody* b_               = static_cast<ComponentBody*>(B->getUserPointer());
    if (a_ && b_) {
        glm::vec3 ptA               = Math::btVectorToGLM(cp.getPositionWorldOnA());
        glm::vec3 ptB               = Math::btVectorToGLM(cp.getPositionWorldOnB());
        glm::vec3 normalOnB         = Math::btVectorToGLM(cp.m_normalWorldOnB);
        glm::vec3 localPositionA    = Math::btVectorToGLM(cp.m_localPointA);
        glm::vec3 localPositionB    = Math::btVectorToGLM(cp.m_localPointB);
        glm::vec3 normalA           = -glm::normalize(ptB - ptA);
        glm::vec3 normalB           = -glm::normalize(ptA - ptB);
        

        if (shapeA && shapeA->getShapeType() == TRIANGLE_SHAPE_PROXYTYPE) {
            const btTriangleShape* triA = static_cast<const btTriangleShape*>(shapeA);
            btVector3 normal = (triA->m_vertices1[1] - triA->m_vertices1[0]).cross(triA->m_vertices1[2] - triA->m_vertices1[0]);
            normal.normalize();
            normalA = Math::btVectorToGLM(normal);
        }
        if (shapeB && shapeB->getShapeType() == TRIANGLE_SHAPE_PROXYTYPE) {
            const btTriangleShape* triB = static_cast<const btTriangleShape*>(shapeB);
            btVector3 normal = (triB->m_vertices1[1] - triB->m_vertices1[0]).cross(triB->m_vertices1[2] - triB->m_vertices1[0]);
            normal.normalize();
            normalB = Math::btVectorToGLM(normal);
        }

        CollisionCallbackEventData dataA(*a_, *b_, ptA, ptB, normalOnB, localPositionA, localPositionB, normalA, normalB);
        dataA.ownerCollisionObj = A;
        dataA.otherCollisionObj = B;
        CollisionCallbackEventData dataB(*b_, *a_, ptB, ptA, normalOnB, localPositionB, localPositionA, normalB, normalA);
        dataB.ownerCollisionObj = B;
        dataB.otherCollisionObj = A;

        if (childA) {
            auto& modelInstance = *static_cast<ModelInstance*>(childA->getUserPointer());
            dataA.ownerModelInstanceIndex = modelInstance.index();
            dataB.otherModelInstanceIndex = modelInstance.index();
        }
        if (childB) {
            auto& modelInstance = *static_cast<ModelInstance*>(childB->getUserPointer());
            dataA.otherModelInstanceIndex = modelInstance.index();
            dataB.ownerModelInstanceIndex = modelInstance.index();
        }

        a_->collisionResponse(dataA);
        b_->collisionResponse(dataB);

        cp.setDistance((btScalar)9999999999999.0); //hacky way of saying "dont process this again"
    }

}

bool CustomMaterialContactAddedCallback(btManifoldPoint& cp, const btCollisionObjectWrapper* colObj0Wrap, int partId0, int index0, const btCollisionObjectWrapper* colObj1Wrap, int partId1, int index1) {
    btCollisionShape* childShapeA       = nullptr;
    btCollisionShape* childShapeB       = nullptr;
    btCollisionObjectWrapper* ARoot     = const_cast<btCollisionObjectWrapper*>(colObj0Wrap);
    btCollisionObjectWrapper* BRoot     = const_cast<btCollisionObjectWrapper*>(colObj1Wrap);
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
        if (ARootPrev) {
            childShapeA = const_cast<btCollisionShape*>(ARootPrev->getCollisionShape());
        }
    }
    if (BRoot->getCollisionShape()->getShapeType() == BroadphaseNativeTypes::COMPOUND_SHAPE_PROXYTYPE) {
        if (BRootPrev) {
            childShapeB = const_cast<btCollisionShape*>(BRootPrev->getCollisionShape());
        }
    }

    if (cp.getDistance() < 0.0f) {
        //btAdjustInternalEdgeContacts(cp, colObj1Wrap, colObj0Wrap, partId1, index1);
        processManifoldContact(
            cp,
            const_cast<btCollisionObject*>(colObj0Wrap->getCollisionObject()),
            const_cast<btCollisionObject*>(colObj1Wrap->getCollisionObject()),
            childShapeA,
            childShapeB,
            colObj0Wrap->m_shape,
            colObj1Wrap->m_shape
        );
    }
    return true;
}

priv::PhysicsManager* physicsManager = nullptr;

priv::PhysicsManager::PhysicsManager(){ 
    physicsManager          = this;
}
priv::PhysicsManager::~PhysicsManager(){ 
    cleanup();
}
void priv::PhysicsManager::cleanup() {
    auto& world = *m_Data.m_World;
    int collisionObjCount = world.getNumCollisionObjects();
    for (int i = 0; i < collisionObjCount; ++i) {
        btCollisionObject* obj = world.getCollisionObjectArray()[i];
        if (obj) {
            btRigidBody* body = btRigidBody::upcast(obj);
            if (body) {
                auto* motionState = body->getMotionState();
                SAFE_DELETE(motionState);
            }
            world.removeCollisionObject(obj);
            SAFE_DELETE(obj);
        }
    }
}
void priv::PhysicsManager::debug_draw_line(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color) {
    priv::PhysicsManager::debug_draw_line(start, end, color.r, color.g, color.b, color.a);
}
void priv::PhysicsManager::debug_draw_line(const glm::vec3& start, const glm::vec3& end, const float r, const float g, const float b, const float a) {
    m_Data.m_World->getDebugDrawer()->drawLine(Math::btVectorFromGLM(start), Math::btVectorFromGLM(end), btVector4(r, g, b, a));
}
void priv::PhysicsManager::_init(){
    m_Data.m_DebugDrawer.init();

    gContactAddedCallback = CustomMaterialContactAddedCallback;
}
void priv::PhysicsManager::_update(const float dt, int maxSubSteps, float fixedTimeStep){ 
    if (m_Paused)
        return;
    m_Data.m_World->stepSimulation(static_cast<btScalar>(dt), maxSubSteps, static_cast<btScalar>(fixedTimeStep));

    for (int i = 0; i < m_Data.m_Dispatcher->getNumManifolds(); ++i) {
        btPersistentManifold& contactManifold = *m_Data.m_Dispatcher->getManifoldByIndexInternal(i);

        for (int j = 0; j < contactManifold.getNumContacts(); ++j) {
            btManifoldPoint& cp = contactManifold.getContactPoint(j);
            if (cp.getDistance() < 0.0f) {
                processManifoldContact(
                    cp,
                    const_cast<btCollisionObject*>(contactManifold.getBody0()),
                    const_cast<btCollisionObject*>(contactManifold.getBody1()),
                    nullptr,
                    nullptr,
                    nullptr,
                    nullptr
                );
            }
        }
    }
}
void priv::PhysicsManager::_render(const Camera& camera){
    m_Data.m_World->debugDrawWorld();
    const glm::vec3 camPos = camera.getPosition();
    const glm::mat4 model = glm::mat4(1.0f);
    Engine::Renderer::sendUniformMatrix4("Model", model);
    Engine::Renderer::sendUniformMatrix4("VP", camera.getViewProjection());
    m_Data.m_DebugDrawer.drawAccumulatedLines();
    m_Data.m_DebugDrawer.postRender();
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
    physicsManager->m_Data.m_World->setGravity(btVector3(x,y,z));
}
void Physics::setGravity(const glm::vec3& gravity){ 
    Physics::setGravity(gravity.x,gravity.y,gravity.z); 
}
void Physics::addRigidBody(btRigidBody* rigidBody, short group, short mask){ 
    auto& data = physicsManager->m_Data;
    int collisionObjCount = data.m_World->getNumCollisionObjects();
    for (int i = 0; i < collisionObjCount; ++i) {
        btRigidBody* body = btRigidBody::upcast(data.m_World->getCollisionObjectArray()[i]);
        if (body) {
            if (body == rigidBody) {
                return;
            }
        }
    }
    data.m_World->addRigidBody(rigidBody, group, mask);
}
void Physics::addRigidBody(btRigidBody* rigidBody){ 
    auto& data = physicsManager->m_Data;
    int collisionObjCount = data.m_World->getNumCollisionObjects();
    for (int i = 0; i < collisionObjCount; ++i) {
        btRigidBody* body = btRigidBody::upcast(data.m_World->getCollisionObjectArray()[i]);
        if (body) {
            if (body == rigidBody) {
                return;
            }
        }
    }
    data.m_World->addRigidBody(rigidBody);
}
void Physics::removeRigidBody(btRigidBody* rigidBody){ 
    auto& data = physicsManager->m_Data;
    int collisionObjCount = data.m_World->getNumCollisionObjects();
    for (int i = 0; i < collisionObjCount; ++i) {
        btRigidBody* body = btRigidBody::upcast(data.m_World->getCollisionObjectArray()[i]);
        if (body) {
            if (body == rigidBody) {
                for (int i = body->getNumConstraintRefs() - 1; i >= 0; i--) {
                    btTypedConstraint* con = body->getConstraintRef(i);
                    data.m_World->removeConstraint(con);
                }
                data.m_World->removeRigidBody(rigidBody);
                return;
            }
        }
    }
}
void Physics::removeCollisionObject(btCollisionObject* object) {
    physicsManager->m_Data.m_World->removeCollisionObject(object);
}
void Physics::updateRigidBody(btRigidBody* rigidBody){ 
    physicsManager->m_Data.m_World->updateSingleAabb(rigidBody);
}
void Physics::addRigidBody(ComponentBody& body) {
    Physics::addRigidBody(const_cast<btRigidBody*>(&body.getBtBody()), body.getCollisionGroup(), body.getCollisionMask());
}
void Physics::removeRigidBody(ComponentBody& body) {
    Physics::removeRigidBody(&const_cast<btRigidBody&>(body.getBtBody()));
}
void Physics::addRigidBodyThreadSafe(btRigidBody* body, short group, short mask) {
    std::lock_guard<std::mutex> lock(physicsManager->m_Mutex);
    Physics::addRigidBody(body, group, mask);
}
void Physics::addRigidBodyThreadSafe(btRigidBody* body) {
    std::lock_guard<std::mutex> lock(physicsManager->m_Mutex);
    Physics::addRigidBody(body);
}
void Physics::removeRigidBodyThreadSafe(btRigidBody* body) {
    std::lock_guard<std::mutex> lock(physicsManager->m_Mutex);
    Physics::removeRigidBody(body);
}
void Physics::updateRigidBodyThreadSafe(btRigidBody* body) {
    std::lock_guard<std::mutex> lock(physicsManager->m_Mutex);
    Physics::updateRigidBody(body);
}
void Physics::addRigidBodyThreadSafe(ComponentBody& body){
    Physics::addRigidBodyThreadSafe(const_cast<btRigidBody*>(&body.getBtBody()), body.getCollisionGroup(), body.getCollisionMask());
}
void Physics::removeRigidBodyThreadSafe(ComponentBody& body) {
    Physics::removeRigidBodyThreadSafe(&const_cast<btRigidBody&>(body.getBtBody()));
}
void Physics::removeCollisionObjectThreadSafe(btCollisionObject* object) {
    std::lock_guard<std::mutex> lock(physicsManager->m_Mutex);
    physicsManager->m_Data.m_World->removeCollisionObject(object);
}


RayCastResult _rayCastInternal_Nearest(const btVector3& start, const btVector3& end, const unsigned short group, const unsigned short mask) {
    btCollisionWorld::ClosestRayResultCallback RayCallback(start, end);
    RayCallback.m_collisionFilterMask = mask;
    RayCallback.m_collisionFilterGroup = group;

    physicsManager->m_Data.m_World->rayTest(start, end, RayCallback);
#ifdef _DEBUG
    physicsManager->m_Data.m_World->getDebugDrawer()->drawLine(start, end, btVector4(1, 1, 0, 1));
#endif

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

    physicsManager->m_Data.m_World->rayTest(start, end, RayCallback);
#ifdef _DEBUG
    physicsManager->m_Data.m_World->getDebugDrawer()->drawLine(start, end, btVector4(1, 0.5, 0, 1));
#endif

    vector<RayCastResult> result;
    if (RayCallback.hasHit()) {
        auto& pts     = RayCallback.m_hitPointWorld;
        auto& normals = RayCallback.m_hitNormalWorld;

        for (int i = 0; i < pts.size(); ++i) { 
            const glm::vec3 hitPoint  = Math::btVectorToGLM(RayCallback.m_hitPointWorld[i]);
            const glm::vec3 hitNormal = Math::btVectorToGLM(RayCallback.m_hitNormalWorld[i]);

            RayCastResult res;
            res.hitPosition = hitPoint;
            res.hitNormal   = hitNormal;
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