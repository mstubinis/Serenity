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
void priv::threading::finalizeJob(std::function<void()>& task){
    threadManager->m_ThreadPool.add_job(std::move(task));
}
void priv::threading::finalizeJob(std::function<void()>& task, std::function<void()>& then_task){
    threadManager->m_ThreadPool.add_job(std::move(task), std::move(then_task));
}
void priv::threading::waitForAll(){ 
    threadManager->m_ThreadPool.wait_for_all();
}
