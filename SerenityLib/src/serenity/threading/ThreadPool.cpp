
#include <serenity/threading/ThreadPool.h>
#include <numeric>

namespace {
    [[nodiscard]] inline bool isFutureReady(const Engine::priv::FutureType& future) noexcept {
        return (future.valid() && future.wait_for(0s) == std::future_status::ready);
    }
}

Engine::priv::ThreadPool::ThreadPool() {
    #if !defined(ENGINE_FORCE_NO_THEAD_POOL)
        m_FuturesCallback.reserve(1200);
    #endif
}
Engine::priv::ThreadPool::~ThreadPool() {
    shutdown();
}
void Engine::priv::ThreadPool::shutdown() noexcept {
    if (!m_Stopped) {
        m_WaitCounter = 0;
        m_Stopped = true;
        m_ConditionVariableAny.notify_all();// wake up all threads.
        m_WorkerThreads.clear();
    }
}
bool Engine::priv::ThreadPool::startup(int numThreads) {
#if defined(ENGINE_FORCE_NO_THEAD_POOL)
    numThreads = 0;
#endif
    if (numThreads != size()) { //only shutdown if we want a different amount of threads
        shutdown();
    }
    if (m_Stopped) { //only start up if we did not start up yet or if we shutdowned
        m_Stopped = false;
#if !defined(ENGINE_FORCE_NO_THEAD_POOL)
        m_WorkerThreads.reserve(numThreads);
        for (int i = 0; i < numThreads; ++i) {
            m_WorkerThreads.add_thread([this]() {
                while (!m_Stopped) {
                    PoolTaskPtr job;
                    {
                        std::unique_lock lock{ m_Mutex };
                        m_ConditionVariableAny.wait(lock, [this] { return m_Stopped || !m_TaskQueue.empty(); });
                        if (m_Stopped) {
                            return;
                        }
                        job = m_TaskQueue.front();
                        m_TaskQueue.pop();
                    }
                    ASSERT(!m_Stopped && job.get() != nullptr, __FUNCTION__ << "(): job was nullptr!");
                    job->operator()();
                    m_WaitCounter -= 1;
                }
            });
        }
#endif
        return true;
    }
    return false;
}
void Engine::priv::ThreadPool::internal_execute_callbacks() {
    m_FuturesCallback.erase(std::remove_if(std::begin(m_FuturesCallback), std::end(m_FuturesCallback), [](const auto& future) {
        if (isFutureReady(future.first)) {
            future.second();
            return true;
        }
        return false;
    }), std::end(m_FuturesCallback));
}
void Engine::priv::ThreadPool::internal_update_single_threaded() {
    while (m_TaskQueue.size() > 0) {
        m_TaskQueue.front()->operator()();
        m_TaskQueue.pop();
    }
    internal_execute_callbacks();
}
void Engine::priv::ThreadPool::internal_update_multi_threaded() {
    std::lock_guard lock{ m_Mutex };   
    //this CANNOT be split up in different loops / steps: future is_ready MIGHT be false for the first run,
    //and then true for the second run, in which case it gets removed without calling its then function
    internal_execute_callbacks();
}
void Engine::priv::ThreadPool::update() {
    if (size() == 0) {
        internal_update_single_threaded();
    } else {
        internal_update_multi_threaded();
    }
}
void Engine::priv::ThreadPool::wait_for_all() noexcept {
#if !defined(ENGINE_FORCE_NO_THEAD_POOL)
    if (size() > 0) {
        while (m_WaitCounter > 0);
        //while (m_WaitCounter.load(std::memory_order_relaxed) > 0);
    }
#endif
    update();
}
