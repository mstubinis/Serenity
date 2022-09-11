#pragma once
#ifndef ENGINE_THREADING_MODULE_H
#define ENGINE_THREADING_MODULE_H

namespace Engine::priv {
    class  ThreadPool;
};

#include <serenity/threading/ThreadingHelpers.h>
#include <serenity/threading/ThreadPool.h>

namespace Engine::priv {
    class ThreadingModule final {
        public:
            static Engine::view_ptr<Engine::priv::ThreadingModule> THREADING_MODULE;
        public:
            ThreadPool m_ThreadPool;

            ThreadingModule();
            ~ThreadingModule();

            void cleanup();
            void update(const float dt);
    };
    namespace threading {
        bool isOpenGLThread() noexcept;
        bool isMainThread() noexcept;

        void setThreadName(std::thread&, std::string_view name);
        void setThreadName(std::jthread&, std::string_view name);

        [[nodiscard]] inline std::optional<std::stop_token> getThreadStopToken() noexcept { return ThreadingModule::THREADING_MODULE->m_ThreadPool.getThreadStopToken(); }
        [[nodiscard]] inline bool isWorkerThreadStopped() noexcept { return ThreadingModule::THREADING_MODULE->m_ThreadPool.isWorkerThreadStopped(); }

        void waitForAll() noexcept;

        template<class JOB> inline void addJob(JOB&& job) {
            ThreadingModule::THREADING_MODULE->m_ThreadPool.add_job(std::forward<JOB>(job));
        }
        template<class JOB, class THEN> inline Engine::view_ptr<FutureType> addJobWithPostCallback(JOB&& job, THEN&& then){
            return ThreadingModule::THREADING_MODULE->m_ThreadPool.add_job(std::forward<JOB>(job), std::forward<THEN>(then));
        }

        template<class JOB, class ... ARGS> void addJobSplitVectored(JOB&& job, bool waitForAll, bool includeCallingThread, size_t size, int32_t numJobs = 0, ARGS&&... args) {
            if (Engine::hardware_concurrency() > 1 && size > 0) {
                if (size < Engine::hardware_concurrency()) {
                    for (size_t index = 0; index != size; ++index) {
                        job(index, 0, std::forward<ARGS>(args)...);
                    }
                } else {
                    size_t accumulator = 0;
                    if (numJobs <= 0) {
                        numJobs = Engine::hardware_concurrency();
                    }
                    const size_t jobSize = size / numJobs;
                    for (int32_t jobIndex = 0; jobIndex != numJobs - 1; ++jobIndex) {
                        //Engine::priv::threading::addJob([jobSize, accumulator, jobIndex, job_ = std::forward<JOB>(job), ... args_ = std::forward<ARGS>(args)]() mutable {
                        Engine::priv::threading::addJob([jobSize, accumulator, jobIndex, job_ = job, ... args_ = args]() mutable {
                            const auto end = accumulator + jobSize;
                            for (size_t index = accumulator; index != end; ++index) {
                                job_(index, jobIndex, args_...);
                            }
                        });
                        accumulator += jobSize;
                    }
                    auto lastJob = [accumulator, numJobs, size, job_ = std::forward<JOB>(job), ... args_ = std::forward<ARGS>(args)]() mutable {
                        for (size_t index = accumulator; index != size; ++index) {
                            job_(index, numJobs - 1, args_...);
                        }
                    };
                    if (includeCallingThread) {
                        lastJob();
                    } else {
                        Engine::priv::threading::addJob(std::move(lastJob));
                    }
                    if (waitForAll) {
                        Engine::priv::threading::waitForAll();
                    }
                }
            } else {
                for (size_t index = 0; index != size; ++index) {
                    job(index, 0, std::forward<ARGS>(args)...);
                }
            }
        }
    };
};

#endif