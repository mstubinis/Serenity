#pragma once
#ifndef ENGINE_CONTAINERS_QUEUE_THREAD_SAFE_H
#define ENGINE_CONTAINERS_QUEUE_THREAD_SAFE_H

namespace Engine {
    template<typename T> class queue_ts {
        private:
            std::queue<T>             m_Queue;
            mutable std::mutex        m_Mutex;
            std::condition_variable   m_ConditionVariable;
        public:
            queue_ts() = default;
            ~queue_ts() = default;

            queue_ts(const queue_ts& other) {
                std::lock_guard lock(m_Mutex);
                m_Queue = other.m_Queue;
            }
            queue_ts& operator=(const queue_ts& other) = delete;
            /*
            bool try_pop(T& value) {
                std::lock_guard lock(m_Mutex);
                if (m_Queue.empty()) {
                    return false;
                }
                value = m_Queue.front();
                m_Queue.pop();
                return true;
            }
            */
            std::optional<T> try_pop() {
                std::lock_guard lock(m_Mutex);
                if (m_Queue.empty()) {
                    return {};
                }
                T res(m_Queue.front());
                m_Queue.pop();
                return std::move(res);
            }
            T pop() {
                std::lock_guard lock(m_Mutex);
                T res(m_Queue.front());
                m_Queue.pop();
                return std::move(res);
            }
            /*
            void wait_and_pop(T& value) {
                std::unique_lock lock(m_Mutex);
                m_ConditionVariable.wait(lock, [this]() {
                    return !m_Queue.empty();
                });
                value = m_Queue.front();
                m_Queue.pop();
            }
            */
            T wait_and_pop() {
                std::unique_lock lock(m_Mutex);
                m_ConditionVariable.wait(lock, [this]() {
                    return !m_Queue.empty();
                });
                T res(m_Queue.front());
                m_Queue.pop();
                return std::move(res);
            }
            void push(T&& u) {
                {
                    std::lock_guard lock(m_Mutex);
                    m_Queue.emplace(std::forward<decltype(u)>(u));
                }
                m_ConditionVariable.notify_one();
            }
            void push(const T& u) {
                {
                    std::lock_guard lock(m_Mutex);
                    m_Queue.push(u);
                }
                m_ConditionVariable.notify_one();
            }
            bool empty() const {
                std::lock_guard lock(m_Mutex);
                return m_Queue.empty();
            }

        };
};

#endif