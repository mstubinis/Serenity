
#include <serenity/threading/ThreadPool.h>
#include <serenity/threading/ThreadingModule.h>
#include <serenity/utils/Utils.h>
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
        for (auto& jThread : m_WorkerThreads) {
            jThread.get_stop_source().request_stop();
        }
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
        for (int index = 0; index < numThreads; ++index) {
            internal_add_thread(index);
        }
#endif
        return true;
    }
    return false;
}
void Engine::priv::ThreadPool::internal_add_thread(int index) {
    auto& workerThread = *m_WorkerThreads.add_thread([this](std::stop_token threadStopToken) {
        while (!m_Stopped && !threadStopToken.stop_requested()) {
            PoolTaskPtr job;
            {
                std::unique_lock lock{ m_Mutex };
                m_ConditionVariableAny.wait(lock, [this] { return m_Stopped || !m_TaskQueue.empty(); });
                if (m_Stopped) {
                    return;
                }
                assert(!m_TaskQueue.empty());
                job = m_TaskQueue.front();
                m_TaskQueue.pop();
            }
            assert(!m_Stopped && job.get() != nullptr);
            job->operator()();
            m_WaitCounter -= 1;
        }
    }, index);
    Engine::priv::threading::setThreadName(workerThread, "worker_thread_" + std::to_string(index));
}
std::optional<std::stop_token> Engine::priv::ThreadPool::getThreadStopToken() noexcept {
    return m_WorkerThreads.contains(std::this_thread::get_id()) ? m_WorkerThreads[std::this_thread::get_id()]->get_stop_token() : std::optional<std::stop_token>{};
}
bool Engine::priv::ThreadPool::isWorkerThreadStopped() const noexcept {
    return m_WorkerThreads.contains(std::this_thread::get_id()) ? m_WorkerThreads[std::this_thread::get_id()]->get_stop_token().stop_requested() : false;
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
    std::scoped_lock lock{ m_Mutex };
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
        while (m_WaitCounter.load(std::memory_order_relaxed) > 0) {
            std::this_thread::yield();
        }
        //while (m_WaitCounter > 0);
    }
#endif
    update();
}