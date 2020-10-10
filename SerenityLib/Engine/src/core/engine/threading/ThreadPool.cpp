#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/threading/ThreadPool.h>
#include <core/engine/utils/Utils.h>

using namespace Engine;
using namespace Engine::priv;

#pragma region ThreadPoolFuture
ThreadPoolFuture::ThreadPoolFuture(FutureType&& future)
    : m_Future{ std::move(future) }
{}
#pragma endregion

#pragma region ThreadPoolFutureCallback
ThreadPoolFutureCallback::ThreadPoolFutureCallback(FutureType&& future, ThreadJob&& callback)
    : m_Future   { std::move(future) }
    , m_Callback { std::move(callback) }
{}
#pragma endregion

#pragma region ThreadPoolFutureContainer
void ThreadPoolFutureContainer::update_section(size_t section) noexcept {
    std::erase_if(m_FuturesBasic[section], [](const Engine::priv::ThreadPoolFuture& future) {
        return future.isReady();
    });
    std::for_each(m_FuturesCallback[section].begin(), m_FuturesCallback[section].end(), [](const Engine::priv::ThreadPoolFutureCallback& callback) {
        ASSERT(callback == true, __FUNCTION__ << "(): callback had empty function!");
        if (callback.isReady()) {
            callback(); //this executes the "then" functions
        }
    });
    std::erase_if(m_FuturesCallback[section], [](const Engine::priv::ThreadPoolFutureCallback& callback) {
        return callback.isReady();
    });
}
void ThreadPoolFutureContainer::wait_for_all(size_t section) noexcept {
    //TODO: use std::experimental::when_all when it becomes available, as it should be faster than individual wait's
    std::for_each(m_FuturesBasic[section].begin(), m_FuturesBasic[section].end(), [](const Engine::priv::ThreadPoolFuture& future) {
        if (!future.isReady()) {
            future.m_Future.wait();
        }
    });
    std::for_each(m_FuturesCallback[section].begin(), m_FuturesCallback[section].end(), [](const Engine::priv::ThreadPoolFutureCallback& callback) {
        if (!callback.isReady()) {
            callback.m_Future.wait();
        }
    });
}
#pragma endregion

#pragma region ThreadPool

ThreadPool::ThreadPool(size_t sections) {
    m_Futures.resize(sections);
    m_Futures.reserve(200);
    m_TaskQueues.resize(sections);
}
ThreadPool::~ThreadPool() {
    shutdown();
}
void ThreadPool::shutdown() {
    if (!m_Stopped) {
        m_Stopped = true;
        m_ConditionVariableAny.notify_all();// wake up all threads.
    }
}
bool ThreadPool::startup(size_t num_threads) {
    if (num_threads != size()) { //only shutdown if we want a different amount of threads
        shutdown();
    }
    if (m_Stopped) { //only start up if we did not start up yet or if we shutdowned
        m_Stopped = false;
        m_WorkerThreads.reserve(num_threads);
        for (size_t i = 0U; i < num_threads; ++i) {
            m_WorkerThreads.add_thread([this]() {
                while (!m_Stopped) {
                    Engine::priv::PoolTaskPtr job;
                    {
                        std::unique_lock uniqueLock(m_Mutex);
                        m_ConditionVariableAny.wait(uniqueLock, [this] { return !(task_queue_is_empty() && !m_Stopped); });
                        if (m_Stopped) {
                            return;
                        }
                        job = internal_get_next_available_job();
                    }
                    (*job)();
                }
            });
        }
        return true;
    }
    return false;
}
void ThreadPool::add_job(ThreadJob&& job, size_t section) {
    ASSERT(job,      __FUNCTION__ << "(ThreadJob&& job, size_t section): job was not valid!" );
    internal_create_packaged_task(std::move(job), section);
}
void ThreadPool::add_job(ThreadJob&& job, ThreadJob&& callback, size_t section) {
    ASSERT(job,      __FUNCTION__ << "(ThreadJob&& job, size_t section): job was not valid!");
    ASSERT(callback, __FUNCTION__ << "(ThreadJob&& job, ThreadJob&& callback size_t section): callback was not valid!");
    internal_create_packaged_task(std::move(job), std::move(callback), section);
}
bool ThreadPool::task_queue_is_empty() const {
    for (const auto& queue : m_TaskQueues) {
        if (!queue.empty()) {
            return false;
        }
    }
    return true;
}
Engine::priv::PoolTaskPtr ThreadPool::internal_get_next_available_job() {
    PoolTaskPtr ret = nullptr;
    for (auto& queue : m_TaskQueues) {
        if (!queue.empty()) {
            ret = queue.front();
            queue.pop();
            return ret;
        }
    }
    return ret;
}
void ThreadPool::internal_emplace(ThreadPoolFuture&& future, PoolTaskPtr&& task, size_t section) noexcept {
    m_Futures.emplace(std::move(future), section);
    m_TaskQueues[section].emplace(std::move(task));
}
void ThreadPool::internal_emplace(ThreadPoolFutureCallback&& futureCallback, PoolTaskPtr&& task, size_t section) noexcept {
    m_Futures.emplace(std::move(futureCallback), section);
    m_TaskQueues[section].emplace(std::move(task));
}
void ThreadPool::internal_create_packaged_task(ThreadJob&& job, ThreadJob&& callback, size_t section) {
    auto task = std::make_shared<PoolTask>(std::move(job));
    ThreadPoolFutureCallback thread_pool_future(std::move(task->get_future()), std::move(callback));
    if (size() > 0) {
        {
            std::lock_guard lock(m_Mutex);
            internal_emplace(std::move(thread_pool_future), std::move(task), section);
        }
        m_ConditionVariableAny.notify_one();
    }else{
        //on single threaded, we just execute the tasks on the main thread below in update()
        internal_emplace(std::move(thread_pool_future), std::move(task), section);
    }
}
void ThreadPool::internal_create_packaged_task(ThreadJob&& job, size_t section) {
    auto task = std::make_shared<PoolTask>(std::move(job));
    ThreadPoolFuture thread_pool_future(std::move(task->get_future()));
    if (size() > 0) {
        {
            std::lock_guard lock(m_Mutex);
            internal_emplace(std::move(thread_pool_future), std::move(task), section);
        }
        m_ConditionVariableAny.notify_one();
    }else{
        //on single threaded, we just execute the tasks on the main thread below in update()
        internal_emplace(std::move(thread_pool_future), std::move(task), section);
    }
}
void ThreadPool::update() {
    if (size() == 0) { //for single threaded stuff
        for (size_t i = 0; i < m_TaskQueues.size(); ++i) {
            while (m_TaskQueues[i].size() > 0) {
                (*m_TaskQueues[i].front())();
                m_TaskQueues[i].pop();
            }
        }
    }
    for (size_t i = 0; i < m_Futures.size(); ++i) {
        m_Futures.update_section(i);
    }
}
void ThreadPool::join_all() {
    for (auto& worker_thread : m_WorkerThreads) {
        if (worker_thread.joinable()) {
            worker_thread.join();
        }
    }
}
void ThreadPool::wait_for_all(size_t section) {
    if (ThreadPool::size() > 0) {
        m_Futures.wait_for_all(section);
    }
    update();
}

#pragma endregion