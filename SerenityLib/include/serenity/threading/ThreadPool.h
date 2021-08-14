#pragma once
#ifndef ENGINE_THREADING_THREAD_POOL_H
#define ENGINE_THREADING_THREAD_POOL_H

namespace Engine::priv {
    class WorkerThread;
    class ThreadPoolFutureContainer;
    class ThreadPool;
};

#include <serenity/threading/WorkerThreadContainer.h>
#include <future>
#include <functional>
#include <queue>
#include <mutex>

using ThreadJob = std::function<void()>;

namespace Engine::priv {
    using PoolTask           = std::packaged_task<void()>;
    using PoolTaskPtr        = std::shared_ptr<PoolTask>;
    using FutureType         = std::future<void>;
    using FutureCallbackType = std::pair<FutureType, ThreadJob>;
    using TaskQueueType      = std::queue<PoolTaskPtr>;

    class ThreadPool final{
        friend class Engine::priv::WorkerThread;
        private:
            WorkerThreadContainer                               m_WorkerThreads;
            std::vector<std::vector<FutureCallbackType>>        m_FuturesCallback;
            mutable std::mutex                                  m_Mutex;
            std::vector<TaskQueueType>                          m_TaskQueues;
            std::condition_variable_any                         m_ConditionVariableAny;
            std::unique_ptr<std::vector<std::atomic<int32_t>>>  m_WaitCounters;
            bool                                                m_Stopped               = true;
      
            void internal_update_single_threaded();
            void internal_update_multi_threaded();
            bool internal_task_queue_is_empty() const noexcept;
            [[nodiscard]] std::pair<PoolTaskPtr, int> internal_get_next_available_job() noexcept; //first = task, second = section
        public:
            ThreadPool(int sections = 2);
            ~ThreadPool();

            ThreadPool(const ThreadPool&) noexcept            = delete;
            ThreadPool& operator=(const ThreadPool&) noexcept = delete;
            ThreadPool(ThreadPool&&) noexcept                 = delete;
            ThreadPool& operator=(ThreadPool&&) noexcept      = delete;

            //starts the threadpool. the threadpool is stopped by default
            bool startup(int num_threads);

            void shutdown() noexcept;

            [[nodiscard]] size_t get_number_of_tasks_in_queue() const noexcept;
            [[nodiscard]] inline int size() const noexcept { return static_cast<int>(m_WorkerThreads.size()); }

            template<class JOB> [[nodiscard]] void add_job(JOB&& job, int section = 0) {
                if (size() > 0) {
                    (*m_WaitCounters)[section] += 1;
                    {
                        std::lock_guard lock{ m_Mutex };
                        m_TaskQueues[section].emplace(std::make_shared<PoolTask>(std::forward<JOB>(job)));
                    }
                    m_ConditionVariableAny.notify_one();
                } else {
                    //on single threaded, we just execute the tasks on the main thread below in update()
                    m_TaskQueues[section].emplace(std::make_shared<PoolTask>(std::forward<JOB>(job)));
                }
            }
            template<class JOB, class THEN> [[nodiscard]] FutureType& add_job(JOB&& job, THEN&& callback, int section = 0) {
                if (size() > 0) {
                    (*m_WaitCounters)[section] += 1;
                    FutureType* ret;
                    {
                        std::lock_guard lock{ m_Mutex };
                        auto& task = m_TaskQueues[section].emplace(std::make_shared<PoolTask>(std::forward<JOB>(job)));
                        ret = &m_FuturesCallback[section].emplace_back(task->get_future(), std::forward<THEN>(callback)).first;
                    }
                    m_ConditionVariableAny.notify_one();
                    return *ret;
                } else {
                    //on single threaded, we just execute the tasks on the main thread below in update()
                    auto& task = m_TaskQueues[section].emplace(std::make_shared<PoolTask>(std::forward<JOB>(job)));
                    return m_FuturesCallback[section].emplace_back(task->get_future(), std::forward<THEN>(callback)).first;
                }
            }

            void update();
            void wait_for_all(int section) noexcept;
    };
};
#endif