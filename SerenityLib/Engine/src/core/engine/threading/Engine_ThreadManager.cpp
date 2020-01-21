#include <core/engine/threading/Engine_ThreadManager.h>
#include <core/engine/threading/ThreadPool.h>
#include <core/engine/utils/Utils.h>

using namespace Engine;
using namespace std;

priv::ThreadManager* threadManager;

priv::ThreadManager::ThreadManager(){ 
    m_ThreadPool = NEW ThreadPool(std::thread::hardware_concurrency());
    threadManager = this;
}
priv::ThreadManager::~ThreadManager(){ 
    cleanup();
}
void priv::ThreadManager::cleanup() {
    m_ThreadPool->shutdown();
    SAFE_DELETE(m_ThreadPool);
}
void priv::ThreadManager::_update(const double& dt){ 
    m_ThreadPool->update();
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