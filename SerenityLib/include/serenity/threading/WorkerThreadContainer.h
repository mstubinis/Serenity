#pragma once
#ifndef ENGINE_THREADING_WORKER_THREAD_CONTAINER_H
#define ENGINE_THREADING_WORKER_THREAD_CONTAINER_H

#include <unordered_map>
#include <vector>
#include <thread>
#include <serenity/system/Macros.h>
#include <serenity/types/Types.h>
#include <functional>

namespace Engine::priv {
    class WorkerThreadContainer final {
        using ThreadType     = std::jthread;
        using ThreadID       = ThreadType::id;
        using ThreadVector   = std::vector<ThreadType>;
        using ThreadHashMap  = std::unordered_map<ThreadID, ThreadType*>;
        using ThreadFunction = std::function<void(std::stop_token)>;
        private:
            ThreadVector           m_WorkerThreads;
            ThreadHashMap          m_WorkerThreadsHashed;
        public:
            WorkerThreadContainer() = default;

            void clear() noexcept;
            void reserve(size_t newReserveSize) noexcept;

            [[nodiscard]] inline size_t size() const noexcept { return m_WorkerThreads.size(); }
            [[nodiscard]] inline bool contains(ThreadID id) const noexcept { return m_WorkerThreadsHashed.contains(id); }
            [[nodiscard]] inline bool contains(const ThreadType& thread) const noexcept { return m_WorkerThreadsHashed.contains(thread.get_id()); }
            [[nodiscard]] inline ThreadType* operator[](ThreadID threadID) noexcept { return m_WorkerThreadsHashed.at(threadID); }
            [[nodiscard]] inline ThreadType* operator[](const ThreadType& thread) noexcept { return m_WorkerThreadsHashed.at(thread.get_id()); }
            [[nodiscard]] inline const ThreadType* const operator[](ThreadID threadID) const noexcept { return m_WorkerThreadsHashed.at(threadID); }
            [[nodiscard]] inline const ThreadType* const operator[](const ThreadType& thread) const noexcept { return m_WorkerThreadsHashed.at(thread.get_id()); }

            Engine::view_ptr<ThreadType> add_thread(ThreadFunction&& threadFunction, uint32_t index);

            BUILD_BEGIN_END_ITR_CLASS_MEMBERS(ThreadVector, m_WorkerThreads)
    };
};

#endif