#include <core/engine/physics/PhysicsPipeline.h>
#include <core/engine/system/Engine.h>
#include <core/engine/threading/Engine_ThreadManager.h>
#include <core/engine/utils/Utils.h>

#include <BulletCollision/Gimpact/btGImpactCollisionAlgorithm.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolver.h>
#include <BulletDynamics/ConstraintSolver/btSequentialImpulseConstraintSolverMT.h>
#include <BulletCollision/BroadphaseCollision/btDbvtBroadphase.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorld.h>
#include <BulletDynamics/Dynamics/btDiscreteDynamicsWorldMT.h>
#include <BulletCollision/CollisionDispatch/btCollisionDispatcherMt.h>
#include <BulletCollision/CollisionDispatch/btDefaultCollisionConfiguration.h>

#include <LinearMath/btIDebugDraw.h>

#include <algorithm>
#include <iostream>

using namespace std;
using namespace Engine;

#pragma region PhysicsTaskScheduler

priv::PhysicsTaskScheduler::PhysicsTaskScheduler(const char* name) : btITaskScheduler(name){
    m_name = name;
    m_sumRes.store(btScalar(0.0), std::memory_order_relaxed);
}
priv::PhysicsTaskScheduler::~PhysicsTaskScheduler() {

}
const char* priv::PhysicsTaskScheduler::getName() const {
    return m_name; 
}
int priv::PhysicsTaskScheduler::getMaxNumThreads() const {
    return Engine::priv::threading::hardware_concurrency();
}
int priv::PhysicsTaskScheduler::getNumThreads() const {
    return Engine::priv::threading::hardware_concurrency();
}
void priv::PhysicsTaskScheduler::setNumThreads(int numThreads) {

}
void priv::PhysicsTaskScheduler::parallelFor(int iBegin, int iEnd, int grainSize, const btIParallelForBody& body) {
    auto parallel = [this, &body, iEnd, iBegin]() {
        auto split = Engine::priv::threading::splitVectorPairs(iEnd - iBegin, 0);
        auto lamda = [&body](const std::pair<size_t, size_t>& p, const unsigned int inK) {
            body.forLoop((int)p.first, (int)p.second + 1);
        };
        for (size_t i = 0; i < split.size(); ++i) {
            Engine::priv::Core::m_Engine->m_ThreadManager.add_job_engine_controlled(lamda, std::ref(split[i]), i);
        }
        Engine::priv::Core::m_Engine->m_ThreadManager.wait_for_all_engine_controlled();
    };
    
    if (m_DoConcurrency) {
        parallel();
    }else{
        body.forLoop(iBegin, iEnd);
    }
}
btScalar priv::PhysicsTaskScheduler::parallelSum(int iBegin, int iEnd, int grainSize, const btIParallelSumBody& body) {
    auto parallel = [this, &body, iEnd, iBegin]() {
        auto split = Engine::priv::threading::splitVectorPairs(iEnd - iBegin, 0);
        m_sumRes.store(btScalar(0.0), std::memory_order_relaxed);
        auto lamda = [&body, this](const std::pair<size_t, size_t>& p, const unsigned int inK) {
            m_sumRes += body.sumLoop((int)p.first, (int)p.second + 1);
        };
        for (size_t i = 0; i < split.size(); ++i) {
            Engine::priv::Core::m_Engine->m_ThreadManager.add_job_engine_controlled(lamda, std::ref(split[i]), i);
        }
        Engine::priv::Core::m_Engine->m_ThreadManager.wait_for_all_engine_controlled();
        return m_sumRes.load(std::memory_order_relaxed);
    };
    
    btScalar res = 0.0;
    if (m_DoConcurrency) {
        res = parallel();
    }else{
        res = body.sumLoop(iBegin, iEnd);
    }
    return res;

}
void priv::PhysicsTaskScheduler::sleepWorkerThreadsHint() {
    // hint the task scheduler that we may not be using these threads for a little while
} 
// internal use only
void priv::PhysicsTaskScheduler::activate() {
    m_isActive = true;
}
void priv::PhysicsTaskScheduler::deactivate() {
    m_isActive = false;
}

#pragma endregion

#pragma region PhysicsPipeline

template <const size_t UID, typename RES, typename... ARGS>
struct fun_ptr_helper{
    public:
        typedef std::function<RES(ARGS...)> function_type;
        static void bind(function_type&& f){
            instance().fn_.swap(f);
        }
        static void bind(const function_type& f){
            instance().fn_ = f;
        }
        static RES invoke(ARGS... args){
            return instance().fn_(args...);
        }
        typedef decltype(&fun_ptr_helper::invoke) pointer_type;
        static pointer_type ptr(){
            return &invoke;
        }
    private:
        static fun_ptr_helper& instance(){
            static fun_ptr_helper inst_;
            return inst_;
        }
        fun_ptr_helper() {}
        function_type fn_;
};

template <const size_t UID, typename RES, typename... ARGS>
typename fun_ptr_helper<UID, RES, ARGS...>::pointer_type
get_fn_ptr(const std::function<RES(ARGS...)>& f){
    fun_ptr_helper<UID, RES, ARGS...>::bind(f);
    return fun_ptr_helper<UID, RES, ARGS...>::ptr();
}

priv::PhysicsPipeline::PhysicsPipeline() {
    const auto hardware_concurrency = Engine::priv::threading::hardware_concurrency();

    m_Broadphase             = new btDbvtBroadphase();
    m_CollisionConfiguration = new btDefaultCollisionConfiguration();
    m_Dispatcher             = new btCollisionDispatcher(m_CollisionConfiguration);
    if (hardware_concurrency <= 1) {
        m_Solver             = new btSequentialImpulseConstraintSolver();
        m_World              = new btDiscreteDynamicsWorld(m_Dispatcher, m_Broadphase, m_Solver, m_CollisionConfiguration);
    }else{
        m_TaskScheduler      = NEW PhysicsTaskScheduler("PhysicsTaskScheduler");
        btSetTaskScheduler(m_TaskScheduler);
        m_SolverPool         = new btConstraintSolverPoolMt(hardware_concurrency);
        m_SolverMT           = new btSequentialImpulseConstraintSolverMt();
        m_World              = new btDiscreteDynamicsWorldMt(m_Dispatcher, m_Broadphase, m_SolverPool, m_SolverMT, m_CollisionConfiguration);
    }
    m_DebugDrawer.setDebugMode(btIDebugDraw::DBG_MAX_DEBUG_DRAW_MODE);
    m_World->setDebugDrawer(&m_DebugDrawer);
    m_World->setGravity(btVector3(static_cast<btScalar>(0.0), static_cast<btScalar>(0.0), static_cast<btScalar>(0.0)));
    btGImpactCollisionAlgorithm::registerAlgorithm(m_Dispatcher);
    setPreTickCallback(m_PreTickCallback);
    setPostTickCallback(m_PostTickCallback);
}
void priv::PhysicsPipeline::setPreTickCallback(function<void(btDynamicsWorld* world, btScalar timeStep)> preTicCallback) {
    m_PreTickCallback = preTicCallback;
    btInternalTickCallback btFunc = get_fn_ptr<0>(preTicCallback);
    m_World->setInternalTickCallback(btFunc, (void*)m_World, true);
}
void priv::PhysicsPipeline::setPostTickCallback(function<void(btDynamicsWorld* world, btScalar timeStep)> postTickCallback) {
    m_PostTickCallback = postTickCallback;
    btInternalTickCallback btFunc = get_fn_ptr<0>(postTickCallback);
    m_World->setInternalTickCallback(btFunc, (void*)m_World, false);
}
void priv::PhysicsPipeline::update(const float dt) {
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
priv::PhysicsPipeline::~PhysicsPipeline() {
    //btSetTaskScheduler(nullptr);
    SAFE_DELETE(m_World);
    SAFE_DELETE(m_Solver);
    SAFE_DELETE(m_SolverPool);
    SAFE_DELETE(m_SolverMT);
    SAFE_DELETE(m_Dispatcher);
    SAFE_DELETE(m_CollisionConfiguration);
    SAFE_DELETE(m_Broadphase);
    SAFE_DELETE(m_TaskScheduler);
}

#pragma endregion