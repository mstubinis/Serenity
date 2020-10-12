#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/threading/ThreadPool.h>

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
                        std::unique_lock l{ m_Mutex };
                        m_ConditionVariableAny.wait(l, [this] { return m_Stopped || !internal_task_queue_is_empty(); });
                        if (m_Stopped) {
                            return;
                        }
                        job = internal_get_next_available_job();
                    }
                    ASSERT((!m_Stopped && job) || (m_Stopped && !job), __FUNCTION__ << "(): job was nullptr!");
                    //if (job) {
                        job->operator()();
                    //}
                }
            });
        }
        return true;
    }
    return false;
}
Engine::priv::PoolTaskPtr ThreadPool::internal_get_next_available_job() noexcept {
    for (auto& queue : m_TaskQueues) {
        if (!queue.empty()) {
            PoolTaskPtr task = std::move(queue.front());
            queue.pop();
            return task;
        }
    }
    return nullptr;
}
void ThreadPool::internal_create_packaged_task(ThreadJob&& job, size_t section) {
    ASSERT(job, __FUNCTION__ << "(ThreadJob&& job, size_t section): job was not valid!");
    if (size() > 0) {
        {
            std::lock_guard lock{ m_Mutex };
            auto& task = m_TaskQueues[section].emplace(std::make_shared<PoolTask>(std::move(job)));
            m_FuturesBasic[section].emplace_back(task->get_future());
        }
        m_ConditionVariableAny.notify_one();
    }else{
        //on single threaded, we just execute the tasks on the main thread below in update()
        auto& task = m_TaskQueues[section].emplace(std::make_shared<PoolTask>(std::move(job)));
        m_FuturesBasic[section].emplace_back(task->get_future());
    }
}
void ThreadPool::internal_create_packaged_task(ThreadJob&& job, ThreadJob&& callback, size_t section) {
    ASSERT(job,      __FUNCTION__ << "(ThreadJob&& job, ThreadJob&& callback size_t section): job was not valid!");
    ASSERT(callback, __FUNCTION__ << "(ThreadJob&& job, ThreadJob&& callback size_t section): callback was not valid!");
    if (size() > 0) {
        {
            std::lock_guard lock{ m_Mutex };
            auto& task = m_TaskQueues[section].emplace(std::make_shared<PoolTask>(std::move(job)));
            m_FuturesCallback[section].emplace_back(task->get_future(), std::move(callback));
        }
        m_ConditionVariableAny.notify_one();
    }else{
        //on single threaded, we just execute the tasks on the main thread below in update()
        auto& task = m_TaskQueues[section].emplace(std::make_shared<PoolTask>(std::move(job)));
        m_FuturesCallback[section].emplace_back(task->get_future(), std::move(callback));
    }
}
void ThreadPool::update() {
    if (size() == 0) { //for single threaded stuff
        for (auto& queue : m_TaskQueues) {
            while (queue.size() > 0) {
                queue.front()->operator()();
                queue.pop();
            }
        }
    }
    for (size_t section = 0; section < m_FuturesCallback.size(); ++section) {
        std::erase_if(m_FuturesBasic[section], [](const Engine::priv::ThreadPoolFuture& future) {
            return future.isReady();
        });
        std::for_each(m_FuturesCallback[section].cbegin(), m_FuturesCallback[section].cend(), [](const Engine::priv::ThreadPoolFutureCallback& callback) {
            ASSERT(callback == true, __FUNCTION__ << "(): callback had empty function!");
            if (callback.isReady()) {
                callback(); //this executes the "then" functions
            }
        });
        std::erase_if(m_FuturesCallback[section], [](const Engine::priv::ThreadPoolFutureCallback& callback) {
            return callback.isReady();
        });
    }
}

#pragma endregion