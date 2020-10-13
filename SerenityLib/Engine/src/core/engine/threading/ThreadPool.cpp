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
                    job->operator()();
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
    }
    //this CANNOT be split up in different loops / steps.
    for (auto& callbackSection : m_FuturesCallback) {
        for (auto it = callbackSection.begin(); it != callbackSection.end();) {
            if ((*it).isReady()) {
                (*it)();
                it = callbackSection.erase(it);
            }else{
                ++it;
            }

        }
    }
    for (auto& basicSection : m_FuturesBasic) {
        std::erase_if(basicSection, [](const Engine::priv::ThreadPoolFuture& future) {
            return future.isReady();
        });
    }
}

#pragma endregion