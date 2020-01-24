#pragma once
#ifndef ENGINE_THREADING_THREAD_POOL_H
#define ENGINE_THREADING_THREAD_POOL_H

namespace Engine {
    namespace priv {
        class WorkerThread;
        class ThreadPool;
    };
};

#include <queue>
#include <vector>
#include <thread>
#include <future>
#include <functional>
#include <condition_variable>
#include <memory>

namespace Engine {
    namespace priv {
        class ThreadPoolFuture final {
            friend class Engine::priv::ThreadPool;
            private:
                std::future<void>           m_Future;
                std::function<void()>       m_Callback;

                ThreadPoolFuture() = delete;
            public:
                ThreadPoolFuture(std::future<void>&& future, std::function<void()>&& callback);
                ~ThreadPoolFuture() = default;

                ThreadPoolFuture(const ThreadPoolFuture& other) noexcept = delete;
                ThreadPoolFuture& operator=(const ThreadPoolFuture& other) noexcept = delete;
                ThreadPoolFuture(ThreadPoolFuture&& other) noexcept;
                ThreadPoolFuture& operator=(ThreadPoolFuture&& other) noexcept;
        };
        class ThreadPool final{
            friend class Engine::priv::WorkerThread;
            private:
                std::condition_variable                                         m_ConditionVariable;
                std::mutex                                                      m_Mutex;
                std::queue<std::shared_ptr<std::packaged_task<void()>>>         m_TaskQueue;
                std::vector<std::thread>                                        m_WorkerThreads;
                std::vector<Engine::priv::ThreadPoolFuture>                    m_Futures;
                bool                                                            m_Stopped;

                void init(const unsigned int num_threads);

                void internal_create_packaged_task(std::function<void()>&& job, std::function<void()>&& callback);

                ThreadPool() = delete;
            public:
                ThreadPool(const unsigned int num_threads);
                ~ThreadPool();

                ThreadPool(const ThreadPool& other) noexcept = delete;
                ThreadPool& operator=(const ThreadPool& other) noexcept = delete;
                ThreadPool(ThreadPool&& other) noexcept = delete;
                ThreadPool& operator=(ThreadPool&& other) noexcept = delete;

                const size_t size() const;

                void addJob(std::function<void()>&& job);
                void addJob(std::function<void()>&& job, std::function<void()>&& callback);

                void update();

                void join_all();
                void wait_for_all();

                void shutdown();

                //void clear_all_jobs();
        };
    };
};
#endif