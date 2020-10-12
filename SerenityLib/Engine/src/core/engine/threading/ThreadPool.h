#pragma once
#ifndef ENGINE_THREADING_THREAD_POOL_H
#define ENGINE_THREADING_THREAD_POOL_H

namespace Engine::priv {
    class WorkerThread;
    class ThreadPoolFuture;
    class ThreadPoolFutureCallback;
    class ThreadPoolFutureContainer;
    class ThreadPool;
};

#include <core/engine/threading/WorkerThreadContainer.h>
#include <core/engine/threading/Task.h>

using ThreadJob = std::function<void()>;

namespace Engine::priv {
    using PoolTask    = Engine::priv::Task;
    using PoolTaskPtr = std::shared_ptr<PoolTask>;
    using FutureType = std::future<void>;
    class ThreadPoolFuture final {
        friend class Engine::priv::ThreadPool;
        friend class Engine::priv::ThreadPoolFutureContainer;
        private:
            FutureType m_Future;

            ThreadPoolFuture() = delete;
        public:
            ThreadPoolFuture(FutureType&& future);

            ThreadPoolFuture(const ThreadPoolFuture&) noexcept            = delete;
            ThreadPoolFuture& operator=(const ThreadPoolFuture&) noexcept = delete;
            ThreadPoolFuture(ThreadPoolFuture&&) noexcept                 = default;
            ThreadPoolFuture& operator=(ThreadPoolFuture&&) noexcept      = default;

            inline bool isReady() const noexcept { return (m_Future.valid() && m_Future.wait_for(0s) == std::future_status::ready); }
            //inline bool isReady() const noexcept { return (m_Future._Is_ready() && m_Future.valid()); }
    };
    class ThreadPoolFutureCallback final {
        friend class Engine::priv::ThreadPool;
        friend class Engine::priv::ThreadPoolFutureContainer;
        private:
            FutureType  m_Future;
            ThreadJob   m_Callback;

            ThreadPoolFutureCallback() = delete;
        public:
            ThreadPoolFutureCallback(FutureType&& future, ThreadJob&& callback);

            ThreadPoolFutureCallback(const ThreadPoolFutureCallback&) noexcept            = delete;
            ThreadPoolFutureCallback& operator=(const ThreadPoolFutureCallback&) noexcept = delete;
            ThreadPoolFutureCallback(ThreadPoolFutureCallback&&) noexcept                 = default;
            ThreadPoolFutureCallback& operator=(ThreadPoolFutureCallback&&) noexcept      = default;

            inline bool isReady() const noexcept { return (m_Future.valid() && m_Future.wait_for(0s) == std::future_status::ready); }
            //inline bool isReady() const noexcept { return (m_Future._Is_ready() && m_Future.valid()); }

            inline operator bool() const noexcept { return (bool)m_Callback; }
            inline bool operator==(bool other) const noexcept { return (other && m_Callback); }

            inline void operator()() const noexcept {
                ASSERT(m_Callback, __FUNCTION__ << "(): m_Callback was invalid!");
                if(m_Callback)
                    m_Callback();
            }
    };
    class ThreadPool final{
        friend class Engine::priv::WorkerThread;
        private:
            WorkerThreadContainer                                             m_WorkerThreads;
            std::vector<std::vector<Engine::priv::ThreadPoolFuture>>          m_FuturesBasic;
            std::vector<std::vector<Engine::priv::ThreadPoolFutureCallback>>  m_FuturesCallback;
            std::mutex                                                        m_Mutex;
            std::vector<std::queue<PoolTaskPtr>>                              m_TaskQueues;
            std::condition_variable_any                                       m_ConditionVariableAny;
            bool                                                              m_Stopped = true;
      
            void internal_create_packaged_task(ThreadJob&& job, size_t section);
            void internal_create_packaged_task(ThreadJob&& job, ThreadJob&& callback, size_t section);

            bool internal_task_queue_is_empty() const noexcept {
                for (const auto& queue : m_TaskQueues) {
                    if (!queue.empty()) {
                        return false;
                    }
                }
                return true;
            }
            PoolTaskPtr internal_get_next_available_job() noexcept;
        public:
            ThreadPool(size_t sections = 2U);
            ~ThreadPool();

            bool startup(size_t num_threads);

            ThreadPool(const ThreadPool&) noexcept            = delete;
            ThreadPool& operator=(const ThreadPool&) noexcept = delete;
            ThreadPool(ThreadPool&&) noexcept                 = delete;
            ThreadPool& operator=(ThreadPool&&) noexcept      = delete;

            void shutdown() noexcept {
                if (!m_Stopped) {
                    m_Stopped = true;
                    m_ConditionVariableAny.notify_all();// wake up all threads.
                    m_WorkerThreads.clear();
                }
            }

            size_t get_number_of_tasks_in_queue() const noexcept {
                size_t count = 0;
                for (const auto& q : m_TaskQueues) {
                    count += q.size();
                }
                return count;
            }
            inline size_t size() const noexcept { return m_WorkerThreads.size(); }

            template<class TASK> inline void add_job(TASK&& job, size_t section) {
                internal_create_packaged_task(std::move(job), section);
            }
            template<class TASK, class THEN> inline void add_job(TASK&& job, THEN&& callback, size_t section) {
                internal_create_packaged_task(std::move(job), std::move(callback), section);
            }

            void update();

            void join_all() noexcept {
                for (auto& worker_thread : m_WorkerThreads) {
                    if (worker_thread.joinable()) {
                        worker_thread.join();
                    }
                }
            }
            void wait_for_all(size_t section) noexcept {
                if (size() > 0) {
                    //TODO: use std::experimental::when_all when it becomes available, as it should be faster than individual wait's
                    std::for_each(m_FuturesBasic[section].cbegin(), m_FuturesBasic[section].cend(), [](const Engine::priv::ThreadPoolFuture& future) {
                        if (!future.isReady()) {
                            future.m_Future.wait();
                        }
                    });
                    std::for_each(m_FuturesCallback[section].cbegin(), m_FuturesCallback[section].cend(), [](const Engine::priv::ThreadPoolFutureCallback& callback) {
                        if (!callback.isReady()) {
                            callback.m_Future.wait();
                        }
                    });
                }
                update();
            }
    };
};
#endif