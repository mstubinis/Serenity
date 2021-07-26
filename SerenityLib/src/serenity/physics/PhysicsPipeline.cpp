#include <serenity/physics/PhysicsPipeline.h>
#include <serenity/system/Engine.h>
#include <serenity/threading/ThreadingModule.h>
#include <serenity/utils/Utils.h>

#include <BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h>
#include <BulletCollision/BroadphaseCollision/btDbvtBroadphase.h>
#include <BulletCollision/CollisionDispatch/btCollisionDispatcherMt.h>
#include <BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolverMT.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorldMT.h>

#include <LinearMath/btIDebugDraw.h>

#pragma region PhysicsPipeline

Engine::priv::PhysicsPipeline::PhysicsPipeline() {
    auto hardware_concurrency = Engine::hardware_concurrency();
    m_Broadphase              = std::make_unique<btDbvtBroadphase>();
    m_CollisionConfiguration  = std::make_unique<btDefaultCollisionConfiguration>();
    m_Dispatcher              = std::make_unique<btCollisionDispatcher>(m_CollisionConfiguration.get());
    if (hardware_concurrency <= 1) {
        m_Solver              = std::make_unique<btSequentialImpulseConstraintSolver>();
        m_World               = std::make_unique<btDiscreteDynamicsWorld>(m_Dispatcher.get(), m_Broadphase.get(), m_Solver.get(), m_CollisionConfiguration.get());
    } else {
        m_TaskScheduler       = std::make_unique<PhysicsTaskScheduler>("PhysicsTaskScheduler");
        btSetTaskScheduler(m_TaskScheduler.get());
        m_SolverPool          = std::make_unique<btConstraintSolverPoolMt>(hardware_concurrency);
        m_SolverMT            = std::make_unique<btSequentialImpulseConstraintSolverMt>();
        m_World               = std::make_unique<btDiscreteDynamicsWorldMt>(m_Dispatcher.get(), m_Broadphase.get(), m_SolverPool.get(), m_SolverMT.get(), m_CollisionConfiguration.get());
    }
    m_DebugDrawer.setDebugMode(btIDebugDraw::DBG_MAX_DEBUG_DRAW_MODE);
    m_World->setDebugDrawer(&m_DebugDrawer);
    m_World->setGravity(btVector3{ 0.0, 0.0, 0.0 });
    m_World->setForceUpdateAllAabbs(false); //TODO: optional optimization thing. remove if it causes issues

    btGImpactCollisionAlgorithm::registerAlgorithm(m_Dispatcher.get());
    setPreTickCallback(m_PreTickCallback);
    setPostTickCallback(m_PostTickCallback);
}
Engine::priv::PhysicsPipeline::~PhysicsPipeline() {
    cleanup();
}
void Engine::priv::PhysicsPipeline::init() {
    m_DebugDrawer.init();
}
void Engine::priv::PhysicsPipeline::drawLine(const glm::vec3& start, const glm::vec3& end, float r, float g, float b) {
    m_DebugDrawer.drawLine(btVector3{ start.x, start.y, start.z }, btVector3{ end.x, end.y, end.z }, btVector3{ r, g, b });
}
void Engine::priv::PhysicsPipeline::cleanup() {
    int collisionObjCount = m_World->getNumCollisionObjects();
    for (int i = 0; i < collisionObjCount; ++i) {
        btCollisionObject* BTCollisionObj = m_World->getCollisionObjectArray()[i];
        if (BTCollisionObj) {
            btRigidBody* BTRigidBody = btRigidBody::upcast(BTCollisionObj);
            if (BTRigidBody) {
                auto motionState = BTRigidBody->getMotionState();
                SAFE_DELETE(motionState);
            }
            m_World->removeCollisionObject(BTCollisionObj);
            SAFE_DELETE(BTCollisionObj);
        }
    }
}
void Engine::priv::PhysicsPipeline::setPreTickCallback(btInternalTickCallback preTicCallback) {
    m_PreTickCallback = preTicCallback;
    m_World->setInternalTickCallback(m_PreTickCallback, (void*)m_World.get(), true);
}
void Engine::priv::PhysicsPipeline::setPostTickCallback(btInternalTickCallback postTickCallback) {
    m_PostTickCallback = postTickCallback;
    m_World->setInternalTickCallback(m_PostTickCallback, (void*)m_World.get(), false);
}
#pragma endregion