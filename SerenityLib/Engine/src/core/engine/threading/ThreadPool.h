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

            inline bool isReady() const noexcept { return (m_Future.wait_for(0s) == std::future_status::ready /*&& m_Future.valid()*/); }
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

            inline bool isReady() const noexcept { return (m_Future.wait_for(0s) == std::future_status::ready /*&& m_Future.valid()*/); }

            inline operator bool() const noexcept { return (bool)m_Callback; }
            inline bool operator==(bool other) const noexcept { return (other && m_Callback); }

            inline void operator()() const noexcept {
                ASSERT(m_Callback, __FUNCTION__ << "(): m_Callback was invalid!");
                //if(m_Callback) 
                m_Callback();
            }
    };
    
    class ThreadPoolFutureContainer final {
        friend class Engine::priv::ThreadPool;
        private:
            std::vector<std::vector<Engine::priv::ThreadPoolFuture>>          m_FuturesBasic;
            std::vector<std::vector<Engine::priv::ThreadPoolFutureCallback>>  m_FuturesCallback;

        public:
            void resize(size_t newSize) {
                m_FuturesBasic.resize(newSize);
                m_FuturesCallback.resize(newSize);
            }
            void reserve(size_t newSize) {
                for (auto& section : m_FuturesBasic) { section.reserve(newSize); }
                for (auto& section : m_FuturesCallback) { section.reserve(newSize); }
            }
            inline size_t size() const noexcept { return std::min(m_FuturesBasic.size(), m_FuturesCallback.size()); }
            inline void emplace(ThreadPoolFuture&& future, size_t section) { 
                m_FuturesBasic[section].emplace_back(std::move(future)); 
            }
            inline void emplace(ThreadPoolFutureCallback&& future, size_t section) { 
                m_FuturesCallback[section].emplace_back(std::move(future)); 
            }
            void update_section(size_t section) noexcept;
            void wait_for_all(size_t section) noexcept;
    };

    class ThreadPool final{
        friend class Engine::priv::WorkerThread;
        private:
            WorkerThreadContainer                     m_WorkerThreads;
            ThreadPoolFutureContainer                 m_Futures;
            std::mutex                                m_Mutex;
            std::vector<std::queue<PoolTaskPtr>>      m_TaskQueues;
            std::condition_variable_any               m_ConditionVariableAny;
            bool                                      m_Stopped = true;
      
            void internal_create_packaged_task(ThreadJob&& job, size_t section);
            void internal_create_packaged_task(ThreadJob&& job, ThreadJob&& callback, size_t section);

            bool task_queue_is_empty() const;
            PoolTaskPtr internal_get_next_available_job();
            void internal_emplace(ThreadPoolFuture&&, PoolTaskPtr&&, size_t section) noexcept;
            void internal_emplace(ThreadPoolFutureCallback&&, PoolTaskPtr&&, size_t section) noexcept;
        public:
            ThreadPool(size_t sections = 2U);
            ~ThreadPool();

            bool startup(size_t num_threads);

            ThreadPool(const ThreadPool&) noexcept            = delete;
            ThreadPool& operator=(const ThreadPool&) noexcept = delete;
            ThreadPool(ThreadPool&&) noexcept                 = delete;
            ThreadPool& operator=(ThreadPool&&) noexcept      = delete;

            inline size_t size() const noexcept { return m_WorkerThreads.size(); }

            void add_job(ThreadJob&& job, size_t section);
            void add_job(ThreadJob&& job, ThreadJob&& callback, size_t section);

            void update();

            void join_all();
            void wait_for_all(size_t section);

            void shutdown();
    };
};
#endif