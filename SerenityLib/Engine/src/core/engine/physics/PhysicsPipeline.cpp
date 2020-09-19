#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/physics/PhysicsPipeline.h>
#include <core/engine/system/Engine.h>
#include <core/engine/threading/ThreadingModule.h>
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

using namespace Engine;

#pragma region PhysicsTaskScheduler

priv::PhysicsTaskScheduler::PhysicsTaskScheduler(const char* name) 
    : btITaskScheduler{ name }
    , m_name{ name }
{
    m_sumRes.store(btScalar(0.0), std::memory_order_relaxed);
}
priv::PhysicsTaskScheduler::~PhysicsTaskScheduler() {

}
int priv::PhysicsTaskScheduler::getMaxNumThreads() const {
    return Engine::hardware_concurrency();
}
int priv::PhysicsTaskScheduler::getNumThreads() const {
    return Engine::hardware_concurrency();
}
void priv::PhysicsTaskScheduler::setNumThreads(int numThreads) {

}
void priv::PhysicsTaskScheduler::parallelFor(int iBegin, int iEnd, int grainSize, const btIParallelForBody& body) {
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
btScalar priv::PhysicsTaskScheduler::parallelSum(int iBegin, int iEnd, int grainSize, const btIParallelSumBody& body) {
    auto parallel = [this, &body, iEnd, iBegin]() {
        auto pairs = Engine::splitVectorPairs(iEnd - iBegin, 0);
        m_sumRes.store(btScalar(0.0), std::memory_order_relaxed);
        for (size_t i = 0; i < pairs.size(); ++i) {
            auto lambda = [&body, this, i, &pairs]() {
                #ifdef ENVIRONMENT64
                    m_sumRes += body.sumLoop((int)pairs[i].first, (int)pairs[i].second + 1);
                #else
                    auto data = m_sumRes.load();
                    data += body.sumLoop((int)pairs[i].first, (int)pairs[i].second + 1);
                    m_sumRes.store(data, std::memory_order_relaxed);
                #endif
            };
            Engine::priv::threading::addJob(lambda, 0);
        }
        Engine::priv::threading::waitForAll(0);
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
    m_World->setGravity(btVector3((btScalar)0.0, (btScalar)0.0, (btScalar)0.0));
    btGImpactCollisionAlgorithm::registerAlgorithm(m_Dispatcher.get());
    setPreTickCallback(m_PreTickCallback);
    setPostTickCallback(m_PostTickCallback);
}
void priv::PhysicsPipeline::setPreTickCallback(std::function<void(btDynamicsWorld* world, btScalar timeStep)> preTicCallback) {
    m_PreTickCallback = preTicCallback;
    btInternalTickCallback btFunc = get_fn_ptr<0>(preTicCallback);
    m_World->setInternalTickCallback(btFunc, (void*)m_World.get(), true);
}
void priv::PhysicsPipeline::setPostTickCallback(std::function<void(btDynamicsWorld* world, btScalar timeStep)> postTickCallback) {
    m_PostTickCallback = postTickCallback;
    btInternalTickCallback btFunc = get_fn_ptr<0>(postTickCallback);
    m_World->setInternalTickCallback(btFunc, (void*)m_World.get(), false);
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
}

#pragma endregion