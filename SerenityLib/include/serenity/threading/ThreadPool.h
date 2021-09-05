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
            WorkerThreadContainer                        m_WorkerThreads;
            std::vector<FutureCallbackType>              m_FuturesCallback;
            mutable std::mutex                           m_Mutex;
            TaskQueueType                                m_TaskQueue;
            std::condition_variable_any                  m_ConditionVariableAny;
            std::atomic<int32_t>                         m_WaitCounter           = 0;
            bool                                         m_Stopped               = true;
      
            void internal_update_single_threaded();
            void internal_execute_callbacks();
            void internal_update_multi_threaded();
        public:
            ThreadPool();
            ~ThreadPool();

            ThreadPool(const ThreadPool&) noexcept            = delete;
            ThreadPool& operator=(const ThreadPool&) noexcept = delete;
            ThreadPool(ThreadPool&&) noexcept                 = delete;
            ThreadPool& operator=(ThreadPool&&) noexcept      = delete;

            //starts the threadpool. the threadpool is stopped by default
            bool startup(int num_threads);

            void shutdown() noexcept;

            [[nodiscard]] inline int size() const noexcept { return static_cast<int>(m_WorkerThreads.size()); }

            template<class JOB> [[nodiscard]] void add_job(JOB&& job) {
                #if !defined(ENGINE_FORCE_NO_THEAD_POOL)
                    if (size() > 0) {
                        {
                            std::lock_guard lock{ m_Mutex };
                            m_TaskQueue.emplace(std::make_shared<PoolTask>(std::forward<JOB>(job)));
                        }
                        m_ConditionVariableAny.notify_one();
                        m_WaitCounter += 1;
                    } else {
                #endif
                        job();
                #if !defined(ENGINE_FORCE_NO_THEAD_POOL)
                    }
                #endif
            }
            template<class JOB, class THEN> [[nodiscard]] Engine::view_ptr<FutureType> add_job(JOB&& job, THEN&& callback) {
                #if !defined(ENGINE_FORCE_NO_THEAD_POOL)
                    if (size() > 0) {
                        FutureType* ret = nullptr;
                        {
                            std::lock_guard lock{ m_Mutex };
                            auto& task = m_TaskQueue.emplace(std::make_shared<PoolTask>(std::forward<JOB>(job)));
                            ret = &m_FuturesCallback.emplace_back(task->get_future(), std::forward<THEN>(callback)).first;
                        }
                        m_ConditionVariableAny.notify_one();
                        m_WaitCounter += 1;
                        return ret;
                    } else {
                #endif
                        job();
                        callback();
                        return nullptr;
                #if !defined(ENGINE_FORCE_NO_THEAD_POOL)
                    }
                #endif
                    return nullptr;
            }

            void update();
            void wait_for_all() noexcept;
    };
};
#endif