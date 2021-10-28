#pragma once
#ifndef ENGINE_THREADING_WORKER_THREAD_CONTAINER_H
#define ENGINE_THREADING_WORKER_THREAD_CONTAINER_H

#include <unordered_map>
#include <vector>
#include <thread>
#include <serenity/system/Macros.h>
#include <serenity/types/Types.h>

namespace Engine::priv {
    class WorkerThreadContainer final {
        using ThreadType    = std::jthread;
        using ThreadID      = ThreadType::id;
        using ThreadVector  = std::vector<ThreadType>;
        using ThreadHashMap = std::unordered_map<ThreadID, std::jthread*>;
        private:
            ThreadVector           m_WorkerThreads;
            ThreadHashMap          m_WorkerThreadsHashed;
        public:
            WorkerThreadContainer() = default;

            void clear() noexcept;
            void reserve(size_t newReserveSize) noexcept;
            [[nodiscard]] inline bool contains(ThreadID id) const noexcept { return m_WorkerThreadsHashed.contains(id); }
            [[nodiscard]] inline size_t size() const noexcept { return m_WorkerThreads.size(); }

            template<class FUNC> Engine::view_ptr<ThreadType> add_thread(FUNC&& func) noexcept {
                if (m_WorkerThreads.size() >= m_WorkerThreads.capacity()) {
                    ENGINE_PRODUCTION_LOG(__FUNCTION__ << "(): m_WorkerThreads reached its capacity!")
                    return nullptr;
                }
                auto& worker = m_WorkerThreads.emplace_back( std::forward<FUNC>(func) );
                m_WorkerThreadsHashed.emplace( std::piecewise_construct, std::forward_as_tuple(worker.get_id()), std::forward_as_tuple(&worker) );
                return &worker;
            }

            [[nodiscard]] inline ThreadType* operator[](ThreadType::id threadID) noexcept { return m_WorkerThreadsHashed.at(threadID); }
            [[nodiscard]] inline const ThreadType* const operator[](ThreadType::id threadID) const noexcept { return m_WorkerThreadsHashed.at(threadID); }

            BUILD_BEGIN_END_ITR_CLASS_MEMBERS(ThreadVector, m_WorkerThreads)
    };
};

#endif