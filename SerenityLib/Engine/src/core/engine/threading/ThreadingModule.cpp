#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/threading/ThreadingModule.h>
#include <core/engine/utils/Utils.h>

Engine::priv::ThreadingModule* threadingModule = nullptr;

Engine::priv::ThreadingModule::ThreadingModule() {
    auto hardware_concurrency = Engine::hardware_concurrency();
    if (hardware_concurrency > 1) {
        m_ThreadPool.startup(hardware_concurrency);
        ENGINE_PRODUCTION_LOG("Starting thread pool with " << hardware_concurrency)
    }
    threadingModule = this;
}
Engine::priv::ThreadingModule::~ThreadingModule(){
    cleanup();
}
void Engine::priv::ThreadingModule::cleanup() {
    m_ThreadPool.shutdown();
}
void Engine::priv::ThreadingModule::update(const float dt){
    m_ThreadPool.update();
}
void Engine::priv::threading::finalizeJob(std::function<void()>&& task, unsigned int section){
    threadingModule->m_ThreadPool.add_job(std::move(task), section);
}
void Engine::priv::threading::finalizeJob(std::function<void()>&& task, std::function<void()>&& then_task, unsigned int section){
    threadingModule->m_ThreadPool.add_job(std::move(task), std::move(then_task), section);
}
void Engine::priv::threading::waitForAll(unsigned int section){
    threadingModule->m_ThreadPool.wait_for_all(section);
}
