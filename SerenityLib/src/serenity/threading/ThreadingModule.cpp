
#include <serenity/threading/ThreadingModule.h>
#include <serenity/system/window/Window.h>
#include <serenity/system/Engine.h>

using namespace Engine::priv;

Engine::view_ptr<ThreadingModule> ThreadingModule::THREADING_MODULE = nullptr;

ThreadingModule::ThreadingModule() {
    auto hardware_concurrency = Engine::hardware_concurrency();
    if (hardware_concurrency > 1) {
        m_ThreadPool.startup(hardware_concurrency);
        ENGINE_PRODUCTION_LOG("Starting thread pool with " << hardware_concurrency << " threads")
    }
    THREADING_MODULE = this;
}
ThreadingModule::~ThreadingModule() {
    cleanup();
}
void ThreadingModule::cleanup() {
    m_ThreadPool.shutdown();
}
void ThreadingModule::update(const float dt) {
    m_ThreadPool.update(); 
}
void Engine::priv::threading::submitTaskForMainThread(std::function<void()>&& task) noexcept {
    Engine::priv::Core::m_Engine->m_Misc.m_QueuedCommands.push(std::move(task));
}
void Engine::priv::threading::waitForAll(size_t section = 0) noexcept {
    ThreadingModule::THREADING_MODULE->m_ThreadPool.wait_for_all(section);
}
bool Engine::priv::threading::isMainThread() noexcept {
    return std::this_thread::get_id() == Engine::priv::Core::m_Engine->m_Misc.m_MainThreadID;
}
bool Engine::priv::threading::isOpenGLThread() noexcept {
    return std::this_thread::get_id() == Engine::Resources::getWindow().getOpenglThreadID();
}