#pragma once
#ifndef ENGINE_CONTAINERS_QUEUE_THREAD_SAFE_H
#define ENGINE_CONTAINERS_QUEUE_THREAD_SAFE_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

namespace Engine{
    template<class T, class CONTAINER = std::deque<T>> 
    class queue_ts {
        private:
            std::queue<T, CONTAINER>  m_Queue;
            mutable std::mutex        m_Mutex;
            std::condition_variable   m_ConditionVariable;
        public:
            queue_ts() = default;
            queue_ts(const queue_ts& other) {
                std::scoped_lock lock{ m_Mutex, other.m_Mutex };
                m_Queue = other.m_Queue;
            }
            queue_ts& operator=(const queue_ts& other) {
                if (this != &other) {
                    std::scoped_lock lock{ m_Mutex, other.m_Mutex };
                    m_Queue = other.m_Queue;
                }
                return *this;
            }
            queue_ts(queue_ts&& other) noexcept {
                std::scoped_lock lock{ m_Mutex, other.m_Mutex };
                m_Queue = std::move(other.m_Queue);
            }
            queue_ts& operator=(queue_ts&& other) noexcept {
                if (this != &other) {
                    std::scoped_lock lock{ m_Mutex, other.m_Mutex };
                    m_Queue = std::move(other.m_Queue);
                }
                return *this;
            }

            size_t size() const noexcept {
                std::scoped_lock lock{ m_Mutex };
                return m_Queue.size();
            }
            T& front() noexcept {
                std::scoped_lock lock{ m_Mutex };
                return m_Queue.front();
            }
            template<class FUNC> 
            void for_each_and_clear(FUNC&& func) {
                std::scoped_lock lock{ m_Mutex };
                while (m_Queue.size() > 0) {
                    func(m_Queue.front());
                    m_Queue.pop();
                }
            }
            std::optional<T> try_pop() {
                std::scoped_lock lock{ m_Mutex };
                if (!m_Queue.empty()) {
                    T frontItem{ m_Queue.front() };
                    m_Queue.pop();
                    return frontItem;
                }
                return {};
            }
            T pop() {
                std::scoped_lock lock{ m_Mutex };
                T frontItem{ m_Queue.front() };
                m_Queue.pop();
                return frontItem;
            }
            T wait_and_pop() {
                std::unique_lock lock{ m_Mutex };
                m_ConditionVariable.wait(lock, [this]() {
                    return !m_Queue.empty();
                });
                T frontItem{ m_Queue.front() };
                m_Queue.pop();
                return frontItem;
            }
            void clear() {
                {
                    std::scoped_lock lock{ m_Mutex };
                    while (!m_Queue.empty()) {
                        m_Queue.pop();
                    }
                }
                m_ConditionVariable.notify_all(); //TODO: test this
            }
            T& push(T&& item) {
                std::scoped_lock lock{ m_Mutex };
                m_Queue.push(std::move(item));
                m_ConditionVariable.notify_one();
                return m_Queue.back();
            }
            T& push(const T& item) {
                std::scoped_lock lock{ m_Mutex };
                m_Queue.push(item);
                m_ConditionVariable.notify_one();
                return m_Queue.back();
            }
            template<typename ... ARGS> T& emplace(ARGS&&... args) {
                std::scoped_lock lock{ m_Mutex };
                m_Queue.emplace(std::forward<ARGS>(args)...);
                m_ConditionVariable.notify_one();
                return m_Queue.back();
            }
            bool empty() const noexcept {
                std::scoped_lock lock{ m_Mutex };
                return m_Queue.empty();
            }
        };
};

#endif