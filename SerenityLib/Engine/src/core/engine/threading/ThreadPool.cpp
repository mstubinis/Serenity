#include <core/engine/threading/ThreadPool.h>
#include <core/engine/threading/WorkerThread.h>
#include <core/engine/utils/Utils.h>
#include <mutex>

using namespace std;
using namespace Engine;
using namespace Engine::epriv;

ThreadPoolFuture::ThreadPoolFuture() {
    ThreadPoolFuture::EmptyCallback emptyCallback;
    m_Callback = std::bind<void>(emptyCallback);
}
ThreadPoolFuture::ThreadPoolFuture(std::shared_future<void>&& future) {
    m_Future = future;
    ThreadPoolFuture::EmptyCallback emptyCallback;
    m_Callback = std::bind<void>(emptyCallback);
}
ThreadPoolFuture::ThreadPoolFuture(std::shared_future<void>&& future, std::function<void()>&& callback) {
    m_Future = future;
    m_Callback = callback;
}
ThreadPoolFuture::~ThreadPoolFuture() {

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




ThreadPool::ThreadPool() {
    unsigned int numberOfThreads = std::max(1U, std::thread::hardware_concurrency());
    init(numberOfThreads);
}
ThreadPool::ThreadPool(const unsigned int num_threads) {
    unsigned int numberOfThreads = std::min(num_threads, std::thread::hardware_concurrency());
    numberOfThreads = std::max(1U, numberOfThreads);
    init(numberOfThreads);
}
ThreadPool::~ThreadPool() {
    if (!m_Stopped)
        shutdown();
    SAFE_DELETE_VECTOR(m_Futures);
    SAFE_DELETE_VECTOR(m_WorkerThreads);
}
void ThreadPool::init(const unsigned int num_threads) {
    m_Terminated = false;
    m_Stopped = false;
    for (unsigned int i = 0; i < num_threads; ++i) {
        m_WorkerThreads.push_back(new WorkerThread(*this));
    }
}
void ThreadPool::addJob(std::function<void()>&& job) {
    auto task = std::make_shared<std::packaged_task<void()>>(job);
    ThreadPoolFuture* future = new ThreadPoolFuture(std::move(std::shared_future<void>(task->get_future())));
    m_Futures.push_back(future);

    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_Tasks.push(std::move(task));
    }
    m_Condition.notify_one();
}
void ThreadPool::addJob(std::function<void()>&& job, std::function<void()>&& callback) {
    auto task = std::make_shared<std::packaged_task<void()>>(job);
    ThreadPoolFuture* future = new ThreadPoolFuture(std::move(std::shared_future<void>(task->get_future())));
    future->m_Callback = std::move(callback);
    m_Futures.push_back(future);

    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        m_Tasks.push(std::move(task));
    }
    m_Condition.notify_one();
}
void ThreadPool::update() {
    if (m_Futures.size() == 0)
        return;
    for (auto it = m_Futures.begin(); it != m_Futures.end();) {
        auto& future = *(*it);
        if (future.m_Future._Is_ready()) {
            future.m_Callback();
            SAFE_DELETE(*it);
            it = m_Futures.erase(it);
        }
        else { ++it; }
    }
}
const unsigned int ThreadPool::numThreads() const {
    return m_WorkerThreads.size();
}
void ThreadPool::join_all() {
    for (auto& thread : m_WorkerThreads) {
        thread->join();
    }
}
void ThreadPool::wait_for_all() {
    if (m_Futures.size() > 0) {
        for (auto& future : m_Futures) {
            future->m_Future.wait();
        }
    }
}
void ThreadPool::shutdown() {
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Terminated = true;
    m_Condition.notify_all();// wake up all threads.
    m_Stopped = true;
}