#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/physics/Engine_Physics.h>
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
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorldMt.h>
#include <BulletDynamics/Dynamics/btSimulationIslandManagerMt.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolverMt.h>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

//yes, this code is needed or stuff breaks. find out why
#include <SFML/OpenGL.hpp>
#include <GL/freeglut.h>

using namespace Engine;

Engine::view_ptr<Engine::priv::PhysicsManager> Engine::priv::PhysicsManager::PHYSICS_MANAGER = nullptr;

void processManifoldContact(btManifoldPoint& cp, btCollisionObject* A, btCollisionObject* B, btCollisionShape* childA, btCollisionShape* childB, const btCollisionShape* shapeA, const btCollisionShape* shapeB) {
    ComponentBody* a_               = (ComponentBody*)A->getUserPointer();
    ComponentBody* b_               = (ComponentBody*)B->getUserPointer();
    if (a_ && b_) {
        glm::vec3 ptA               = Math::btVectorToGLM(cp.getPositionWorldOnA());
        glm::vec3 ptB               = Math::btVectorToGLM(cp.getPositionWorldOnB());
        glm::vec3 normalOnB         = Math::btVectorToGLM(cp.m_normalWorldOnB);
        glm::vec3 localPositionA    = Math::btVectorToGLM(cp.m_localPointA);
        glm::vec3 localPositionB    = Math::btVectorToGLM(cp.m_localPointB);
        glm::vec3 normalA           = -glm::normalize(ptB - ptA);
        glm::vec3 normalB           = -glm::normalize(ptA - ptB);      

        if (shapeA && shapeA->getShapeType() == TRIANGLE_SHAPE_PROXYTYPE) {
            const btTriangleShape* triA = (const btTriangleShape*)shapeA;
            btVector3 normal = (triA->m_vertices1[1] - triA->m_vertices1[0]).cross(triA->m_vertices1[2] - triA->m_vertices1[0]);
            normal.normalize();
            normalA = Math::btVectorToGLM(normal);
        }
        if (shapeB && shapeB->getShapeType() == TRIANGLE_SHAPE_PROXYTYPE) {
            const btTriangleShape* triB = (const btTriangleShape*)shapeB;
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
            auto& modelInstance = *(ModelInstance*)childA->getUserPointer();
            dataA.ownerModelInstanceIndex = modelInstance.index();
            dataB.otherModelInstanceIndex = modelInstance.index();
        }
        if (childB) {
            auto& modelInstance = *(ModelInstance*)childB->getUserPointer();
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
        processManifoldContact(cp, const_cast<btCollisionObject*>(colObj0Wrap->getCollisionObject()), const_cast<btCollisionObject*>(colObj1Wrap->getCollisionObject()), childShapeA, childShapeB, colObj0Wrap->m_shape, colObj1Wrap->m_shape);
    }
    return true;
}


priv::PhysicsManager::PhysicsManager(){ 
    PHYSICS_MANAGER = this;
}
priv::PhysicsManager::~PhysicsManager(){ 
    cleanup();
}
void priv::PhysicsManager::cleanup() {
    auto& pipeline        = *m_Pipeline.m_World;
    int collisionObjCount = pipeline.getNumCollisionObjects();
    for (int i = 0; i < collisionObjCount; ++i) {
        btCollisionObject* obj = pipeline.getCollisionObjectArray()[i];
        if (obj) {
            btRigidBody* body  = btRigidBody::upcast(obj);
            if (body) {
                auto* motionState = body->getMotionState();
                SAFE_DELETE(motionState);
            }
            pipeline.removeCollisionObject(obj);
            SAFE_DELETE(obj);
        }
    }
}
void priv::PhysicsManager::debug_draw_line(const glm::vec3& start, const glm::vec3& end, const glm::vec4& color) {
    priv::PhysicsManager::debug_draw_line(start, end, color.r, color.g, color.b, color.a);
}
void priv::PhysicsManager::debug_draw_line(const glm::vec3& start, const glm::vec3& end, float r, float g, float b, float a) {
    m_Pipeline.m_World->getDebugDrawer()->drawLine(Math::btVectorFromGLM(start), Math::btVectorFromGLM(end), btVector4(r, g, b, a));
}
void priv::PhysicsManager::_init(){
    m_Pipeline.m_DebugDrawer.init();

    gContactAddedCallback = CustomMaterialContactAddedCallback;
}
bool priv::PhysicsManager::add_rigid_body(btRigidBody* rigidBody, short group, short mask, bool doGroupAndMask) noexcept {
    auto& physicsWorld = *m_Pipeline.m_World;
    for (int i = 0; i < physicsWorld.getNumCollisionObjects(); ++i) {
        btRigidBody* btRigidBodyUpCast = btRigidBody::upcast(physicsWorld.getCollisionObjectArray()[i]);
        if (btRigidBodyUpCast && btRigidBodyUpCast == rigidBody) {
            return false;
        }
    }
    (doGroupAndMask) ? physicsWorld.addRigidBody(rigidBody, group, mask) : physicsWorld.addRigidBody(rigidBody);
    return true;
}
void priv::PhysicsManager::_update(const float dt, int maxSubSteps, float fixedTimeStep){ 
    if (m_Paused) {
        return;
    }
    m_Pipeline.update(dt);
    m_Pipeline.m_World->stepSimulation((btScalar)dt, maxSubSteps, (btScalar)fixedTimeStep);

    for (int i = 0; i < m_Pipeline.m_Dispatcher->getNumManifolds(); ++i) {
        btPersistentManifold& contactManifold = *m_Pipeline.m_Dispatcher->getManifoldByIndexInternal(i);

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
    m_Pipeline.m_World->debugDrawWorld();
    const glm::vec3 camPos = camera.getPosition();
    const glm::mat4 model  = glm::mat4(1.0f);
    Engine::Renderer::sendUniformMatrix4("Model", model);
    Engine::Renderer::sendUniformMatrix4("VP", camera.getViewProjection());
    m_Pipeline.m_DebugDrawer.drawAccumulatedLines();
    m_Pipeline.m_DebugDrawer.postRender();
}
void Physics::setNumberOfStepsPerFrame(unsigned int numSteps) {
    Engine::priv::PhysicsManager::PHYSICS_MANAGER->m_NumberOfStepsPerFrame = glm::max(1U, numSteps);
}
unsigned int Physics::getNumberOfStepsPerFrame() {
    return Engine::priv::PhysicsManager::PHYSICS_MANAGER->m_NumberOfStepsPerFrame;
}
void Physics::pause(bool b){ 
    Engine::priv::PhysicsManager::PHYSICS_MANAGER->m_Paused = b;
}
void Physics::unpause(){ 
    Engine::priv::PhysicsManager::PHYSICS_MANAGER->m_Paused = false;
}
void Physics::setGravity(float x, float y, float z){ 
    Engine::priv::PhysicsManager::PHYSICS_MANAGER->m_Pipeline.m_World->setGravity(btVector3(x,y,z));
}
void Physics::setGravity(const glm::vec3& gravity){ 
    Physics::setGravity(gravity.x,gravity.y,gravity.z); 
}
bool Physics::addRigidBody(btRigidBody* rigidBody, short group, short mask){
    return Engine::priv::PhysicsManager::PHYSICS_MANAGER->add_rigid_body(rigidBody, group, mask, true);
}
bool Physics::addRigidBody(btRigidBody* rigidBody){
    return Engine::priv::PhysicsManager::PHYSICS_MANAGER->add_rigid_body(rigidBody, 0, 0, false);
}
bool Physics::removeRigidBody(btRigidBody* rigidBody){
    auto& physicsWorld    = *Engine::priv::PhysicsManager::PHYSICS_MANAGER->m_Pipeline.m_World;
    int collisionObjCount = physicsWorld.getNumCollisionObjects();
    for (int i = 0; i < collisionObjCount; ++i) {
        btRigidBody* btRigidBodyUpCast = btRigidBody::upcast(physicsWorld.getCollisionObjectArray()[i]);
        if (btRigidBodyUpCast && btRigidBodyUpCast == rigidBody) {
            for (int i = btRigidBodyUpCast->getNumConstraintRefs() - 1; i >= 0; --i) {
                btTypedConstraint* constraint = btRigidBodyUpCast->getConstraintRef(i);
                physicsWorld.removeConstraint(constraint);
            }
            physicsWorld.removeRigidBody(rigidBody);
            return true;
        }
    }
    return false;
}
bool Physics::removeCollisionObject(btCollisionObject* object) {
    Engine::priv::PhysicsManager::PHYSICS_MANAGER->m_Pipeline.m_World->removeCollisionObject(object);
    return true;
}
void Physics::updateRigidBody(btRigidBody* rigidBody){ 
    Engine::priv::PhysicsManager::PHYSICS_MANAGER->m_Pipeline.m_World->updateSingleAabb(rigidBody);
}
bool Physics::addRigidBody(Entity entity) {
    ComponentBody* body = entity.getComponent<ComponentBody>();
    if (body && body->hasPhysics()) {
        return Physics::addRigidBody(*body);
    }
    return false;
}
bool Physics::addRigidBody(ComponentBody& body) {
    return Physics::addRigidBody(&body.getBtBody(), body.getCollisionGroup(), body.getCollisionMask());
}
bool Physics::removeRigidBody(Entity entity) {
    ComponentBody* body = entity.getComponent<ComponentBody>();
    if (body && body->hasPhysics()) {
        return Physics::removeRigidBody(*body);
    }
    return false;
}
bool Physics::removeRigidBody(ComponentBody& body) {
    return Physics::removeRigidBody(&body.getBtBody());
}
bool Physics::addRigidBodyThreadSafe(Entity entity) {
    std::lock_guard lock(Engine::priv::PhysicsManager::PHYSICS_MANAGER->m_Mutex);
    return Physics::addRigidBody(entity);
}
bool Physics::addRigidBodyThreadSafe(btRigidBody* body, short group, short mask) {
    std::lock_guard lock(Engine::priv::PhysicsManager::PHYSICS_MANAGER->m_Mutex);
    return Physics::addRigidBody(body, group, mask);
}
bool Physics::addRigidBodyThreadSafe(btRigidBody* body) {
    std::lock_guard lock(Engine::priv::PhysicsManager::PHYSICS_MANAGER->m_Mutex);
    return Physics::addRigidBody(body);
}
bool Physics::removeRigidBodyThreadSafe(Entity entity) {
    std::lock_guard lock(Engine::priv::PhysicsManager::PHYSICS_MANAGER->m_Mutex);
    return Physics::removeRigidBody(entity);
}
bool Physics::removeRigidBodyThreadSafe(btRigidBody* body) {
    std::lock_guard lock(Engine::priv::PhysicsManager::PHYSICS_MANAGER->m_Mutex);
    return Physics::removeRigidBody(body);
}
void Physics::updateRigidBodyThreadSafe(btRigidBody* body) {
    std::lock_guard lock(Engine::priv::PhysicsManager::PHYSICS_MANAGER->m_Mutex);
    Physics::updateRigidBody(body);
}
bool Physics::addRigidBodyThreadSafe(ComponentBody& body){
    return Physics::addRigidBodyThreadSafe(&body.getBtBody(), body.getCollisionGroup(), body.getCollisionMask());
}
bool Physics::removeRigidBodyThreadSafe(ComponentBody& body) {
    return Physics::removeRigidBodyThreadSafe(&body.getBtBody());
}
bool Physics::removeCollisionObjectThreadSafe(btCollisionObject* object) {
    std::lock_guard lock(Engine::priv::PhysicsManager::PHYSICS_MANAGER->m_Mutex);
    return Physics::removeCollisionObject(object);
}


RayCastResult internal_ray_cast_nearest(btVector3& start, btVector3& end, unsigned short group, unsigned short mask) {
    btCollisionWorld::ClosestRayResultCallback RayCallback(start, end);
    RayCallback.m_collisionFilterMask  = mask;
    RayCallback.m_collisionFilterGroup = group;

    Engine::priv::PhysicsManager::PHYSICS_MANAGER->m_Pipeline.m_World->rayTest(start, end, RayCallback);
#ifdef _DEBUG
    Engine::priv::PhysicsManager::PHYSICS_MANAGER->m_Pipeline.m_World->getDebugDrawer()->drawLine(start, end, btVector4(1.0f, 0.5f, 0.0f, 1.0f));
#endif

    RayCastResult result;
    if (RayCallback.hasHit()) {
        auto& pts           = RayCallback.m_hitPointWorld;
        auto& normals       = RayCallback.m_hitNormalWorld;

        glm::vec3 hitPoint  = Math::btVectorToGLM(RayCallback.m_hitPointWorld);
        glm::vec3 hitNormal = Math::btVectorToGLM(RayCallback.m_hitNormalWorld);

        result.hitPosition  = hitPoint;
        result.hitNormal    = hitNormal;
    }
    return result;
}
std::vector<RayCastResult> internal_ray_cast(btVector3& start, btVector3& end, unsigned short group, unsigned short mask) {
    btCollisionWorld::AllHitsRayResultCallback RayCallback(start, end);
    RayCallback.m_collisionFilterMask  = mask;
    RayCallback.m_collisionFilterGroup = group;

    Engine::priv::PhysicsManager::PHYSICS_MANAGER->m_Pipeline.m_World->rayTest(start, end, RayCallback);
#ifdef _DEBUG
    Engine::priv::PhysicsManager::PHYSICS_MANAGER->m_Pipeline.m_World->getDebugDrawer()->drawLine(start, end, btVector4(1, 0.5, 0, 1));
#endif

    std::vector<RayCastResult> result;
    if (RayCallback.hasHit()) {
        auto& pts     = RayCallback.m_hitPointWorld;
        auto& normals = RayCallback.m_hitNormalWorld;
        result.reserve(pts.size());
        for (int i = 0; i < pts.size(); ++i) { 
            const glm::vec3 hitPoint  = Math::btVectorToGLM(RayCallback.m_hitPointWorld[i]);
            const glm::vec3 hitNormal = Math::btVectorToGLM(RayCallback.m_hitNormalWorld[i]);

            RayCastResult res;
            res.hitPosition = hitPoint;
            res.hitNormal   = hitNormal;
            result.emplace_back(std::move(res));
        }
    }
    return result;
}
std::vector<RayCastResult> Physics::rayCast(btVector3& start, btVector3& end, ComponentBody* ignored, unsigned short group, unsigned short mask){
    if(ignored){
        Physics::removeRigidBody(*ignored);
    }
    std::vector<RayCastResult> result = internal_ray_cast(start, end, group, mask);
    if(ignored){
        Physics::addRigidBody(*ignored);
    }
    return result;
}
std::vector<RayCastResult> Physics::rayCast(btVector3& start, btVector3& end, std::vector<ComponentBody*>& ignored, unsigned short group, unsigned short mask){
    for(auto& body : ignored){
        Physics::removeRigidBody(*body);
    }
    std::vector<RayCastResult> result = internal_ray_cast(start, end, group, mask);
    for(auto& body : ignored){
        Physics::addRigidBody(*body);
    }
    return result;
 }
std::vector<RayCastResult> Physics::rayCast(glm::vec3& start, glm::vec3& end, Entity* ignored, unsigned short group, unsigned short mask){
    btVector3 start_ = Math::btVectorFromGLM(start);
    btVector3 end_   = Math::btVectorFromGLM(end);
    if (ignored) {
        ComponentBody* body = ignored->getComponent<ComponentBody>();
        if (body && body->hasPhysics()) {
            return Physics::rayCast(start_, end_, body, group, mask); 
        }
    }
    return Physics::rayCast(start_, end_, nullptr, group, mask);
 }
std::vector<RayCastResult> Physics::rayCast(glm::vec3& start, glm::vec3& end , std::vector<Entity>& ignored, unsigned short group, unsigned short mask){
    btVector3 start_ = Math::btVectorFromGLM(start);
    btVector3 end_   = Math::btVectorFromGLM(end);
    std::vector<ComponentBody*> objs;
    for(auto& ent : ignored){
        ComponentBody* body = ent.getComponent<ComponentBody>();
        if(body && body->hasPhysics()) {
            objs.emplace_back(body);
        }
    }
    return Physics::rayCast(start_, end_, objs, group, mask);
}

RayCastResult Physics::rayCastNearest(btVector3& start, btVector3& end, ComponentBody* ignored, unsigned short group, unsigned short mask) {
    if (ignored) {
        Physics::removeRigidBody(*ignored);
    }
    RayCastResult result = internal_ray_cast_nearest(start, end, group, mask);
    if (ignored) {
        Physics::addRigidBody(*ignored);
    }
    return result;
}
RayCastResult Physics::rayCastNearest(btVector3& start, btVector3& end, std::vector<ComponentBody*>& ignored, unsigned short group, unsigned short mask) {
    for (auto& object : ignored) {
        Physics::removeRigidBody(*object);
    }
    RayCastResult result = internal_ray_cast_nearest(start, end, group, mask);
    for (auto& object : ignored) {
        Physics::addRigidBody(*object);
    }
    return result;
}
RayCastResult Physics::rayCastNearest(glm::vec3& start, glm::vec3& end, Entity* ignored, unsigned short group, unsigned short mask) {
    btVector3 start_ = Math::btVectorFromGLM(start);
    btVector3 end_   = Math::btVectorFromGLM(end);
    if (ignored) {
        ComponentBody* body = ignored->getComponent<ComponentBody>();
        if (body && body->hasPhysics()) {
            return Physics::rayCastNearest(start_, end_, body, group, mask);
        }
    }
    return Physics::rayCastNearest(start_, end_, nullptr, group, mask);
}
RayCastResult Physics::rayCastNearest(glm::vec3& start, glm::vec3& end, std::vector<Entity>& ignored, unsigned short group, unsigned short mask) {
    btVector3 start_ = Math::btVectorFromGLM(start);
    btVector3 end_   = Math::btVectorFromGLM(end);
    std::vector<ComponentBody*> objs;
    for (auto& o : ignored) {
        ComponentBody* body = o.getComponent<ComponentBody>();
        if (body && body->hasPhysics()) {
            objs.emplace_back(body);
        }
    }
    return Physics::rayCastNearest(start_, end_, objs, group, mask);
}