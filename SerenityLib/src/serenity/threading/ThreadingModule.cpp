
#include <serenity/threading/ThreadingModule.h>
#include <serenity/system/window/Window.h>
#include <serenity/system/Engine.h>

Engine::view_ptr<Engine::priv::ThreadingModule> Engine::priv::ThreadingModule::THREADING_MODULE = nullptr;

Engine::priv::ThreadingModule::ThreadingModule() {
    auto hardware_concurrency = Engine::hardware_concurrency();
    if (hardware_concurrency > 1) {
        m_ThreadPool.startup(hardware_concurrency);
        ENGINE_PRODUCTION_LOG("Starting thread pool with " << hardware_concurrency << " threads")
    }
    THREADING_MODULE = this;
}
Engine::priv::ThreadingModule::~ThreadingModule() {
    cleanup();
}
void Engine::priv::ThreadingModule::cleanup() {
    m_ThreadPool.shutdown();
}
void Engine::priv::ThreadingModule::update(const float dt) {
    m_ThreadPool.update(); 
}
void Engine::priv::threading::submitTaskForMainThread(std::function<void()>&& task) noexcept {
    Engine::priv::Core::m_Engine->m_Misc.m_QueuedCommands.push(std::move(task));
}
void Engine::priv::threading::waitForAll(int section) noexcept {
    ThreadingModule::THREADING_MODULE->m_ThreadPool.wait_for_all(section);
}
bool Engine::priv::threading::isMainThread() noexcept {
    return std::this_thread::get_id() == Engine::priv::Core::m_Engine->m_Misc.m_MainThreadID;
}
bool Engine::priv::threading::isOpenGLThread() noexcept {
    return std::this_thread::get_id() == Engine::Resources::getWindow().getOpenglThreadID();
}