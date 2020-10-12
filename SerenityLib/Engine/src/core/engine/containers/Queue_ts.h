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
            queue_ts(const queue_ts& other) {
                std::lock_guard lock(m_Mutex);
                m_Queue = other.m_Queue;
            }
            queue_ts& operator=(const queue_ts& other) = delete;
            queue_ts(queue_ts&& other) noexcept {
                std::lock_guard lock(m_Mutex);
                m_Queue = std::move(other.m_Queue);
            }
            queue_ts& operator=(queue_ts&& other) noexcept = delete;

            inline size_t size() const noexcept {
                std::lock_guard lock(m_Mutex);
                return m_Queue.size();
            }

            inline T& front() noexcept {
                std::lock_guard lock(m_Mutex);
                return m_Queue.front();
            }

            template<typename FUNCTION>
            void for_each_and_clear(FUNCTION&& func) {
                std::unique_lock lock(m_Mutex);
                while (m_Queue.size() > 0) {
                    func(m_Queue.front());
                    m_Queue.pop();
                }
            }

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
                T frontItem(m_Queue.front());
                m_Queue.pop();
                return frontItem;
            }
            T pop() {
                std::lock_guard lock(m_Mutex);
                T frontItem(m_Queue.front());
                m_Queue.pop();
                return frontItem;
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
                T frontItem(m_Queue.front());
                m_Queue.pop();
                return frontItem;
            }
            void clear() {
                {
                    std::lock_guard lock(m_Mutex);
                    while (!m_Queue.empty()) {
                        m_Queue.pop();
                    }
                }
                m_ConditionVariable.notify_all(); //TODO: test this
            }
            void push(T&& item) {
                {
                    std::lock_guard lock(m_Mutex);
                    m_Queue.emplace(std::move(item));
                }
                m_ConditionVariable.notify_one();
            }
            void push(const T& item) {
                {
                    std::lock_guard lock(m_Mutex);
                    m_Queue.push(item);
                }
                m_ConditionVariable.notify_one();
            }
            inline bool empty() const noexcept {
                std::lock_guard lock(m_Mutex);
                return m_Queue.empty();
            }

        };
};

#endif