
#include <serenity/physics/PhysicsModule.h>
#include <serenity/physics/DebugDrawer.h>
#include <serenity/system/Engine.h>

#include <serenity/renderer/Renderer.h>
#include <serenity/scene/Camera.h>
#include <serenity/ecs/components/ComponentModel.h>
#include <serenity/ecs/components/ComponentTransform.h>

#include <BulletCollision/CollisionShapes/btCollisionShape.h>
#include <BulletCollision/CollisionShapes/btTriangleShape.h>
#include <BulletDynamics/Dynamics/btRigidBody.h>
#include <BulletCollision/NarrowPhaseCollision/btRaycastCallback.h>
#include <BulletCollision/CollisionDispatch/btInternalEdgeUtility.h>

#include <serenity/physics/btRigidBodyEnhanced.h>

Engine::view_ptr<Engine::priv::PhysicsModule> Engine::priv::PhysicsModule::PHYSICS_MANAGER = nullptr;

void ProcessManifoldContact(btManifoldPoint& cp, btCollisionObject* collObjA, btCollisionObject* collObjB, btCollisionShape* childShapeA, btCollisionShape* childShapeB, const btCollisionShape* shapeA, const btCollisionShape* shapeB) {
    auto aRigidBody   = static_cast<ComponentRigidBody*>(collObjA->getUserPointer());
    auto bRigidBody   = static_cast<ComponentRigidBody*>(collObjB->getUserPointer());
    if (aRigidBody && bRigidBody) {
        glm::vec3 ptA            = Engine::Math::toGLM(cp.getPositionWorldOnA());
        glm::vec3 ptB            = Engine::Math::toGLM(cp.getPositionWorldOnB());
        glm::vec3 normalOnB      = Engine::Math::toGLM(cp.m_normalWorldOnB);
        glm::vec3 localPositionA = Engine::Math::toGLM(cp.m_localPointA);
        glm::vec3 localPositionB = Engine::Math::toGLM(cp.m_localPointB);
        glm::vec3 normalA        = -glm::normalize(ptB - ptA);
        glm::vec3 normalB        = -glm::normalize(ptA - ptB);
        auto isTriangleType = [](int t) {
            return (t == TRIANGLE_SHAPE_PROXYTYPE || 
                    t == SCALED_TRIANGLE_MESH_SHAPE_PROXYTYPE || 
                    t == TRIANGLE_MESH_SHAPE_PROXYTYPE || 
                    t == MULTIMATERIAL_TRIANGLE_MESH_PROXYTYPE || 
                    t == GIMPACT_SHAPE_PROXYTYPE
            );
        };
        auto extractTriangleNormal = [&isTriangleType](const btCollisionShape* shape, glm::vec3& outNormal) {
            if (shape && isTriangleType(shape->getShapeType())) {
                const auto tri         = static_cast<const btTriangleShape*>(shape);
                const btVector3 normal = (tri->m_vertices1[1] - tri->m_vertices1[0]).cross(tri->m_vertices1[2] - tri->m_vertices1[0]).normalize();
                outNormal = Engine::Math::toGLM(normal);
            }
        };

        extractTriangleNormal(shapeA,      normalA);
        extractTriangleNormal(shapeB,      normalB);
        extractTriangleNormal(childShapeA, normalA);
        extractTriangleNormal(childShapeB, normalB);

        RigidCollisionCallbackEventData dataA{ *aRigidBody, *bRigidBody, ptA, ptB, normalOnB, localPositionA, localPositionB, normalA, normalB };
        dataA.ownerCollisionObj       = collObjA;
        dataA.otherCollisionObj       = collObjB;
        dataA.ownerCollisionShape     = childShapeA;
        dataA.otherCollisionShape     = childShapeB;
        dataA.ownerCollisionShapeRoot = const_cast<btCollisionShape*>(shapeA);
        dataA.ownerCollisionShapeRoot = const_cast<btCollisionShape*>(shapeB);
        RigidCollisionCallbackEventData dataB{ *bRigidBody, *aRigidBody, ptB, ptA, normalOnB, localPositionB, localPositionA, normalB, normalA };
        dataB.ownerCollisionObj       = collObjB;
        dataB.otherCollisionObj       = collObjA;
        dataB.ownerCollisionShape     = childShapeB;
        dataB.otherCollisionShape     = childShapeA;
        dataB.ownerCollisionShapeRoot = const_cast<btCollisionShape*>(shapeB);
        dataB.otherCollisionShapeRoot = const_cast<btCollisionShape*>(shapeA);

        if (childShapeA && childShapeA->getUserPointer()) {
            auto modelInstanceChildA      = static_cast<ModelInstance*>(childShapeA->getUserPointer());
            dataA.ownerModelInstanceIndex = modelInstanceChildA->getIndex();
            dataB.otherModelInstanceIndex = modelInstanceChildA->getIndex();
        }
        if (childShapeB && childShapeB->getUserPointer()) {
            auto modelInstanceChildB      = static_cast<ModelInstance*>(childShapeB->getUserPointer());
            dataA.otherModelInstanceIndex = modelInstanceChildB->getIndex();
            dataB.ownerModelInstanceIndex = modelInstanceChildB->getIndex();
        }
        aRigidBody->collisionResponse(dataA);
        bRigidBody->collisionResponse(dataB);
        cp.setDistance(static_cast<btScalar>(9999999999999.0)); //hacky way of saying "dont process this again"
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
        btCollisionObject* colObject0 = const_cast<btCollisionObject*>(colObj0Wrap->getCollisionObject());
        btCollisionObject* colObject1 = const_cast<btCollisionObject*>(colObj1Wrap->getCollisionObject());
        ProcessManifoldContact(cp, colObject0, colObject1, childShapeA, childShapeB, colObj0Wrap->m_shape, colObj1Wrap->m_shape);
    }
    return true;
}
Engine::priv::PhysicsModule::PhysicsModule(){
    PHYSICS_MANAGER       = this;
    gContactAddedCallback = CustomMaterialContactAddedCallback;
}
void Engine::priv::PhysicsModule::init(){
    m_Pipeline.m_DebugDrawer.init();
}

bool add_rigid_body(Engine::priv::PhysicsPipeline& pipeline, btRigidBody* inRigidBody, MaskType group, MaskType mask) noexcept {
    ASSERT(inRigidBody, __FUNCTION__ << "(): rigidBody was nullptr!");
    if (!inRigidBody || (inRigidBody && inRigidBody->isInWorld())) {
        return false;
    }
    pipeline.m_World->addRigidBody(inRigidBody, group, mask);
    return true;
}
bool add_rigid_body(Engine::priv::PhysicsPipeline& pipeline, btRigidBody* inRigidBody) noexcept {
    ASSERT(inRigidBody, __FUNCTION__ << "(): rigidBody was nullptr!");
    if (!inRigidBody || (inRigidBody && inRigidBody->isInWorld())) {
        return false;
    }
    pipeline.m_World->addRigidBody(inRigidBody);
    return true;
}
void Engine::priv::PhysicsModule::update(Scene& scene, const float dt, int maxSubSteps, float fixedTimeStep){
    Engine::priv::Core::m_Engine->m_DebugManager.stop_clock_physics();
    if (!m_Paused) {
        m_Pipeline.m_World->stepSimulation((btScalar)dt, maxSubSteps, (btScalar)fixedTimeStep);
        for (int i = 0; i < m_Pipeline.m_Dispatcher->getNumManifolds(); ++i) {
            auto& contactMnifld = *m_Pipeline.m_Dispatcher->getManifoldByIndexInternal(i);
            for (int j = 0; j < contactMnifld.getNumContacts(); ++j) {
                auto& cp = contactMnifld.getContactPoint(j);
                if (cp.getDistance() < 0.0f) {
                    btCollisionObject* colObject0 = const_cast<btCollisionObject*>(contactMnifld.getBody0());
                    btCollisionObject* colObject1 = const_cast<btCollisionObject*>(contactMnifld.getBody1());
                    ProcessManifoldContact(cp, colObject0, colObject1, nullptr, nullptr, nullptr, nullptr);
                }
            }
        }
    }
    Engine::priv::Core::m_Engine->m_DebugManager.calculate_physics();
}
void Engine::priv::PhysicsModule::render(Scene& scene, const Camera& camera){
    m_Pipeline.m_World->debugDrawWorld();
    Engine::Renderer::sendUniformMatrix4("Model", glm::mat4{ 1.0f });
    Engine::Renderer::sendUniformMatrix4("VP", camera.getViewProjection());
    m_Pipeline.m_DebugDrawer.drawAccumulatedLines();
    m_Pipeline.m_DebugDrawer.postRender();
}
void Engine::Physics::cleanProxyFromPairs(btRigidBody* BTRigidBody) {
    auto& world = *Engine::priv::PhysicsModule::PHYSICS_MANAGER->m_Pipeline.m_World;
    world.getBroadphase()->getOverlappingPairCache()->cleanProxyFromPairs(BTRigidBody->getBroadphaseHandle(), world.getDispatcher());
}
void Engine::Physics::updateDiscreteCollisionDetection() noexcept {
    Engine::priv::PhysicsModule::PHYSICS_MANAGER->m_Pipeline.m_World->performDiscreteCollisionDetection();
}
void Engine::Physics::updateAABBs() noexcept {
    Engine::priv::PhysicsModule::PHYSICS_MANAGER->m_Pipeline.m_World->updateAabbs();
}
void Engine::Physics::calculateOverlappingPairs() noexcept {
    Engine::priv::PhysicsModule::PHYSICS_MANAGER->m_Pipeline.m_Broadphase->calculateOverlappingPairs(Engine::priv::PhysicsModule::PHYSICS_MANAGER->m_Pipeline.m_Dispatcher.get());
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
bool Engine::Physics::addRigidBody(btRigidBody* rigidBody, MaskType group, MaskType mask){
    return add_rigid_body(Engine::priv::PhysicsModule::PHYSICS_MANAGER->m_Pipeline, rigidBody, group, mask);
}
bool Engine::Physics::addRigidBody(btRigidBody* rigidBody){
    return add_rigid_body(Engine::priv::PhysicsModule::PHYSICS_MANAGER->m_Pipeline, rigidBody);
}
bool Engine::Physics::removeRigidBody(btRigidBody* rigidBody){
    if (!rigidBody || (rigidBody && !rigidBody->isInWorld())) {
        return false;
    }
    Engine::priv::PhysicsModule::PHYSICS_MANAGER->m_Pipeline.m_World->removeRigidBody(rigidBody);
    return true;
}
bool Engine::Physics::removeCollisionObject(btCollisionObject* object) {
    Engine::priv::PhysicsModule::PHYSICS_MANAGER->m_Pipeline.m_World->removeCollisionObject(object);
    return true;
}
void Engine::Physics::updateRigidBody(btRigidBody* rigidBody){
    Engine::priv::PhysicsModule::PHYSICS_MANAGER->m_Pipeline.m_World->updateSingleAabb(rigidBody);
}
bool Engine::Physics::addRigidBodyThreadSafe(btRigidBody* body, MaskType group, MaskType mask) {
    std::lock_guard lock{ Engine::priv::PhysicsModule::PHYSICS_MANAGER->m_Mutex };
    return Engine::Physics::addRigidBody(body, group, mask);
}
bool Engine::Physics::addRigidBodyThreadSafe(btRigidBody* body) {
    std::lock_guard lock{ Engine::priv::PhysicsModule::PHYSICS_MANAGER->m_Mutex };
    return Engine::Physics::addRigidBody(body);
}
bool Engine::Physics::removeRigidBodyThreadSafe(btRigidBody* body) {
    std::lock_guard lock{ Engine::priv::PhysicsModule::PHYSICS_MANAGER->m_Mutex };
    return Engine::Physics::removeRigidBody(body);
}
void Engine::Physics::updateRigidBodyThreadSafe(btRigidBody* body) {
    std::lock_guard lock{ Engine::priv::PhysicsModule::PHYSICS_MANAGER->m_Mutex };
    Engine::Physics::updateRigidBody(body);
}
bool Engine::Physics::removeCollisionObjectThreadSafe(btCollisionObject* object) {
    std::lock_guard lock{ Engine::priv::PhysicsModule::PHYSICS_MANAGER->m_Mutex };
    return Engine::Physics::removeCollisionObject(object);
}
void debugDrawRaycastLine(const btVector3& start, const btVector3& end, float r, float g, float b, float a) {
    #if defined(_DEBUG) || defined(ENGINE_FORCE_PHYSICS_DEBUG_DRAW)
        Engine::priv::PhysicsModule::PHYSICS_MANAGER->m_Pipeline.m_World->getDebugDrawer()->drawLine(start, end, btVector4(r, g, b, a));
    #endif
}
Engine::RayCastResult internal_ray_cast_nearest(const btVector3& start, const btVector3& end, MaskType group, MaskType mask) {
    btCollisionWorld::ClosestRayResultCallback RayCallback{ start, end };
    RayCallback.m_collisionFilterGroup = group;
    RayCallback.m_collisionFilterMask  = mask;
    Engine::priv::PhysicsModule::PHYSICS_MANAGER->m_Pipeline.m_World->rayTest(start, end, RayCallback);
    debugDrawRaycastLine(start, end, 1.0f, 0.5f, 0.0f, 1.0f);
    Engine::RayCastResult result{ RayCallback };
    return result;
}
std::vector<Engine::RayCastResult> internal_ray_cast(const btVector3& start, const btVector3& end, MaskType group, MaskType mask) {
    btCollisionWorld::AllHitsRayResultCallback AllHitsRayCallback{ start, end };
    AllHitsRayCallback.m_collisionFilterGroup = group;
    AllHitsRayCallback.m_collisionFilterMask  = mask;
    Engine::priv::PhysicsModule::PHYSICS_MANAGER->m_Pipeline.m_World->rayTest(start, end, AllHitsRayCallback);
    debugDrawRaycastLine(start, end, 1.0f, 0.5f, 0.0f, 1.0f);
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
std::vector<Engine::RayCastResult> Engine::Physics::rayCast(const btVector3& start, const btVector3& end, ComponentRigidBody* ignoredRigidBody, MaskType group, MaskType mask){
    if (ignoredRigidBody) {
        Engine::Physics::removeRigidBody(ignoredRigidBody->getBtBody());
    }
    std::vector<Engine::RayCastResult> result = internal_ray_cast(start, end, group, mask);
    if (ignoredRigidBody) {
        Engine::Physics::addRigidBody(ignoredRigidBody->getBtBody(), ignoredRigidBody->getCollisionGroup(), ignoredRigidBody->getCollisionMask());
    }
    return result;
}
std::vector<Engine::RayCastResult> Engine::Physics::rayCast(const btVector3& start, const btVector3& end, std::vector<ComponentRigidBody*>& ignoredRigidBodies, MaskType group, MaskType mask){
    for (auto& ignoredRigidBody : ignoredRigidBodies) {
        Engine::Physics::removeRigidBody(ignoredRigidBody->getBtBody());
    }
    std::vector<RayCastResult> result = internal_ray_cast(start, end, group, mask);
    for (auto& ignoredRigidBody : ignoredRigidBodies) {
        Engine::Physics::addRigidBody(ignoredRigidBody->getBtBody(), ignoredRigidBody->getCollisionGroup(), ignoredRigidBody->getCollisionMask());
    }
    return result;
 }
std::vector<Engine::RayCastResult> Engine::Physics::rayCast(const glm::vec3& start, const glm::vec3& end, Entity ignoredEntity, MaskType group, MaskType mask){
    btVector3 start_ = Engine::Math::toBT(start);
    btVector3 end_   = Engine::Math::toBT(end);
    if (ignoredEntity) {
        auto rigidBody = ignoredEntity.getComponent<ComponentRigidBody>();
        if (rigidBody) {
            return Engine::Physics::rayCast(start_, end_, rigidBody, group, mask);
        }
    }
    return Engine::Physics::rayCast(start_, end_, nullptr, group, mask);
 }
std::vector<Engine::RayCastResult> Engine::Physics::rayCast(const glm::vec3& start, const glm::vec3& end , std::vector<Entity>& ignoredEntities, MaskType group, MaskType mask){
    btVector3 start_ = Engine::Math::toBT(start);
    btVector3 end_   = Engine::Math::toBT(end);
    auto objs = Engine::create_and_reserve<std::vector<ComponentRigidBody*>>((uint32_t)ignoredEntities.size());
    for(auto ignoredEntity : ignoredEntities){
        auto rigid = ignoredEntity.getComponent<ComponentRigidBody>();
        if(rigid) {
            objs.emplace_back(rigid);
        }
    }
    return Engine::Physics::rayCast(start_, end_, objs, group, mask);
}
Engine::RayCastResult Engine::Physics::rayCastNearest(const btVector3& start, const btVector3& end, ComponentRigidBody* ignoredRigidBody, MaskType group, MaskType mask) {
    if (ignoredRigidBody) {
        Engine::Physics::removeRigidBody(ignoredRigidBody->getBtBody());
    }
    Engine::RayCastResult result = internal_ray_cast_nearest(start, end, group, mask);
    if (ignoredRigidBody) {
        Engine::Physics::addRigidBody(ignoredRigidBody->getBtBody(), ignoredRigidBody->getCollisionGroup(), ignoredRigidBody->getCollisionMask());
    }
    return result;
}
Engine::RayCastResult Engine::Physics::rayCastNearest(const btVector3& start, const btVector3& end, std::vector<ComponentRigidBody*>& ignoredRigidBodies, MaskType group, MaskType mask) {
    for (auto& ignoredRigidBody : ignoredRigidBodies) {
        Physics::removeRigidBody(ignoredRigidBody->getBtBody());
    }
    Engine::RayCastResult result = internal_ray_cast_nearest(start, end, group, mask);
    for (auto& ignoredRigidBody : ignoredRigidBodies) {
        Engine::Physics::addRigidBody(ignoredRigidBody->getBtBody(), ignoredRigidBody->getCollisionGroup(), ignoredRigidBody->getCollisionMask());
    }
    return result;
}
Engine::RayCastResult Engine::Physics::rayCastNearest(const glm::vec3& start, const glm::vec3& end, Entity ignoredEntity, MaskType group, MaskType mask) {
    btVector3 start_ = Engine::Math::toBT(start);
    btVector3 end_   = Engine::Math::toBT(end);
    if (ignoredEntity) {
        auto rigid = ignoredEntity.getComponent<ComponentRigidBody>();
        if (rigid) {
            return Engine::Physics::rayCastNearest(start_, end_, rigid, group, mask);
        }
    }
    return Engine::Physics::rayCastNearest(start_, end_, nullptr, group, mask);
}
Engine::RayCastResult Engine::Physics::rayCastNearest(const glm::vec3& start, const glm::vec3& end, std::vector<Entity>& ignoredEntities, MaskType group, MaskType mask) {
    btVector3 start_ = Engine::Math::toBT(start);
    btVector3 end_   = Engine::Math::toBT(end);
    auto objs = Engine::create_and_reserve<std::vector<ComponentRigidBody*>>((uint32_t)ignoredEntities.size());
    for (auto ignoredEntity : ignoredEntities) {
        auto rigid = ignoredEntity.getComponent<ComponentRigidBody>();
        if (rigid) {
            objs.emplace_back(rigid);
        }
    }
    return Engine::Physics::rayCastNearest(start_, end_, objs, group, mask);
}