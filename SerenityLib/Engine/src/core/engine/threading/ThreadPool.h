#pragma once
#ifndef ENGINE_THREADING_THREAD_POOL_H
#define ENGINE_THREADING_THREAD_POOL_H

namespace Engine::priv {
    class WorkerThread;
    class ThreadPool;
};

namespace Engine::priv {
    class ThreadPoolFuture final {
        friend class Engine::priv::ThreadPool;
        private:
            std::future<void>           m_Future;

            ThreadPoolFuture() = delete;
        public:
            ThreadPoolFuture(std::future<void>&& future);
            ~ThreadPoolFuture() = default;

            bool isReady() const;

            ThreadPoolFuture(const ThreadPoolFuture& other) noexcept = delete;
            ThreadPoolFuture& operator=(const ThreadPoolFuture& other) noexcept = delete;
            ThreadPoolFuture(ThreadPoolFuture&& other) noexcept;
            ThreadPoolFuture& operator=(ThreadPoolFuture&& other) noexcept;
    };
    class ThreadPoolFutureCallback final {
        friend class Engine::priv::ThreadPool;
        private:
            std::future<void>           m_Future;
            std::function<void()>       m_Callback = []() {};

            ThreadPoolFutureCallback() = delete;
        public:
            ThreadPoolFutureCallback(std::future<void>&& future, std::function<void()>&& callback);
            ~ThreadPoolFutureCallback() = default;

            bool isReady() const;
            void operator()() const;

            ThreadPoolFutureCallback(const ThreadPoolFutureCallback& other) noexcept = delete;
            ThreadPoolFutureCallback& operator=(const ThreadPoolFutureCallback& other) noexcept = delete;
            ThreadPoolFutureCallback(ThreadPoolFutureCallback&& other) noexcept;
            ThreadPoolFutureCallback& operator=(ThreadPoolFutureCallback&& other) noexcept;
    };
    
    class ThreadPool final{
        friend class Engine::priv::WorkerThread;
        private:
            std::condition_variable                                                  m_ConditionVariable;
            std::mutex                                                               m_Mutex;
            std::vector<std::queue<std::shared_ptr<std::packaged_task<void()>>>>     m_TaskQueue;
            std::vector<std::thread>                                                 m_WorkerThreads;
            std::vector<std::vector<Engine::priv::ThreadPoolFuture>>                 m_Futures;
            std::vector<std::vector<Engine::priv::ThreadPoolFutureCallback>>         m_FutureCallbacks;
            bool                                                                     m_Stopped = true;

            
            void internal_create_packaged_task(std::function<void()>&& job, unsigned int section);
            void internal_create_packaged_task(std::function<void()>&& job, std::function<void()>&& callback, unsigned int section);
            void internal_update_section(unsigned int section);
            bool task_queue_is_empty() const;
            std::shared_ptr<std::packaged_task<void()>> internal_get_next_available_job();
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