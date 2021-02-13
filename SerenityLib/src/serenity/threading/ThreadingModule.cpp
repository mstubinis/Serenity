
#include <serenity/threading/ThreadingModule.h>
#include <serenity/system/window/Window.h>
#include <serenity/system/Engine.h>

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
void Engine::priv::threading::submitTaskForMainThread(std::function<void()>&& task) noexcept {
    Engine::priv::Core::m_Engine->m_Misc.m_QueuedCommands.push(std::move(task));
}
bool Engine::priv::threading::isMainThread() noexcept {
    return std::this_thread::get_id() == Engine::priv::Core::m_Engine->m_Misc.m_MainThreadID;
}
bool Engine::priv::threading::isOpenGLThread() noexcept {
    return std::this_thread::get_id() == Engine::Resources::getWindow().getOpenglThreadID();
}