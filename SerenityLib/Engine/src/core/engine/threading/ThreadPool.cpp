#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/threading/ThreadPool.h>
#include <core/engine/utils/Utils.h>

using namespace std;
using namespace Engine;
using namespace Engine::priv;

#pragma region ThreadPoolFuture
ThreadPoolFuture::ThreadPoolFuture(std::future<void>&& future) 
    : m_Future{ std::move(future) }
{}
#pragma endregion

#pragma region ThreadPoolFutureCallback
ThreadPoolFutureCallback::ThreadPoolFutureCallback(std::future<void>&& future, std::function<void()>&& callback) 
    : m_Future{ std::move(future) }
    , m_Callback{ std::move(callback) }
{}
#pragma endregion

#pragma region ThreadPool

ThreadPool::ThreadPool(unsigned int sections) {
    m_Futures.resize(sections);
    m_FutureCallbacks.resize(sections);
    for (auto& future_section : m_Futures) {
        future_section.reserve(800);
    }
    for (auto& future_section : m_FutureCallbacks) {
        future_section.reserve(800);
    }
    m_TaskQueue.resize(sections);
}
ThreadPool::~ThreadPool() {
    shutdown();
}
void ThreadPool::shutdown() {
    if (!m_Stopped) {
        std::lock_guard lock(m_Mutex);
        m_Stopped = true;
        m_ConditionVariableAny.notify_all();// wake up all threads.
    }
}
bool ThreadPool::startup(unsigned int num_threads) {
    if (num_threads != size()) { //only shutdown if we want a different amount of threads
        shutdown();
    }
    if (m_Stopped) { //only start up if we did not start up yet or if we shutdowned
        m_Stopped = false;
        m_WorkerThreads.reserve(num_threads);
        for (unsigned int i = 0; i < num_threads; ++i) {
            auto* worker = m_WorkerThreads.add_thread([this]() {
                while (!m_Stopped) {
                    ThreadPool::PoolTaskPtr job;
                    {
                        std::unique_lock unique_lock(m_Mutex);
                        m_ConditionVariableAny.wait(unique_lock, [this] { return !(task_queue_is_empty() && !m_Stopped); });
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
void ThreadPool::add_job(std::function<void()>&& job, unsigned int section) {
    internal_create_packaged_task(std::move(job), section);
}
void ThreadPool::add_job(std::function<void()>&& job, std::function<void()>&& callback, unsigned int section) {
    internal_create_packaged_task(std::move(job), std::move(callback), section);
}
bool ThreadPool::task_queue_is_empty() const {
    for (const auto& queue : m_TaskQueue) {
        if (!queue.empty()) {
            return false;
        }
    }
    return true;
}
ThreadPool::PoolTaskPtr ThreadPool::internal_get_next_available_job() {
    ThreadPool::PoolTaskPtr ret = nullptr;
    for (auto& queue : m_TaskQueue) {
        if (!queue.empty()) {
            ret = queue.front();
            queue.pop();
            return ret;
        }
    }
    return ret;
}
void ThreadPool::internal_create_packaged_task(std::function<void()>&& job, std::function<void()>&& callback, unsigned int section) {
    auto task = std::make_shared<ThreadPool::PoolTask>(std::move(job));
    ThreadPoolFutureCallback thread_pool_future(std::move(task->get_future()), std::move(callback));
    if (size() > 0) {
        {
            std::lock_guard lock(m_Mutex);
            m_FutureCallbacks[section].emplace_back(std::move(thread_pool_future));
            m_TaskQueue[section].emplace(std::move(task));
        }
        m_ConditionVariableAny.notify_one();
    }else{
        //on single threaded, we just execute the tasks on the main thread below in update()
        m_FutureCallbacks[section].emplace_back(std::move(thread_pool_future));
        m_TaskQueue[section].emplace(std::move(task));
    }
}
void ThreadPool::internal_create_packaged_task(std::function<void()>&& job, unsigned int section) {
    auto task = std::make_shared<ThreadPool::PoolTask>(std::move(job));
    ThreadPoolFuture thread_pool_future(std::move(task->get_future()));
    if (size() > 0) {
        {
            std::lock_guard lock(m_Mutex);
            m_Futures[section].emplace_back(std::move(thread_pool_future));
            m_TaskQueue[section].emplace(std::move(task));
        }
        m_ConditionVariableAny.notify_one();
    }else{
        //on single threaded, we just execute the tasks on the main thread below in update()
        m_Futures[section].emplace_back(std::move(thread_pool_future));
        m_TaskQueue[section].emplace(std::move(task));
    }
}

void ThreadPool::internal_update_section(unsigned int section) {
    for (auto it = m_Futures[section].begin(); it != m_Futures[section].end();) {
        auto& future = (*it);
        if (future.isReady()) {
            {
                //std::lock_guard lock(m_Mutex); //hrmm....
                it = m_Futures[section].erase(it);
            }
        }else{
            ++it;
        }
    }
    for (auto it = m_FutureCallbacks[section].begin(); it != m_FutureCallbacks[section].end();) {
        auto& future = (*it);
        if (future.isReady()) {
            future();
            {
                //std::lock_guard lock(m_Mutex); //hrmm....
                it = m_FutureCallbacks[section].erase(it);
            }
        }else{
            ++it;
        }
    }
}
void ThreadPool::update() {
    if (ThreadPool::size() == 0) { //for single threaded stuff
        for (size_t i = 0; i < m_TaskQueue.size(); ++i) {
            while (m_TaskQueue[i].size() > 0) {
                auto& job = m_TaskQueue[i].front();
                (*job)();
                m_TaskQueue[i].pop();
            }
        }
    }
    //this executes the "then" functions
    for (size_t i = 0; i < m_Futures.size(); ++i) {
        internal_update_section((unsigned int)i);
    }
}
size_t ThreadPool::size() const {
    return m_WorkerThreads.size();
}
void ThreadPool::join_all() {
    for (auto& worker_thread : m_WorkerThreads) {
        if (worker_thread.joinable()) {
            worker_thread.join();
        }
    }
}
void ThreadPool::wait_for_all(unsigned int section) {
    if (ThreadPool::size() > 0) {
        //TODO: use std::experimental::when_all when it becomes available, as it should be faster than individual wait's
        for (auto& future : m_FutureCallbacks[section]) {
            future.m_Future.wait();
        }
        for (auto& future : m_Futures[section]) {
            future.m_Future.wait();
        }
    }
    update();
}

#pragma endregion