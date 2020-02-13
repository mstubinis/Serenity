#include <core/engine/threading/Engine_ThreadManager.h>
#include <core/engine/threading/ThreadPool.h>
#include <core/engine/utils/Utils.h>

using namespace Engine;
using namespace std;

priv::ThreadManager* threadManager;

priv::ThreadManager::ThreadManager(){ 
    m_ThreadPool = NEW ThreadPool(std::thread::hardware_concurrency());
    m_ThreadPoolEngineControlled = NEW ThreadPool(std::thread::hardware_concurrency());
    threadManager = this;
}
priv::ThreadManager::~ThreadManager(){ 
    cleanup();
}
void priv::ThreadManager::cleanup() {
    m_ThreadPool->shutdown();
    m_ThreadPoolEngineControlled->shutdown();
    SAFE_DELETE(m_ThreadPool);
    SAFE_DELETE(m_ThreadPoolEngineControlled);
}
void priv::ThreadManager::_update(const float& dt){ 
    m_ThreadPool->update();
}
void priv::threading::addJobRef(std::function<void()>& func) {
    finalizeJob(func);
}
void priv::threading::finalizeJob(std::function<void()>& task){
    threadManager->m_ThreadPool->addJob(std::move(task));  
}
void priv::threading::finalizeJob(std::function<void()>& task, std::function<void()>& then_task){
    threadManager->m_ThreadPool->addJob(std::move(task), std::move(then_task));
}
void priv::threading::waitForAll(){ 
    threadManager->m_ThreadPool->wait_for_all();
}




void priv::ThreadManager::add_job_ref_engine_controlled(std::function<void()>& func) {
    finalize_job_engine_controlled(func);
}
void priv::ThreadManager::wait_for_all_engine_controlled() {
    m_ThreadPoolEngineControlled->wait_for_all();
}
void priv::ThreadManager::finalize_job_engine_controlled(std::function<void()>& task) {
    m_ThreadPoolEngineControlled->addJob(std::move(task));
}
void priv::ThreadManager::finalize_job_engine_controlled(std::function<void()>& task, std::function<void()>& then_task) {
    m_ThreadPoolEngineControlled->addJob(std::move(task), std::move(then_task));
}