#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/threading/ThreadingModule.h>
#include <core/engine/utils/Utils.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/system/window/Window.h>

using namespace Engine::priv;

Engine::view_ptr<ThreadingModule> ThreadingModule::THREADING_MODULE = nullptr;

ThreadingModule::ThreadingModule() {
    auto hardware_concurrency = Engine::hardware_concurrency();
    if (hardware_concurrency > 1) {
        m_ThreadPool.startup(hardware_concurrency);
        ENGINE_PRODUCTION_LOG("Starting thread pool with " << hardware_concurrency)
    }
    THREADING_MODULE = this;
}
ThreadingModule::~ThreadingModule(){
    cleanup();
}
void ThreadingModule::cleanup() {
    m_ThreadPool.shutdown();
}
void ThreadingModule::update(const float dt){
    m_ThreadPool.update();
}
bool Engine::priv::threading::isMainThread() noexcept {
    return std::this_thread::get_id() == Engine::Resources::getWindow().getOpenglThreadID();
}
void Engine::priv::threading::finalizeJob(std::function<void()>&& task, size_t section){
    ThreadingModule::THREADING_MODULE->m_ThreadPool.add_job(std::move(task), section);
}
void Engine::priv::threading::finalizeJob(std::function<void()>&& task, std::function<void()>&& then_task, size_t section){
    ThreadingModule::THREADING_MODULE->m_ThreadPool.add_job(std::move(task), std::move(then_task), section);
}
void Engine::priv::threading::waitForAll(size_t section){
    ThreadingModule::THREADING_MODULE->m_ThreadPool.wait_for_all(section);
}
