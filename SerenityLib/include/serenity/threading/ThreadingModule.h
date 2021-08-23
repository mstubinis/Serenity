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

        void submitTaskForMainThread(std::function<void()>&& task) noexcept;

        void waitForAll() noexcept;

        template<class JOB> inline void finalizeJob(JOB&& task) {
            ThreadingModule::THREADING_MODULE->m_ThreadPool.add_job(std::forward<JOB>(task));
        }
        template<class JOB, class THEN> inline Engine::view_ptr<FutureType> finalizeJob(JOB&& task, THEN&& then) {
            return ThreadingModule::THREADING_MODULE->m_ThreadPool.add_job(std::forward<JOB>(task), std::forward<THEN>(then));
        }
        template<class JOB> inline void addJob(JOB&& job) {
            finalizeJob(std::forward<JOB>(job));
        }
        template<class JOB, class THEN> inline Engine::view_ptr<FutureType> addJobWithPostCallback(JOB&& job, THEN&& then){
            return finalizeJob(std::forward<JOB>(job), std::forward<THEN>(then));
        }
        template<class JOB, class T> void addJobSplitVectored(JOB&& job, std::vector<T>& collection, bool waitForAll) {
            if (Engine::hardware_concurrency() > 1) {
                auto pairs = Engine::splitVectorPairs(collection);
                for (size_t k = 0U; k < pairs.size(); ++k) {
                    Engine::priv::threading::addJob([&pairs, k, &job, &collection]() {
                        for (size_t j = pairs[k].first; j <= pairs[k].second; ++j) {
                            job(collection[j], j, k);
                        }  
                    });
                }
                if (waitForAll) {
                    Engine::priv::threading::waitForAll();
                }
            }else{
                for (size_t j = 0; j < collection.size(); ++j) {
                    job(collection[j], j, 0U);
                }
            }
        }
    };
};

#endif