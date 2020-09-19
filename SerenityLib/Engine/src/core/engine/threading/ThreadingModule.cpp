#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/threading/ThreadingModule.h>
#include <core/engine/utils/Utils.h>

using namespace Engine;
using namespace std;

priv::ThreadingModule* threadingModule = nullptr;

priv::ThreadingModule::ThreadingModule(){
    auto hardware_concurrency = Engine::hardware_concurrency();
    if (hardware_concurrency > 1) {
        m_ThreadPool.startup(hardware_concurrency);
        ENGINE_PRODUCTION_LOG("Starting thread pool with " << hardware_concurrency)
    }
    threadingModule = this;
}
priv::ThreadingModule::~ThreadingModule(){
    cleanup();
}
void priv::ThreadingModule::cleanup() {
    m_ThreadPool.shutdown();
}
void priv::ThreadingModule::update(const float dt){
    m_ThreadPool.update();
}
void priv::threading::finalizeJob(std::function<void()>& task, unsigned int section){
    threadingModule->m_ThreadPool.add_job(std::move(task), section);
}
void priv::threading::finalizeJob(std::function<void()>& task, std::function<void()>& then_task, unsigned int section){
    threadingModule->m_ThreadPool.add_job(std::move(task), std::move(then_task), section);
}
void priv::threading::waitForAll(unsigned int section){
    threadingModule->m_ThreadPool.wait_for_all(section);
}
