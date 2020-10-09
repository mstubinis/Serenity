#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/threading/ThreadPool.h>
#include <core/engine/utils/Utils.h>

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

#pragma region ThreadPoolFutureContainer
void ThreadPoolFutureContainer::update_section(unsigned int section) noexcept {
    for (auto future = m_Sections[section].begin(); future != m_Sections[section].end();) {
        if (future->isReady()) {
            future = m_Sections[section].erase(future);
        }else{
            ++future;
        }
    }
    for (auto future = m_Callbacks[section].begin(); future != m_Callbacks[section].end();) {
        if (future->isReady()) {
            (*future)();
            future = m_Callbacks[section].erase(future);
        }else{
            ++future;
        }
    }
}
void ThreadPoolFutureContainer::wait_for_all(unsigned int section) noexcept {
    //TODO: use std::experimental::when_all when it becomes available, as it should be faster than individual wait's
    for (auto& future : m_Sections[section]) {
        if (!future.isReady()) {
            future.m_Future.wait();
        }
    }
    for (auto& futureCallback : m_Callbacks[section]) {
        if (!futureCallback.isReady()) {
            futureCallback.m_Future.wait();
        }
    }
}
#pragma endregion

#pragma region ThreadPool

ThreadPool::ThreadPool(unsigned int sections) {
    m_Futures.resize(sections);
    m_Futures.reserve(800);
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
                    Engine::priv::PoolTaskPtr job;
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
Engine::priv::PoolTaskPtr ThreadPool::internal_get_next_available_job() {
    PoolTaskPtr ret = nullptr;
    for (auto& queue : m_TaskQueue) {
        if (!queue.empty()) {
            ret = queue.front();
            queue.pop();
            return ret;
        }
    }
    return ret;
}
void ThreadPool::internal_emplace(ThreadPoolFuture&& future, Engine::priv::PoolTaskPtr&& task, unsigned int section) noexcept {
    m_Futures.emplace(std::move(future), section);
    m_TaskQueue[section].emplace(std::move(task));
}
void ThreadPool::internal_emplace(ThreadPoolFutureCallback&& futureCallback, Engine::priv::PoolTaskPtr&& task, unsigned int section) noexcept {
    m_Futures.emplace(std::move(futureCallback), section);
    m_TaskQueue[section].emplace(std::move(task));
}
void ThreadPool::internal_create_packaged_task(std::function<void()>&& job, std::function<void()>&& callback, unsigned int section) {
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
void ThreadPool::internal_create_packaged_task(std::function<void()>&& job, unsigned int section) {
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
    if (ThreadPool::size() == 0) { //for single threaded stuff
        for (size_t i = 0; i < m_TaskQueue.size(); ++i) {
            while (m_TaskQueue[i].size() > 0) {
                (*m_TaskQueue[i].front())();
                m_TaskQueue[i].pop();
            }
        }
    }
    //this executes the "then" functions
    for (size_t i = 0; i < m_Futures.size(); ++i) {
        m_Futures.update_section(i);
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
        m_Futures.wait_for_all(section);
    }
    update();
}

#pragma endregion