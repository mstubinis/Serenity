
#include <serenity/threading/WorkerThreadContainer.h>

#ifdef _WIN32
#include <windows.h>
#endif

void Engine::priv::WorkerThreadContainer::clear() noexcept {
    m_WorkerThreadsHashed.clear();
    m_WorkerThreads.clear();
}
void Engine::priv::WorkerThreadContainer::reserve(size_t newReserveSize) noexcept {
    m_WorkerThreadsHashed.reserve(newReserveSize);
    m_WorkerThreads.reserve(newReserveSize);
}

Engine::view_ptr<Engine::priv::WorkerThreadContainer::ThreadType> Engine::priv::WorkerThreadContainer::add_thread(ThreadFunction&& func, int index) {
    if (m_WorkerThreads.size() >= m_WorkerThreads.capacity()) {
        ENGINE_PRODUCTION_LOG(__FUNCTION__ << "(): m_WorkerThreads reached its capacity!");
        return nullptr;
    }
    auto& workerThread = m_WorkerThreads.emplace_back(std::move(func));
    m_WorkerThreadsHashed.emplace(std::piecewise_construct, std::forward_as_tuple(workerThread.get_id()), std::forward_as_tuple(&workerThread));
#ifdef _WIN32
    DWORD_PTR dw = SetThreadAffinityMask(workerThread.native_handle(), DWORD_PTR(1) << index);
    if (dw == 0) {
        DWORD dwErr = GetLastError();
        ENGINE_PRODUCTION_LOG(__FUNCTION__ << "(): SetThreadAffinityMask failed, error = " << dwErr << '\n');
    }
#endif
    return &workerThread;
}