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
            ThreadPool m_ThreadPool;

            ThreadingModule();
            ~ThreadingModule();

            void cleanup();

            void update(const float dt);
    };
    namespace threading{
        void waitForAll(unsigned int section = 0);
        void finalizeJob(std::function<void()>&& job, unsigned int section);
        void finalizeJob(std::function<void()>&& job, std::function<void()>&& then, unsigned int section);
         
        template<typename JOB> void addJob(JOB&& job, unsigned int section = 0U){
            std::function<void()> job_func = job;
            finalizeJob(std::move(job_func), section);
        }
        template<typename JOB, typename THEN> void addJobWithPostCallback(JOB&& job, THEN&& then, unsigned int section = 0U){
            std::function<void()> job_func = job;
            std::function<void()> then_func = then;
            finalizeJob(std::move(job_func), std::move(then_func), section);
        }
        template<typename JOB, typename T> void addJobSplitVectored(JOB&& job, std::vector<T>& collection, bool waitForAll, unsigned int section = 0U) {
            if (Engine::hardware_concurrency() > 1) {
                auto pairs = Engine::splitVectorPairs(collection);
                for (size_t k = 0; k < pairs.size(); ++k) {
                    auto lambda = [&pairs, k, &job, &collection]() {
                        for (size_t j = pairs[k].first; j <= pairs[k].second; ++j) {
                            job(collection[j], j, k);
                        }
                    };
                    Engine::priv::threading::addJob(lambda, section);
                }
                if (waitForAll) {
                    Engine::priv::threading::waitForAll(section);
                }
            }else{
                for (size_t j = 0; j < collection.size(); ++j) {
                    job(collection[j], j, 0);
                }
            }
        }
    };
};

#endif