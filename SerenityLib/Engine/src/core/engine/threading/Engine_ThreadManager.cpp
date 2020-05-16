#include <core/engine/threading/Engine_ThreadManager.h>
#include <core/engine/utils/Utils.h>
#include <iostream>

using namespace Engine;
using namespace std;

priv::ThreadManager* threadManager = nullptr;

priv::ThreadManager::ThreadManager(){ 
    const auto hardware_concurrency = priv::threading::hardware_concurrency();
    if (hardware_concurrency > 1) {
        m_ThreadPool.startup(hardware_concurrency);
        m_ThreadPoolEngineControlled.startup(hardware_concurrency);
        cout << "Starting thread pool with " << hardware_concurrency << " threads\n";
    }
    threadManager                   = this;
}
priv::ThreadManager::~ThreadManager(){ 
    cleanup();
}
void priv::ThreadManager::cleanup() {
    m_ThreadPool.shutdown();
    m_ThreadPoolEngineControlled.shutdown();
}
void priv::ThreadManager::_update(const float dt){ 
    m_ThreadPoolEngineControlled.update();
    m_ThreadPool.update();
}
void priv::ThreadManager::wait_for_all_engine_controlled() {
    m_ThreadPoolEngineControlled.wait_for_all();
}
void priv::ThreadManager::finalize_job_engine_controlled(std::function<void()>& task) {
    m_ThreadPoolEngineControlled.add_job(std::move(task));
}
void priv::ThreadManager::finalize_job_engine_controlled(std::function<void()>& task, std::function<void()>& then_task) {
    m_ThreadPoolEngineControlled.add_job(std::move(task), std::move(then_task));
}


const unsigned int priv::threading::hardware_concurrency() {
    return std::max(1U, std::thread::hardware_concurrency());
    //return 1U;
}
vector<pair<size_t, size_t>> priv::threading::splitVectorPairs(size_t vectorSize, size_t num_cores) {
    if (num_cores == 0) {
        num_cores = Engine::priv::threading::hardware_concurrency();
    }
    vector<pair<size_t, size_t>> outVec;
    if (vectorSize <= num_cores) {
        outVec.emplace_back(make_pair(0, vectorSize - 1));
        return outVec;
    }
    outVec.reserve(num_cores);

    size_t c = vectorSize / num_cores;
    size_t remainder = vectorSize % num_cores; /* Likely uses the result of the division. */
    size_t accumulator = 0;
    std::pair<size_t, size_t> res;
    size_t b;
    size_t e = (num_cores - remainder);
    for (size_t i = 0; i < std::min(num_cores, vectorSize); ++i) {
        if (c == 0)
            b = remainder - 1;
        else
            b = accumulator + (c - 1);
        if (i == e) {
            if (i != (num_cores - remainder)) {
                ++accumulator;
                ++b;
            }
            ++b;
            ++e;
        }
        res = make_pair(accumulator, b);
        outVec.push_back(res);
        accumulator += c;
    }
    return outVec;
}
void priv::threading::finalizeJob(std::function<void()>& task){
    threadManager->m_ThreadPool.add_job(std::move(task));
}
void priv::threading::finalizeJob(std::function<void()>& task, std::function<void()>& then_task){
    threadManager->m_ThreadPool.add_job(std::move(task), std::move(then_task));
}
void priv::threading::waitForAll(){ 
    threadManager->m_ThreadPool.wait_for_all();
}
