#pragma once
#ifndef ENGINE_CONTAINERS_QUEUE_THREAD_SAFE_H
#define ENGINE_CONTAINERS_QUEUE_THREAD_SAFE_H

namespace Engine {
    template<typename T> class queue_ts {
        private:
            std::queue<T>                m_Queue;
            mutable std::shared_mutex    m_SharedMutex;
            std::condition_variable      m_ConditionVariable;
        public:
            queue_ts() = default;
            queue_ts(const queue_ts& other) {
                std::unique_lock lock(m_SharedMutex);
                m_Queue = other.m_Queue;
            }
            queue_ts& operator=(const queue_ts& other) = delete;
            queue_ts(queue_ts&& other) noexcept {
                std::lock_guard lock(m_SharedMutex);
                m_Queue = std::move(other.m_Queue);
            }
            queue_ts& operator=(queue_ts&& other) noexcept = delete;

            inline size_t size() const noexcept {
                std::shared_lock lock(m_SharedMutex);
                return m_Queue.size();
            }

            inline T& front() noexcept {
                std::shared_lock lock(m_SharedMutex);
                return m_Queue.front();
            }

            template<typename FUNCTION> void for_each_and_clear(const FUNCTION& func) {
                std::unique_lock lock(m_SharedMutex);
                while (m_Queue.size() > 0) {
                    func(m_Queue.front());
                    m_Queue.pop();
                }
            }
            std::optional<T> try_pop() {
                std::unique_lock lock(m_SharedMutex);
                if (m_Queue.empty()) {
                    return {};
                }
                T frontItem(m_Queue.front());
                m_Queue.pop();
                return frontItem;
            }
            T pop() {
                std::unique_lock lock(m_SharedMutex);
                T frontItem(m_Queue.front());
                m_Queue.pop();
                return frontItem;
            }
            T wait_and_pop() {
                std::unique_lock lock(m_SharedMutex);
                m_ConditionVariable.wait(lock, [this]() {
                    return !m_Queue.empty();
                });
                T frontItem(m_Queue.front());
                m_Queue.pop();
                return frontItem;
            }
            void clear() {
                {
                    std::unique_lock lock(m_SharedMutex);
                    while (!m_Queue.empty()) {
                        m_Queue.pop();
                    }
                }
                m_ConditionVariable.notify_all(); //TODO: test this
            }
            T& push(T&& item) {
                {
                    std::unique_lock lock(m_SharedMutex);
                    m_Queue.push(std::move(item));
                }
                m_ConditionVariable.notify_one();
                return m_Queue.back();
            }
            T& push(const T& item) {
                {
                    std::unique_lock lock(m_SharedMutex);
                    m_Queue.push(item);
                }
                m_ConditionVariable.notify_one();
                return m_Queue.back();
            }

            template<typename ... ARGS> T& emplace(ARGS&&... args) {
                {
                    std::unique_lock lock(m_SharedMutex);
                    m_Queue.emplace(std::forward<ARGS>(args)...);
                }
                m_ConditionVariable.notify_one();
                return m_Queue.back();
            }

            inline bool empty() const noexcept {
                std::shared_lock lock(m_SharedMutex);
                return m_Queue.empty();
            }

        };
};

#endif