#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/physics/PhysicsPipeline.h>
#include <core/engine/system/Engine.h>
#include <core/engine/threading/ThreadingModule.h>
#include <core/engine/utils/Utils.h>

#include <BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolverMT.h>
#include <BulletCollision/BroadphaseCollision/btDbvtBroadphase.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorldMT.h>
#include <BulletCollision/CollisionDispatch/btCollisionDispatcherMt.h>
#include <BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h>

#include <LinearMath/btIDebugDraw.h>

#pragma region PhysicsTaskScheduler

Engine::priv::PhysicsTaskScheduler::PhysicsTaskScheduler(const char* name)
    : btITaskScheduler{ name }
    , m_name{ name }
{
    m_sumRes = btScalar(0.0);
}
Engine::priv::PhysicsTaskScheduler::~PhysicsTaskScheduler() {

}
int Engine::priv::PhysicsTaskScheduler::getMaxNumThreads() const {
    return Engine::hardware_concurrency();
}
int Engine::priv::PhysicsTaskScheduler::getNumThreads() const {
    return Engine::hardware_concurrency();
}
void Engine::priv::PhysicsTaskScheduler::setNumThreads(int numThreads) {

}
void Engine::priv::PhysicsTaskScheduler::parallelFor(int iBegin, int iEnd, int grainSize, const btIParallelForBody& body) {
    auto parallel = [this, &body, iEnd, iBegin]() {
        auto pairs = Engine::splitVectorPairs(iEnd - iBegin, 0);
        for (size_t i = 0; i < pairs.size(); ++i) {
            auto lambda = [&body, i, &pairs]() {
                body.forLoop((int)pairs[i].first, (int)pairs[i].second + 1);
            };
            Engine::priv::threading::addJob(lambda, 0);
        }
        Engine::priv::threading::waitForAll(0);
    };
    if (m_DoConcurrency) {
        parallel();
    }else{
        body.forLoop(iBegin, iEnd);
    }
}
btScalar Engine::priv::PhysicsTaskScheduler::parallelSum(int iBegin, int iEnd, int grainSize, const btIParallelSumBody& body) {
    auto parallel = [this, &body, iEnd, iBegin]() -> btScalar {
        auto pairs = Engine::splitVectorPairs(iEnd - iBegin, 0);
        m_sumRes = btScalar(0.0);
        for (size_t i = 0; i < pairs.size(); ++i) {
            auto lambda = [&body, this, i, &pairs]() {
                #ifdef ENVIRONMENT64
                    m_sumRes += body.sumLoop((int)pairs[i].first, (int)pairs[i].second + 1);
                #else
                    auto data = m_sumRes.load();
                    data += body.sumLoop((int)pairs[i].first, (int)pairs[i].second + 1);
                    m_sumRes = data;
                #endif
            };
            Engine::priv::threading::addJob(lambda, 0);
        }
        Engine::priv::threading::waitForAll(0);
        return m_sumRes;
    }; 
    btScalar res = 0.0;
    if (m_DoConcurrency) {
        res = parallel();
    }else{
        res = body.sumLoop(iBegin, iEnd);
    }
    return res;
}

#pragma endregion

#pragma region PhysicsPipeline

Engine::priv::PhysicsPipeline::PhysicsPipeline() {
    auto hardware_concurrency = Engine::hardware_concurrency();

    m_Broadphase              = std::make_unique<btDbvtBroadphase>();
    m_CollisionConfiguration  = std::make_unique<btDefaultCollisionConfiguration>();
    m_Dispatcher              = std::make_unique<btCollisionDispatcher>(m_CollisionConfiguration.get());
    if (hardware_concurrency <= 1) {
        m_Solver              = std::make_unique<btSequentialImpulseConstraintSolver>();
        m_World               = std::make_unique<btDiscreteDynamicsWorld>(m_Dispatcher.get(), m_Broadphase.get(), m_Solver.get(), m_CollisionConfiguration.get());
    }else{
        m_TaskScheduler       = std::make_unique<PhysicsTaskScheduler>("PhysicsTaskScheduler");
        btSetTaskScheduler(m_TaskScheduler.get());
        m_SolverPool          = std::make_unique<btConstraintSolverPoolMt>(hardware_concurrency);
        m_SolverMT            = std::make_unique<btSequentialImpulseConstraintSolverMt>();
        m_World               = std::make_unique<btDiscreteDynamicsWorldMt>(m_Dispatcher.get(), m_Broadphase.get(), m_SolverPool.get(), m_SolverMT.get(), m_CollisionConfiguration.get());
    }
    m_DebugDrawer.setDebugMode(btIDebugDraw::DBG_MAX_DEBUG_DRAW_MODE);
    m_World->setDebugDrawer(&m_DebugDrawer);
    m_World->setGravity(btVector3(0.0, 0.0, 0.0));

    m_World->setForceUpdateAllAabbs(false); //TODO: optional optimization thing. remove if it causes issues

    btGImpactCollisionAlgorithm::registerAlgorithm(m_Dispatcher.get());
    setPreTickCallback(m_PreTickCallback);
    setPostTickCallback(m_PostTickCallback);
}
Engine::priv::PhysicsPipeline::~PhysicsPipeline() {
    cleanup();
}
void Engine::priv::PhysicsPipeline::cleanup() {
    int collisionObjCount = m_World->getNumCollisionObjects();
    for (int i = 0; i < collisionObjCount; ++i) {
        btCollisionObject* obj = m_World->getCollisionObjectArray()[i];
        if (obj) {
            btRigidBody* body = btRigidBody::upcast(obj);
            if (body) {
                auto motionState = body->getMotionState();
                SAFE_DELETE(motionState);
            }
            m_World->removeCollisionObject(obj);
            SAFE_DELETE(obj);
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
void Engine::priv::PhysicsPipeline::update(const float dt) {
    /*
    //Test performance here
    if (Engine::isKeyDownOnce(KeyboardKey::M)) {
        m_TaskScheduler->m_DoConcurrency = true;
    }
    if (Engine::isKeyDownOnce(KeyboardKey::N)) {
        m_TaskScheduler->m_DoConcurrency = false;
    }
    */
}

#pragma endregion