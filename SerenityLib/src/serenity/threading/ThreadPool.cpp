
#include <serenity/threading/ThreadPool.h>

using namespace Engine;
using namespace Engine::priv;

#pragma region ThreadPoolFuture
ThreadPoolFuture::ThreadPoolFuture(FutureType&& future) {
    m_Future = std::move(future);
}
#pragma endregion

#pragma region ThreadPoolFutureCallback
ThreadPoolFutureCallback::ThreadPoolFutureCallback(FutureType&& future, ThreadJob&& callback){
    m_Future   = std::move(future);
    m_Callback = std::move(callback);
}
#pragma endregion

#pragma region ThreadPool

ThreadPool::ThreadPool(size_t sections) {
    m_FuturesBasic.resize(sections);
    m_FuturesCallback.resize(sections);

    for (auto& section : m_FuturesBasic) { section.reserve(1200); }
    for (auto& section : m_FuturesCallback) { section.reserve(1200); }

    m_TaskQueues.resize(sections);
}
ThreadPool::~ThreadPool() {
    shutdown();
}
void ThreadPool::shutdown() noexcept {
    if (!m_Stopped) {
        m_Stopped = true;
        m_ConditionVariableAny.notify_all();// wake up all threads.
        m_WorkerThreads.clear();
    }
}
size_t ThreadPool::get_number_of_tasks_in_queue() const noexcept {
    size_t count = 0;
    for (const auto& taskQueue : m_TaskQueues) {
        count += taskQueue.size();
    }
    return count;
}
bool ThreadPool::startup(size_t numThreads) {
    if (numThreads != size()) { //only shutdown if we want a different amount of threads
        shutdown();
    }
    if (m_Stopped) { //only start up if we did not start up yet or if we shutdowned
        m_Stopped = false;
        m_WorkerThreads.reserve(numThreads);
        for (size_t i = 0U; i < numThreads; ++i) {
            m_WorkerThreads.add_thread([this]() {
                while (!m_Stopped) {
                    Engine::priv::PoolTaskPtr job;
                    {
                        std::unique_lock lock{ m_Mutex };
                        m_ConditionVariableAny.wait(lock, [this] { return m_Stopped || !internal_task_queue_is_empty(); });
                        if (m_Stopped) {
                            return;
                        }
                        job = internal_get_next_available_job();
                    }
                    ASSERT((!m_Stopped && job) || (m_Stopped && !job), __FUNCTION__ << "(): job was nullptr!");
                    job->operator()();
                }
            });
        }
        return true;
    }
    return false;
}
bool ThreadPool::internal_task_queue_is_empty() const noexcept {
    for (const auto& queue : m_TaskQueues) {
        if (!queue.empty()) {
            return false;
        }
    }
    return true;
}
Engine::priv::PoolTaskPtr ThreadPool::internal_get_next_available_job() noexcept {
    for (auto& queue : m_TaskQueues) {
        if (!queue.empty()) {
            PoolTaskPtr task = queue.front();
            queue.pop();
            return task;
        }
    }
    return nullptr;
}
void ThreadPool::update() {
    if (size() == 0) { //for single threaded stuff
        for (auto& queue : m_TaskQueues) {
            while (queue.size() > 0) {
                queue.front()->operator()();
                queue.pop();
            }
        }
    }else{
        std::lock_guard lock{ m_Mutex };
        //this CANNOT be split up in different loops / steps: future is_ready MIGHT be false for the first run,
        //and then true for the second run, in which case it gets removed without calling its then function
        for (auto& callbackSection : m_FuturesCallback) {
            auto it = callbackSection.begin();
            while (it != callbackSection.end()) {
                if ((*it).isReady()) {
                    (*it)(); //calls the "then" function
                    it = callbackSection.erase(it);
                }
                else {
                    ++it;
                }
            }
        }
        for (auto& basicSection : m_FuturesBasic) {
            auto it = basicSection.begin();
            while (it != basicSection.end()) {
                if ((*it).isReady()) {
                    it = basicSection.erase(it);
                }
                else {
                    ++it;
                }
            }
        }
    }
}
void ThreadPool::wait_for_all(size_t section) noexcept {
    if (size() > 0) {
        //TODO: use std::experimental::when_all when it becomes available, as it should be faster than individual wait's
        for (auto& future : m_FuturesBasic[section]) {
            if (!future.isReady()) {
                future.m_Future.wait();
            }
        }
        for (auto& callback : m_FuturesCallback[section]) {
            if (!callback.isReady()) {
                callback.m_Future.wait();
            }
        }
    }
    update();
}


#pragma endregion