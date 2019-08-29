#pragma once
#ifndef ENGINE_THREADING_THREAD_POOL_H
#define ENGINE_THREADING_THREAD_POOL_H

#include <queue>
#include <vector>
#include <thread>
#include <future>
#include <functional>
#include <condition_variable>
#include <memory>

namespace Engine {
    namespace epriv {
        class WorkerThread;
        class ThreadPool;
        class ThreadPoolFuture {
            friend class ThreadPool;
            private:
                struct EmptyCallback final { void operator()() const {} };
                std::shared_future<void>    m_Future;
                std::function<void()>       m_Callback;

            public:
                ThreadPoolFuture();
                ThreadPoolFuture(std::shared_future<void>&& future);
                ThreadPoolFuture(std::shared_future<void>&& future, std::function<void()>&& callback);
                ~ThreadPoolFuture();

                ThreadPoolFuture(const ThreadPoolFuture& other) noexcept = delete;
                ThreadPoolFuture& operator=(const ThreadPoolFuture& other) noexcept = delete;
                ThreadPoolFuture(ThreadPoolFuture&& other) noexcept;
                ThreadPoolFuture& operator=(ThreadPoolFuture&& other) noexcept;
        };
        class ThreadPool {
            friend class WorkerThread;
            private:
                std::condition_variable                                   m_Condition;
                std::mutex                                                m_Mutex;
                std::queue<std::shared_ptr<std::packaged_task<void()>>>   m_Tasks;
                std::vector<WorkerThread*>                                m_WorkerThreads;
                std::vector<ThreadPoolFuture*>                            m_Futures;
                bool                                                      m_Terminated;
                bool                                                      m_Stopped;

                void init(const unsigned int num_threads);
            public:
                ThreadPool();
                ThreadPool(const unsigned int num_threads);
                ~ThreadPool();

                const unsigned int numThreads() const;

                void addJob(std::function<void()>&& job);
                void addJob(std::function<void()>&& job, std::function<void()>&& callback);

                void update();

                void join_all();
                void wait_for_all();
                void shutdown();
        };
    };
};
#endif