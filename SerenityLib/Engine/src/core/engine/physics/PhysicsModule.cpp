#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/physics/PhysicsModule.h>
#include <core/engine/physics/DebugDrawer.h>

#include <core/engine/renderer/Renderer.h>
#include <core/engine/scene/Camera.h>
// ecs
#include <ecs/ComponentModel.h>
#include <ecs/ComponentBody.h>

#include <BulletCollision/CollisionShapes/btCollisionShape.h>
#include <BulletCollision/CollisionShapes/btTriangleShape.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <BulletCollision/NarrowPhaseCollision/btRaycastCallback.h>
#include <BulletCollision/CollisionDispatch/btInternalEdgeUtility.h>

Engine::view_ptr<Engine::priv::PhysicsModule> Engine::priv::PhysicsModule::PHYSICS_MANAGER = nullptr;

void processManifoldContact(btManifoldPoint& cp, btCollisionObject* A, btCollisionObject* B, btCollisionShape* childA, btCollisionShape* childB, const btCollisionShape* shapeA, const btCollisionShape* shapeB) {
    ComponentBody* a_               = (ComponentBody*)A->getUserPointer();
    ComponentBody* b_               = (ComponentBody*)B->getUserPointer();
    if (a_ && b_) {
        glm::vec3 ptA               = Engine::Math::btVectorToGLM(cp.getPositionWorldOnA());
        glm::vec3 ptB               = Engine::Math::btVectorToGLM(cp.getPositionWorldOnB());
        glm::vec3 normalOnB         = Engine::Math::btVectorToGLM(cp.m_normalWorldOnB);
        glm::vec3 localPositionA    = Engine::Math::btVectorToGLM(cp.m_localPointA);
        glm::vec3 localPositionB    = Engine::Math::btVectorToGLM(cp.m_localPointB);
        glm::vec3 normalA           = -glm::normalize(ptB - ptA);
        glm::vec3 normalB           = -glm::normalize(ptA - ptB);      

        if (shapeA && shapeA->getShapeType() == TRIANGLE_SHAPE_PROXYTYPE) {
            const btTriangleShape* triA = (const btTriangleShape*)shapeA;
            btVector3 normal = (triA->m_vertices1[1] - triA->m_vertices1[0]).cross(triA->m_vertices1[2] - triA->m_vertices1[0]);
            normal.normalize();
            normalA = Engine::Math::btVectorToGLM(normal);
        }
        if (shapeB && shapeB->getShapeType() == TRIANGLE_SHAPE_PROXYTYPE) {
            const btTriangleShape* triB = (const btTriangleShape*)shapeB;
            btVector3 normal = (triB->m_vertices1[1] - triB->m_vertices1[0]).cross(triB->m_vertices1[2] - triB->m_vertices1[0]);
            normal.normalize();
            normalB = Engine::Math::btVectorToGLM(normal);
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
    btCollisionShape*         childShapeA  = nullptr;
    btCollisionShape*         childShapeB  = nullptr;
    btCollisionObjectWrapper* ARoot        = const_cast<btCollisionObjectWrapper*>(colObj0Wrap);
    btCollisionObjectWrapper* BRoot        = const_cast<btCollisionObjectWrapper*>(colObj1Wrap);
    btCollisionObjectWrapper* ARootPrev    = nullptr;
    btCollisionObjectWrapper* BRootPrev    = nullptr;
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

Engine::priv::PhysicsModule::PhysicsModule(){
    PHYSICS_MANAGER = this;
}
void Engine::priv::PhysicsModule::init(){
    m_Pipeline.m_DebugDrawer.init();
    gContactAddedCallback = CustomMaterialContactAddedCallback;
}
bool Engine::priv::PhysicsModule::add_rigid_body(btRigidBody* rigidBody, MaskType group, MaskType mask, bool doGroupAndMask) noexcept {
    auto& physicsWorld = *m_Pipeline.m_World;
    for (int i = 0; i < physicsWorld.getNumCollisionObjects(); ++i) {
        btRigidBody* btRigidBodyUpCast = btRigidBody::upcast(physicsWorld.getCollisionObjectArray()[i]);
        if (/*btRigidBodyUpCast &&*/ btRigidBodyUpCast == rigidBody) {
            return false;
        }
    }
    if (doGroupAndMask) {
        physicsWorld.addRigidBody(rigidBody, group, mask);
    }else{
        physicsWorld.addRigidBody(rigidBody);
    }
    return true;
}
void Engine::priv::PhysicsModule::preUpdate(const float dt) noexcept {
    m_Pipeline.m_World->performDiscreteCollisionDetection();
}
void Engine::priv::PhysicsModule::update(const float dt, int maxSubSteps, float fixedTimeStep){
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
void Engine::priv::PhysicsModule::render(const Camera& camera){
    const glm::vec3 camPos = camera.getPosition();
    m_Pipeline.m_World->debugDrawWorld();
    Engine::Renderer::sendUniformMatrix4("Model", glm::mat4(1.0f));
    Engine::Renderer::sendUniformMatrix4("VP", camera.getViewProjection());
    m_Pipeline.m_DebugDrawer.drawAccumulatedLines();
    m_Pipeline.m_DebugDrawer.postRender();
}
void Engine::Physics::updateDiscreteCollisionDetection() noexcept {
    Engine::priv::PhysicsModule::PHYSICS_MANAGER->m_Pipeline.m_World->performDiscreteCollisionDetection();
}
void Engine::Physics::setNumberOfStepsPerFrame(uint32_t numSteps) {
    Engine::priv::PhysicsModule::PHYSICS_MANAGER->m_NumberOfStepsPerFrame = glm::max(1U, numSteps);
}
uint32_t Engine::Physics::getNumberOfStepsPerFrame() {
    return Engine::priv::PhysicsModule::PHYSICS_MANAGER->m_NumberOfStepsPerFrame;
}
void Engine::Physics::pause(bool b){
    Engine::priv::PhysicsModule::PHYSICS_MANAGER->m_Paused = b;
}
void Engine::Physics::unpause(){
    Engine::priv::PhysicsModule::PHYSICS_MANAGER->m_Paused = false;
}
void Engine::Physics::setGravity(float x, float y, float z){
    Engine::priv::PhysicsModule::PHYSICS_MANAGER->m_Pipeline.m_World->setGravity(btVector3(x,y,z));
}
void Engine::Physics::setGravity(const glm::vec3& gravity){
    Physics::setGravity(gravity.x,gravity.y,gravity.z); 
}
bool Engine::Physics::addRigidBody(btRigidBody* rigidBody, MaskType group, MaskType mask){
    return Engine::priv::PhysicsModule::PHYSICS_MANAGER->add_rigid_body(rigidBody, group, mask, true);
}
bool Engine::Physics::addRigidBody(btRigidBody* rigidBody){
    return Engine::priv::PhysicsModule::PHYSICS_MANAGER->add_rigid_body(rigidBody, 0, 0, false);
}
bool Engine::Physics::removeRigidBody(btRigidBody* rigidBody){
    auto& physicsWorld    = *Engine::priv::PhysicsModule::PHYSICS_MANAGER->m_Pipeline.m_World;
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
bool Engine::Physics::removeCollisionObject(btCollisionObject* object) {
    Engine::priv::PhysicsModule::PHYSICS_MANAGER->m_Pipeline.m_World->removeCollisionObject(object);
    return true;
}
void Engine::Physics::updateRigidBody(btRigidBody* rigidBody){
    Engine::priv::PhysicsModule::PHYSICS_MANAGER->m_Pipeline.m_World->updateSingleAabb(rigidBody);
}
bool Engine::Physics::addRigidBody(Entity entity) {
    ComponentBody* body = entity.getComponent<ComponentBody>();
    if (body && body->hasPhysics()) {
        return Physics::addRigidBody(*body);
    }
    return false;
}
bool Engine::Physics::addRigidBody(ComponentBody& body) {
    return Physics::addRigidBody(&body.getBtBody(), body.getCollisionGroup(), body.getCollisionMask());
}
bool Engine::Physics::removeRigidBody(Entity entity) {
    ComponentBody* body = entity.getComponent<ComponentBody>();
    if (body && body->hasPhysics()) {
        return Physics::removeRigidBody(*body);
    }
    return false;
}
bool Engine::Physics::removeRigidBody(ComponentBody& body) {
    return Physics::removeRigidBody(&body.getBtBody());
}
bool Engine::Physics::addRigidBodyThreadSafe(Entity entity) {
    std::lock_guard lock(Engine::priv::PhysicsModule::PHYSICS_MANAGER->m_Mutex);
    return Physics::addRigidBody(entity);
}
bool Engine::Physics::addRigidBodyThreadSafe(btRigidBody* body, MaskType group, MaskType mask) {
    std::lock_guard lock(Engine::priv::PhysicsModule::PHYSICS_MANAGER->m_Mutex);
    return Physics::addRigidBody(body, group, mask);
}
bool Engine::Physics::addRigidBodyThreadSafe(btRigidBody* body) {
    std::lock_guard lock(Engine::priv::PhysicsModule::PHYSICS_MANAGER->m_Mutex);
    return Physics::addRigidBody(body);
}
bool Engine::Physics::removeRigidBodyThreadSafe(Entity entity) {
    std::lock_guard lock(Engine::priv::PhysicsModule::PHYSICS_MANAGER->m_Mutex);
    return Physics::removeRigidBody(entity);
}
bool Engine::Physics::removeRigidBodyThreadSafe(btRigidBody* body) {
    std::lock_guard lock(Engine::priv::PhysicsModule::PHYSICS_MANAGER->m_Mutex);
    return Physics::removeRigidBody(body);
}
void Engine::Physics::updateRigidBodyThreadSafe(btRigidBody* body) {
    std::lock_guard lock(Engine::priv::PhysicsModule::PHYSICS_MANAGER->m_Mutex);
    Physics::updateRigidBody(body);
}
bool Engine::Physics::addRigidBodyThreadSafe(ComponentBody& body){
    return Physics::addRigidBodyThreadSafe(&body.getBtBody(), body.getCollisionGroup(), body.getCollisionMask());
}
bool Engine::Physics::removeRigidBodyThreadSafe(ComponentBody& body) {
    return Physics::removeRigidBodyThreadSafe(&body.getBtBody());
}
bool Engine::Physics::removeCollisionObjectThreadSafe(btCollisionObject* object) {
    std::lock_guard lock(Engine::priv::PhysicsModule::PHYSICS_MANAGER->m_Mutex);
    return Physics::removeCollisionObject(object);
}


Engine::RayCastResult internal_ray_cast_nearest(const btVector3& start, const btVector3& end, MaskType group, MaskType mask) {
    btCollisionWorld::ClosestRayResultCallback RayCallback(start, end);
    RayCallback.m_collisionFilterGroup = group;
    RayCallback.m_collisionFilterMask  = mask;
    Engine::priv::PhysicsModule::PHYSICS_MANAGER->m_Pipeline.m_World->rayTest(start, end, RayCallback);
#if defined(_DEBUG) || defined(ENGINE_FORCE_PHYSICS_DEBUG_DRAW)
    Engine::priv::PhysicsModule::PHYSICS_MANAGER->m_Pipeline.m_World->getDebugDrawer()->drawLine(start, end, btVector4(1.0f, 0.5f, 0.0f, 1.0f));
#endif
    Engine::RayCastResult result(RayCallback);
    return result;
}
std::vector<Engine::RayCastResult> internal_ray_cast(const btVector3& start, const btVector3& end, MaskType group, MaskType mask) {
    btCollisionWorld::AllHitsRayResultCallback AllHitsRayCallback(start, end);
    AllHitsRayCallback.m_collisionFilterGroup = (int)group;
    AllHitsRayCallback.m_collisionFilterMask  = (int)mask;

    Engine::priv::PhysicsModule::PHYSICS_MANAGER->m_Pipeline.m_World->rayTest(start, end, AllHitsRayCallback);
#if defined(_DEBUG) || defined(ENGINE_FORCE_PHYSICS_DEBUG_DRAW)
    Engine::priv::PhysicsModule::PHYSICS_MANAGER->m_Pipeline.m_World->getDebugDrawer()->drawLine(start, end, btVector4(1, 0.5, 0, 1));
#endif

    std::vector<Engine::RayCastResult> result;
    if (AllHitsRayCallback.hasHit()) {
        auto& pts = AllHitsRayCallback.m_hitPointWorld;
        result.reserve(pts.size());
        for (int i = 0; i < pts.size(); ++i) { 
            result.emplace_back(AllHitsRayCallback, i);
        }
    }
    return result;
}
std::vector<Engine::RayCastResult> Engine::Physics::rayCast(const btVector3& start, const btVector3& end, ComponentBody* ignoredComponentBody, MaskType group, MaskType mask){
    if (ignoredComponentBody) {
        Engine::Physics::removeRigidBody(*ignoredComponentBody);
    }
    std::vector<Engine::RayCastResult> result = internal_ray_cast(start, end, group, mask);
    if (ignoredComponentBody) {
        Engine::Physics::addRigidBody(*ignoredComponentBody);
    }
    return result;
}
std::vector<Engine::RayCastResult> Engine::Physics::rayCast(const btVector3& start, const btVector3& end, std::vector<ComponentBody*>& ignoredComponentBodies, MaskType group, MaskType mask){
    for (auto& body : ignoredComponentBodies) {
        Engine::Physics::removeRigidBody(*body);
    }
    std::vector<RayCastResult> result = internal_ray_cast(start, end, group, mask);
    for (auto& body : ignoredComponentBodies) {
        Engine::Physics::addRigidBody(*body);
    }
    return result;
 }
std::vector<Engine::RayCastResult> Engine::Physics::rayCast(const glm::vec3& start, const glm::vec3& end, Entity* ignoredEntity, MaskType group, MaskType mask){
    btVector3 start_ = Engine::Math::btVectorFromGLM(start);
    btVector3 end_   = Engine::Math::btVectorFromGLM(end);
    if (ignoredEntity) {
        auto body = ignoredEntity->getComponent<ComponentBody>();
        if (body && body->hasPhysics()) {
            return Engine::Physics::rayCast(start_, end_, body, group, mask);
        }
    }
    return Engine::Physics::rayCast(start_, end_, nullptr, group, mask);
 }
std::vector<Engine::RayCastResult> Engine::Physics::rayCast(const glm::vec3& start, const glm::vec3& end , std::vector<Entity>& ignoredEntities, MaskType group, MaskType mask){
    btVector3 start_ = Engine::Math::btVectorFromGLM(start);
    btVector3 end_   = Engine::Math::btVectorFromGLM(end);
    std::vector<ComponentBody*> objs;
    objs.reserve(ignoredEntities.size());
    for(auto ignoredEntity : ignoredEntities){
        auto body = ignoredEntity.getComponent<ComponentBody>();
        if(body && body->hasPhysics()) {
            objs.emplace_back(body);
        }
    }
    return Engine::Physics::rayCast(start_, end_, objs, group, mask);
}

Engine::RayCastResult Engine::Physics::rayCastNearest(const btVector3& start, const btVector3& end, ComponentBody* ignoredBodyComponent, MaskType group, MaskType mask) {
    if (ignoredBodyComponent) {
        Engine::Physics::removeRigidBody(*ignoredBodyComponent);
    }
    Engine::RayCastResult result = internal_ray_cast_nearest(start, end, group, mask);
    if (ignoredBodyComponent) {
        Engine::Physics::addRigidBody(*ignoredBodyComponent);
    }
    return result;
}
Engine::RayCastResult Engine::Physics::rayCastNearest(const btVector3& start, const btVector3& end, std::vector<ComponentBody*>& ignoredComponentBodies, MaskType group, MaskType mask) {
    for (auto& object : ignoredComponentBodies) {
        Physics::removeRigidBody(*object);
    }
    Engine::RayCastResult result = internal_ray_cast_nearest(start, end, group, mask);
    for (auto& object : ignoredComponentBodies) {
        Engine::Physics::addRigidBody(*object);
    }
    return result;
}
Engine::RayCastResult Engine::Physics::rayCastNearest(const glm::vec3& start, const glm::vec3& end, Entity* ignoredEntity, MaskType group, MaskType mask) {
    btVector3 start_ = Engine::Math::btVectorFromGLM(start);
    btVector3 end_   = Engine::Math::btVectorFromGLM(end);
    if (ignoredEntity) {
        auto body = ignoredEntity->getComponent<ComponentBody>();
        if (body && body->hasPhysics()) {
            return Engine::Physics::rayCastNearest(start_, end_, body, group, mask);
        }
    }
    return Engine::Physics::rayCastNearest(start_, end_, nullptr, group, mask);
}
Engine::RayCastResult Engine::Physics::rayCastNearest(const glm::vec3& start, const glm::vec3& end, std::vector<Entity>& ignoredEntities, MaskType group, MaskType mask) {
    btVector3 start_ = Engine::Math::btVectorFromGLM(start);
    btVector3 end_   = Engine::Math::btVectorFromGLM(end);
    std::vector<ComponentBody*> objs;
    objs.reserve(ignoredEntities.size());
    for (auto ignoredEntity : ignoredEntities) {
        auto body = ignoredEntity.getComponent<ComponentBody>();
        if (body && body->hasPhysics()) {
            objs.emplace_back(body);
        }
    }
    return Engine::Physics::rayCastNearest(start_, end_, objs, group, mask);
}