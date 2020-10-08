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

namespace Engine::priv {
    using PoolTask    = Engine::priv::Task;
    using PoolTaskPtr = std::shared_ptr<PoolTask>;

    class ThreadPoolFuture final {
        friend class Engine::priv::ThreadPool;
        friend class Engine::priv::ThreadPoolFutureContainer;
        private:
            std::future<void>           m_Future;

            ThreadPoolFuture() = delete;
        public:
            ThreadPoolFuture(std::future<void>&& future);
            ~ThreadPoolFuture() = default;

            inline bool isReady() const noexcept { return (m_Future._Is_ready() && m_Future.valid()); }

            ThreadPoolFuture(const ThreadPoolFuture& other) noexcept            = delete;
            ThreadPoolFuture& operator=(const ThreadPoolFuture& other) noexcept = delete;
            ThreadPoolFuture(ThreadPoolFuture&& other) noexcept                 = default;
            ThreadPoolFuture& operator=(ThreadPoolFuture&& other) noexcept      = default;
    };
    class ThreadPoolFutureCallback final {
        friend class Engine::priv::ThreadPool;
        friend class Engine::priv::ThreadPoolFutureContainer;
        private:
            std::future<void>           m_Future;
            std::function<void()>       m_Callback = []() {};

            ThreadPoolFutureCallback() = delete;
        public:
            ThreadPoolFutureCallback(std::future<void>&& future, std::function<void()>&& callback);
            ~ThreadPoolFutureCallback() = default;

            inline bool isReady() const noexcept { return (m_Future._Is_ready() && m_Future.valid()); }
            inline void operator()() const noexcept { if(m_Callback) m_Callback(); }

            ThreadPoolFutureCallback(const ThreadPoolFutureCallback& other) noexcept            = delete;
            ThreadPoolFutureCallback& operator=(const ThreadPoolFutureCallback& other) noexcept = delete;
            ThreadPoolFutureCallback(ThreadPoolFutureCallback&& other) noexcept                 = default;
            ThreadPoolFutureCallback& operator=(ThreadPoolFutureCallback&& other) noexcept      = default;
    };
    
    class ThreadPoolFutureContainer final {
        friend class ThreadPool;
        private:
            std::vector<std::vector<Engine::priv::ThreadPoolFuture>>          m_Sections;
            std::vector<std::vector<Engine::priv::ThreadPoolFutureCallback>>  m_Callbacks;
        public:
            void resize(size_t newSize) {
                m_Sections.resize(newSize);
                m_Callbacks.resize(newSize);
            }
            inline size_t size() const noexcept { return m_Sections.size(); }
            inline void emplace(ThreadPoolFuture&& future, unsigned int section) {
                m_Sections[section].emplace_back(std::move(future));
            }
            inline void emplace(ThreadPoolFutureCallback&& future, unsigned int section) {
                m_Callbacks[section].emplace_back(std::move(future));
            }
            void update_section(unsigned int section) noexcept;
            void wait_for_all(unsigned int section) noexcept;

            void reserve(size_t newSize) {
                for (auto& futureSection : m_Sections) {
                    futureSection.reserve(newSize);
                }
                for (auto& callbackSection : m_Callbacks) {
                    callbackSection.reserve(newSize);
                }
            }
    };

    class ThreadPool final{
        friend class Engine::priv::WorkerThread;
        private:
            WorkerThreadContainer                                             m_WorkerThreads;
            ThreadPoolFutureContainer                                         m_Futures;
            std::mutex                                                        m_Mutex;
            std::vector<std::queue<PoolTaskPtr>>                              m_TaskQueue;
            std::condition_variable_any                                       m_ConditionVariableAny;
            bool                                                              m_Stopped = true;
      
            void internal_create_packaged_task(std::function<void()>&& job, unsigned int section);
            void internal_create_packaged_task(std::function<void()>&& job, std::function<void()>&& callback, unsigned int section);
            bool task_queue_is_empty() const;
            PoolTaskPtr internal_get_next_available_job();
            void internal_emplace(ThreadPoolFuture&&, Engine::priv::PoolTaskPtr&&, unsigned int section) noexcept;
            void internal_emplace(ThreadPoolFutureCallback&&, Engine::priv::PoolTaskPtr&&, unsigned int section) noexcept;
        public:
            ThreadPool(unsigned int sections = 2U);
            ~ThreadPool();

            bool startup(unsigned int num_threads);

            ThreadPool(const ThreadPool& other) noexcept = delete;
            ThreadPool& operator=(const ThreadPool& other) noexcept = delete;
            ThreadPool(ThreadPool&& other) noexcept = delete;
            ThreadPool& operator=(ThreadPool&& other) noexcept = delete;

            size_t size() const;

            void add_job(std::function<void()>&& job, unsigned int section);
            void add_job(std::function<void()>&& job, std::function<void()>&& callback, unsigned int section);

            void update();

            void join_all();
            void wait_for_all(unsigned int section);

            void shutdown();
    };
};
#endif