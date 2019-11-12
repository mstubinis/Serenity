#pragma once
#ifndef ENGINE_THREADING_QUEUE_THREAD_SAFE_H
#define ENGINE_THREADING_QUEUE_THREAD_SAFE_H

#include <queue>
#include <memory>
#include <mutex>
#include <condition_variable>

namespace Engine {
    namespace epriv {
        template<typename T> class Queue_ThreadSafe {
            private:
                mutable std::queue<T>     m_Queue;
                std::mutex                m_Mutex;
                std::condition_variable   m_ConditionVariable;
            public:
                Queue_ThreadSafe() {
                }
                Queue_ThreadSafe(const Queue_ThreadSafe& other) {
                    std::lock_guard<std::mutex> lock(m_Mutex);
                    m_Queue = other.m_Queue;
                }
                Queue_ThreadSafe& operator=(const Queue_ThreadSafe& other) = delete;
                ~Queue_ThreadSafe() {
                }

                const bool try_pop(T& value) {
                    std::lock_guard<std::mutex> lock(m_Mutex);
                    if (m_Queue.empty())
                        return false;
                    value = m_Queue.front();
                    m_Queue.pop();
                    return true;
                }
                std::shared_ptr<T> try_pop() {
                    std::lock_guard<std::mutex> lock(m_Mutex);
                    if (m_Queue.empty())
                        return std::shared_ptr<T>();
                    std::shared_ptr<T> res(std::make_shared<T>(m_Queue.front()));
                    m_Queue.pop();
                    return res;
                }
                void wait_and_pop(T& value) {
                    std::unique_lock<std::mutex> lock(m_Mutex);
                    m_ConditionVariable.wait(lock, [this]() {
                        return !m_Queue.empty();
                    });
                    value = m_Queue.front();
                    m_Queue.pop();
                }
                std::shared_ptr<T> wait_and_pop() {
                    std::unique_lock<std::mutex> lock(m_Mutex);
                    m_ConditionVariable.wait(lock, [this]() {
                        return !m_Queue.empty();
                    });
                    std::shared_ptr<T> res(std::make_shared<T>(m_Queue.front()));
                    m_Queue.pop();
                    return res;
                }
                void push(T pushedValue){
                    std::lock_guard<std::mutex> lock(m_Mutex);
                    m_Queue.push(std::move(pushedValue));
                    m_ConditionVariable.notify_one();
                }
                const bool empty() const {
                    std::lock_guard<std::mutex> lock(m_Mutex);
                    return m_Queue.empty();
                }

        };
    };
};

#endif