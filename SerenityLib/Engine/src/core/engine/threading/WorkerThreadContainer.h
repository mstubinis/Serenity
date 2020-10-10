#pragma once
#ifndef ENGINE_THREADING_WORKER_THREAD_CONTAINER_H
#define ENGINE_THREADING_WORKER_THREAD_CONTAINER_H

namespace Engine::priv {
    class WorkerThreadContainer final {
        private:
            std::vector<std::thread>                            m_WorkerThreads;
            std::unordered_map<std::thread::id, std::thread*>   m_WorkerThreadsHashed;
        public:
            WorkerThreadContainer() = default;
            ~WorkerThreadContainer();

            void clear() noexcept;
            void reserve(size_t newReserveSize) noexcept;

            inline size_t size() const noexcept { return m_WorkerThreads.size(); }

            template<typename FUNCTION>
            std::thread* add_thread(FUNCTION&& function) noexcept {
                if (m_WorkerThreads.size() >= m_WorkerThreads.capacity()) {
                    return nullptr;
                }
                auto& worker = m_WorkerThreads.emplace_back(std::move(function));
                m_WorkerThreadsHashed.emplace(
                    std::piecewise_construct,
                    std::forward_as_tuple(worker.get_id()),
                    std::forward_as_tuple(&worker)
                );
                return &worker;
            }

            inline std::thread* operator[](std::thread::id threadID) noexcept { return m_WorkerThreadsHashed.at(threadID); }

            inline std::vector<std::thread>::iterator begin() { return m_WorkerThreads.begin(); }
            inline std::vector<std::thread>::iterator end() { return m_WorkerThreads.end(); }
            inline std::vector<std::thread>::const_iterator begin() const { return m_WorkerThreads.begin(); }
            inline std::vector<std::thread>::const_iterator end() const { return m_WorkerThreads.end(); }
    };
};

#endif