#include <serenity/physics/PhysicsTaskScheduler.h>
#include <serenity/threading/ThreadingModule.h>

#pragma region PhysicsTaskScheduler

Engine::priv::PhysicsTaskScheduler::PhysicsTaskScheduler(const char* name)
    : btITaskScheduler{ name }
    , m_name{ name }
{
    m_sumRes = btScalar{ 0.0 };
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
    auto parallel = [&body, iEnd, iBegin]() {
        auto pairs = Engine::splitVectorPairs(iEnd - iBegin);
        for (size_t i = 0; i < pairs.size(); ++i) {
            auto lambda = [&body, i, iBegin, &pairs]() {
                body.forLoop((int)pairs[i].first + iBegin, (int)pairs[i].second + 1 + iBegin);
            };
            Engine::priv::threading::addJob(lambda);
        }
        Engine::priv::threading::waitForAll();
    };
    if (m_DoConcurrency) {
        parallel();
    }else{
        body.forLoop(iBegin, iEnd);
    }
}
btScalar Engine::priv::PhysicsTaskScheduler::parallelSum(int iBegin, int iEnd, int grainSize, const btIParallelSumBody& body) {
    auto parallel = [this, &body, iEnd, iBegin]() -> btScalar {
        auto pairs = Engine::splitVectorPairs(iEnd - iBegin);
        m_sumRes = btScalar(0.0);
        for (size_t i = 0; i < pairs.size(); ++i) {
            auto lambda = [&body, this, i, iBegin, &pairs]() {
#ifdef ENVIRONMENT64
                m_sumRes += body.sumLoop((int)pairs[i].first + iBegin, (int)pairs[i].second + 1 + iBegin);
#else
                auto data = m_sumRes.load();
                data += body.sumLoop((int)pairs[i].first, (int)pairs[i].second + 1);
                m_sumRes = data;
#endif
            };
            Engine::priv::threading::addJob(lambda);
        }
        Engine::priv::threading::waitForAll();
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
