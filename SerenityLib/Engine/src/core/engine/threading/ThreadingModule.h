#pragma once
#ifndef ENGINE_THREADING_MODULE_H
#define ENGINE_THREADING_MODULE_H

namespace Engine::priv {
    class  ThreadPool;
    class  ParticleSystem;
    struct ComponentCamera_UpdateFunction;
    struct ComponentBody_UpdateFunction;
    struct ComponentModel_UpdateFunction;
};

#include <core/engine/threading/ThreadingHelpers.h>
#include <core/engine/threading/ThreadPool.h>

namespace Engine::priv{
    class ThreadingModule final {
        friend struct ComponentCamera_UpdateFunction;
        friend struct ComponentBody_UpdateFunction;
        friend struct ComponentModel_UpdateFunction;
        friend class  Engine::priv::ParticleSystem;

        public:
            static Engine::view_ptr<Engine::priv::ThreadingModule> THREADING_MODULE;
        public:
            ThreadPool m_ThreadPool;

            ThreadingModule();
            ~ThreadingModule();

            void cleanup();

            void update(const float dt);
    };
    namespace threading{
        bool isMainThread() noexcept;

        void waitForAll(size_t section = 0);
        void finalizeJob(std::function<void()>&& job, size_t section);
        void finalizeJob(std::function<void()>&& job, std::function<void()>&& then, size_t section);

        template<typename JOB> void addJob(JOB&& job, size_t section = 0U){
            finalizeJob(std::function<void()>{std::move(job)}, section);
        }
        template<typename JOB, typename THEN> void addJobWithPostCallback(JOB&& job, THEN&& then, size_t section = 0U){
            finalizeJob(std::function<void()>{std::move(job)}, std::function<void()>{std::move(then)}, section);
        }
        template<typename JOB, typename T> void addJobSplitVectored(JOB&& job, std::vector<T>& collection, bool waitForAll, size_t section = 0U) {
            if (Engine::hardware_concurrency() > 1) {
                auto pairs = Engine::splitVectorPairs(collection);
                for (size_t k = 0U; k < pairs.size(); ++k) {
                    Engine::priv::threading::addJob([&pairs, k, &job, &collection]() {
                        for (size_t j = pairs[k].first; j <= pairs[k].second; ++j) {
                            job(collection[j], j, k);
                        }  
                    }, section);
                }
                if (waitForAll) {
                    Engine::priv::threading::waitForAll(section);
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