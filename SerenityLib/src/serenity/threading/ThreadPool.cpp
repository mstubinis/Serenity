
#include <serenity/threading/ThreadPool.h>
#include <numeric>

namespace {
    inline bool isFutureReady(const Engine::priv::FutureType& future) noexcept {
        return (future.valid() && future.wait_for(0s) == std::future_status::ready);
    }
}

Engine::priv::ThreadPool::ThreadPool(int sections) {
    m_FuturesCallback.resize(sections);
    m_WaitCounters = std::make_unique<std::vector<std::atomic<int32_t>>>(sections);
    for (auto& counter : *m_WaitCounters) {
        counter = 0;
    }
    for (auto& section : m_FuturesCallback) {
        section.reserve(1200); 
    }
    m_TaskQueues.resize(sections);
}
Engine::priv::ThreadPool::~ThreadPool() {
    shutdown();
}
void Engine::priv::ThreadPool::shutdown() noexcept {
    if (!m_Stopped) {
        for (auto& counter : *m_WaitCounters) {
            counter = 0;
        }
        m_Stopped = true;
        m_ConditionVariableAny.notify_all();// wake up all threads.
        m_WorkerThreads.clear();
    }
}
size_t Engine::priv::ThreadPool::get_number_of_tasks_in_queue() const noexcept {
    return std::accumulate(std::begin(m_TaskQueues), std::end(m_TaskQueues), 0U, [](size_t c, const auto& data) { return c + data.size(); });
}
bool Engine::priv::ThreadPool::startup(int numThreads) {
    if (numThreads != size()) { //only shutdown if we want a different amount of threads
        shutdown();
    }
    if (m_Stopped) { //only start up if we did not start up yet or if we shutdowned
        m_Stopped = false;
        m_WorkerThreads.reserve(numThreads);
        for (int i = 0; i < numThreads; ++i) {
            m_WorkerThreads.add_thread([this]() {
                while (!m_Stopped) {
                    std::pair<Engine::priv::PoolTaskPtr, int> job;
                    {
                        std::unique_lock lock{ m_Mutex };
                        m_ConditionVariableAny.wait(lock, [this] { return m_Stopped || !internal_task_queue_is_empty(); });
                        if (m_Stopped) {
                            return;
                        }
                        job = internal_get_next_available_job();
                    }
                    ASSERT((!m_Stopped && job.first) || (m_Stopped && !job.first), __FUNCTION__ << "(): job was nullptr!");
                    job.first->operator()();
                    auto& counter = (*m_WaitCounters)[job.second];
                    counter--;
                }
            });
        }
        return true;
    }
    return false;
}
bool Engine::priv::ThreadPool::internal_task_queue_is_empty() const noexcept {
    for (const auto& queue : m_TaskQueues) {
        if (!queue.empty()) { 
            return false; 
        }
    }
    return true;
}
std::pair<Engine::priv::PoolTaskPtr, int> Engine::priv::ThreadPool::internal_get_next_available_job() noexcept {
    for (int section = 0; section < static_cast<int>(m_TaskQueues.size()); ++section) {
        if (!m_TaskQueues[section].empty()) {
            PoolTaskPtr task = m_TaskQueues[section].front();
            m_TaskQueues[section].pop();
            return { task, section };
        }
    }
    return { nullptr, -1 };
}
void Engine::priv::ThreadPool::internal_update_single_threaded() {
    for (auto& queue : m_TaskQueues) {
        while (queue.size() > 0) {
            queue.front()->operator()();
            queue.pop();
        }
    }
}
void Engine::priv::ThreadPool::internal_update_multi_threaded() {
    std::lock_guard lock{ m_Mutex };   
    //this CANNOT be split up in different loops / steps: future is_ready MIGHT be false for the first run,
    //and then true for the second run, in which case it gets removed without calling its then function
    for (auto& callbackSection : m_FuturesCallback) {
        callbackSection.erase(std::remove_if(std::begin(callbackSection), std::end(callbackSection), [](const auto& future) {
            if (isFutureReady(future.first)) {
                future.second();
                return true;
            }
            return false;
        }), std::end(callbackSection));
    }
}
void Engine::priv::ThreadPool::update() {
    if (size() == 0) {
        internal_update_single_threaded();
    } else {
        internal_update_multi_threaded();
    }
}
void Engine::priv::ThreadPool::wait_for_all(int section) noexcept {
    if (size() > 0) {
        while ((*m_WaitCounters)[section].load(std::memory_order_relaxed) > 0);
    }
    update();
}
