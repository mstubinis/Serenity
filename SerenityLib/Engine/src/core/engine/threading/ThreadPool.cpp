#include <core/engine/threading/ThreadPool.h>
#include <core/engine/utils/Utils.h>
#include <mutex>

using namespace std;
using namespace Engine;
using namespace Engine::priv;

struct EmptyCallback final {void operator()() const {}};

#pragma region ThreadPoolFuture

ThreadPoolFuture::ThreadPoolFuture(std::shared_future<void>&& future, std::function<void()>&& callback) {
    m_Future   = future;
    m_Callback = callback;
}

ThreadPoolFuture::ThreadPoolFuture(ThreadPoolFuture&& other) noexcept {
    using std::swap;
    swap(m_Future, other.m_Future);
    swap(m_Callback, other.m_Callback);
}
ThreadPoolFuture& ThreadPoolFuture::operator=(ThreadPoolFuture&& other) noexcept {
    using std::swap;
    swap(m_Future, other.m_Future);
    swap(m_Callback, other.m_Callback);
    return *this;
}

#pragma endregion

#pragma region ThreadPool

ThreadPool::ThreadPool(const unsigned int num_threads) {
    const unsigned int numberOfThreads = std::max(1U, num_threads);
    init(numberOfThreads);
}
ThreadPool::~ThreadPool() {
    if (!m_Stopped) {
        shutdown();
    }
    join_all();
}
void ThreadPool::shutdown() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Stopped = true;
    m_ConditionVariable.notify_all();// wake up all threads.
}
void ThreadPool::init(const unsigned int num_threads) {
    m_Stopped = false;
    for (unsigned int i = 0; i < num_threads; ++i) {

        auto thread_exec_func = [&](ThreadPool& pool_) {
            while (!pool_.m_Stopped) {
                std::shared_ptr<std::packaged_task<void()>> job;
                {
                    std::unique_lock<std::mutex> lock(pool_.m_Mutex);
                    pool_.m_ConditionVariable.wait(lock, [&] { return !(pool_.m_TaskQueue.empty() && !pool_.m_Stopped); });

                    if (pool_.m_Stopped) {
                        return;
                    }

                    job = (pool_.m_TaskQueue.front());
                    pool_.m_TaskQueue.pop();
                }
                (*job)();
            }
        };
        m_WorkerThreads.push_back(std::move(std::thread(std::move(thread_exec_func), std::ref(const_cast<ThreadPool&>(*this)))));
    }
}
void ThreadPool::addJob(std::function<void()>&& job) {
    EmptyCallback emptyCallback;
    internal_create_packaged_task(std::move(job), std::move(emptyCallback));
}
void ThreadPool::addJob(std::function<void()>&& job, std::function<void()>&& callback) {
    internal_create_packaged_task(std::move(job), std::move(callback));
}
void ThreadPool::internal_create_packaged_task(std::function<void()>&& job, std::function<void()>&& callback) {
    const auto task = std::make_shared<std::packaged_task<void()>>(std::move(job));
    ThreadPoolFuture thread_pool_future(std::move(std::shared_future<void>(task->get_future())), std::move(callback));
    m_Futures.push_back(std::move(thread_pool_future));
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_TaskQueue.push(std::move(task));
    }
    m_ConditionVariable.notify_one();
}
void ThreadPool::update() {
    if (m_Futures.size() == 0)
        return;
    for (auto it = m_Futures.begin(); it != m_Futures.end();) {
        auto& future = (*it);
        if (future.m_Future._Is_ready()) {
            future.m_Callback();
            it = m_Futures.erase(it);
        }
        else { ++it; }
    }
}
const size_t ThreadPool::size() const {
    return m_WorkerThreads.size();
}
void ThreadPool::join_all() {
    for (auto& worker_thread : m_WorkerThreads) {
        if (worker_thread.joinable()) {
            worker_thread.join();
        }
    }
}
void ThreadPool::wait_for_all() {
    if (m_Futures.size() > 0) {
        for (auto& future : m_Futures) {
            future.m_Future.wait();
        }
    }
    update();
}

/*
void ThreadPool::clear_all_jobs() {
    while (!m_TaskQueue.empty()) {
        auto& x = *m_TaskQueue.front();
        if (x.valid()) {
            x.reset();
        }
        m_TaskQueue.pop();
    }
    m_ConditionVariable.notify_all();// wake up all threads.
}
*/
#pragma endregion