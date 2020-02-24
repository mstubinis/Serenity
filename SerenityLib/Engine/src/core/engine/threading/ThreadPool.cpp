#include <core/engine/threading/ThreadPool.h>
#include <core/engine/utils/Utils.h>
#include <mutex>

using namespace std;
using namespace Engine;
using namespace Engine::priv;

struct EmptyCallback final {void operator()() const {}};

#pragma region ThreadPoolFuture

ThreadPoolFuture::ThreadPoolFuture(std::future<void>&& future, std::function<void()>&& callback) {
    m_Future   = std::move(future);
    m_Callback = std::move(callback);
}
ThreadPoolFuture::ThreadPoolFuture(ThreadPoolFuture&& other) noexcept {
    m_Future   = std::move(other.m_Future);
    m_Callback.swap(other.m_Callback);
}
ThreadPoolFuture& ThreadPoolFuture::operator=(ThreadPoolFuture&& other) noexcept {
    if (&other != this) {
        m_Future   = std::move(other.m_Future);
        m_Callback.swap(other.m_Callback);
    }
    return *this;
}
const bool ThreadPoolFuture::isReady() {
    return (m_Future._Is_ready() && m_Future.valid());
}
void ThreadPoolFuture::operator()() const {
    if (m_Callback) {
        m_Callback();
    }
}


#pragma endregion

#pragma region ThreadPool

ThreadPool::ThreadPool() {
    m_Stopped = true;
    m_Futures.reserve(500);
}
ThreadPool::~ThreadPool() {
    shutdown();
}
void ThreadPool::shutdown() {
    if (m_Stopped == false) {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_Stopped = true;
        m_ConditionVariable.notify_all();// wake up all threads.
    }
    join_all();
}
const bool ThreadPool::startup(const unsigned int num_threads) {
    if (num_threads != size()) { //only shutdown if we want a different amount of threads
        shutdown();
    }
    if (m_Stopped) { //only start up if we did not start up yet or if we shutdowned
        m_Stopped = false;
        m_WorkerThreads.reserve(num_threads);
        for (unsigned int i = 0; i < num_threads; ++i) {
            auto thread_exec_func = [&]() {
                while (!m_Stopped) {
                    std::shared_ptr<std::packaged_task<void()>> job;
                    {
                        std::unique_lock<std::mutex> lock(m_Mutex);
                        m_ConditionVariable.wait(lock, [&] {
                            return !(m_TaskQueue.empty() && !m_Stopped);
                        });
                        if (m_Stopped) {
                            return;
                        }
                        job = (m_TaskQueue.front());
                        m_TaskQueue.pop();
                    }
                    (*job)();
                }
            };
            m_WorkerThreads.emplace_back(thread_exec_func);
        }
        return true;
    }
    return false;
}
void ThreadPool::add_job(std::function<void()>&& job) {
    EmptyCallback empty;
    internal_create_packaged_task(std::move(job), std::move(empty));
}
void ThreadPool::add_job(std::function<void()>&& job, std::function<void()>&& callback) {
    internal_create_packaged_task(std::move(job), std::move(callback));
}

void ThreadPool::internal_create_packaged_task(std::function<void()>&& job, std::function<void()>&& callback) {
    const auto task = std::make_shared<std::packaged_task<void()>>(std::move(job));
    ThreadPoolFuture thread_pool_future(std::move(task->get_future()), std::move(callback));
    if (size() > 0) {
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            m_Futures.push_back(std::move(thread_pool_future)); //TODO: see if this really needs to be in the critical section 
            m_TaskQueue.push(std::move(task));
        }
        m_ConditionVariable.notify_one();
    }else{
        //on single threaded, we just execute the tasks on the main thread below in update()
        m_Futures.push_back(std::move(thread_pool_future));
        m_TaskQueue.push(std::move(task));
    }
}
void ThreadPool::update() {
    if (size() == 0) { //for single threaded stuff
        while (m_TaskQueue.size() > 0) {
            auto job = (m_TaskQueue.front());
            m_TaskQueue.pop();
            (*job)();
        }
    }
    //this executes the "then" functions
    for (auto it = m_Futures.begin(); it != m_Futures.end();) {
        auto& future = (*it);
        if (future.isReady()) {
            future();
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
    if (m_WorkerThreads.size() > 0) {
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