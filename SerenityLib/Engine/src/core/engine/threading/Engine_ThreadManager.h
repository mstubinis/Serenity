#pragma once
#ifndef ENGINE_THREAD_MANAGER_H
#define ENGINE_THREAD_MANAGER_H

#ifndef BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
#define BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
#endif
#ifndef BOOST_RESULT_OF_USE_DECLTYPE
#define BOOST_RESULT_OF_USE_DECLTYPE
#endif

namespace Engine::priv {
    class  ThreadPool;
    class  ParticleSystem;
    struct ComponentCamera_UpdateFunction;
    struct ComponentBody_UpdateFunction;
    struct ComponentModel_UpdateFunction;
};

#include <memory>
#include <functional>
#include <utility>
#include <thread>
#include <core/engine/threading/ThreadPool.h>

namespace Engine::priv{
    class ThreadManager final{
        friend struct ComponentCamera_UpdateFunction;
        friend struct ComponentBody_UpdateFunction;
        friend struct ComponentModel_UpdateFunction;
        friend class  Engine::priv::ParticleSystem;
        public:
            void wait_for_all_engine_controlled();

            void finalize_job_engine_controlled(std::function<void()>& job);
            void finalize_job_engine_controlled(std::function<void()>& job, std::function<void()>& then);

            template<typename Job, typename... ARGS> void add_job_ref_engine_controlled(Job& job, ARGS&& ... args) {
                std::function<void()> job_ = std::bind(job, std::ref(std::forward<ARGS>(args))...);
                finalize_job_engine_controlled(job_);
            }
            template<typename Job, typename... ARGS> void add_job_engine_controlled(Job& job, ARGS&& ... args) {
                std::function<void()> job_ = std::bind(job, std::forward<ARGS>(args)...);
                finalize_job_engine_controlled(job_);
            }
            template<typename Job> void add_job_engine_controlled(Job& job) {
                std::function<void()> job_ = job;
                finalize_job_engine_controlled(job_);
            }
            template<typename Job, typename Then, typename... ARGS> void add_job_with_post_callback_ref_engine_controlled(Job& job, Then& then, ARGS&& ... args) {
                std::function<void()> job_ = std::bind(job, std::ref(std::forward<ARGS>(args))...);
                std::function<void()> then_ = then;
                finalize_job_engine_controlled(job_, then_);
            }
            template<typename Job, typename Then, typename... ARGS> void add_job_with_post_callback_engine_controlled(Job& job, Then& then, ARGS&& ... args) {
                std::function<void()> job_ = std::bind(job, std::forward<ARGS>(args)...);
                std::function<void()> then_ = then;
                finalize_job_engine_controlled(job_, then_);
            }
            template<typename Job, typename Then> void add_job_with_post_callback_engine_controlled(Job& job, Then& then) {
                std::function<void()> job_ = job;
                std::function<void()> then_ = then;
                finalize_job_engine_controlled(job_, then_);
            }
            template<typename Job, typename T, typename... ARGS> void add_job_engine_controlled_split_vectored(Job& job, std::vector<T>& collection, const bool waitForAll, ARGS&& ... args) {
                    
                if (Engine::priv::threading::hardware_concurrency() > 1) {
                    const auto split = Engine::priv::threading::splitVectorPairs(collection);
                    auto lamda = [&](const std::pair<size_t, size_t>& pair) {
                        for (size_t j = pair.first; j <= pair.second; ++j) {
                            T& thing = collection[j];
                            job(thing, j, std::forward<ARGS>(args)...);
                        }
                    };
                    for (auto& pair : split) {
                        Engine::priv::Core::m_Engine->m_ThreadManager.add_job_ref_engine_controlled(lamda, pair);
                    }
                    if (waitForAll)
                        Engine::priv::Core::m_Engine->m_ThreadManager.wait_for_all_engine_controlled();
                }else{
                    for (size_t j = 0; j < collection.size(); ++j) {
                        job(collection[j], j, std::forward<ARGS>(args)...);
                    }
                }
            }

        public:
            ThreadPool m_ThreadPool;
            ThreadPool m_ThreadPoolEngineControlled;

            ThreadManager();
            ~ThreadManager();

            void cleanup();

            void _update(const float dt);
    };
    namespace threading{

        const unsigned int hardware_concurrency();

        //splits vec into n subvectors of equal (or almost equal) number of elements in each split vector. if n is zero, then n will be equal to the number of cores your computer processor has.
        template<typename T> std::vector<std::vector<T>> splitVector(const std::vector<T>& v, size_t num_cores = 0) {
            if (num_cores == 0)
                num_cores = Engine::priv::threading::hardware_concurrency();
            const auto vs = v.size();
            std::vector<std::vector<T>> outVec;
            auto length = vs / num_cores;
            auto remain = vs % num_cores;
            size_t begin = 0;
            size_t end = 0;
            for (auto i = 0; i < std::min(num_cores, vs); ++i) {
                end += (remain > 0) ? (length + !!(remain--)) : length;
                outVec.emplace_back(v.begin() + begin, v.begin() + end);
                begin = end;
            }
            return outVec;
        }
        //splits vec into n subvectors of equal (or almost equal) number of elements in each split vector. if n is zero, then n will be equal to the number of cores your computer processor has.
        template<typename T> std::vector<std::vector<unsigned int>> splitVectorIndices(const std::vector<T>& v, size_t num_cores = 0) {
            if (num_cores == 0)
                num_cores = Engine::priv::threading::hardware_concurrency();
            const auto vs = v.size();
            std::vector<std::vector<unsigned int>> outVec;
            auto length = vs / num_cores;
            auto remain = vs % num_cores;
            size_t begin = 0;
            size_t end = 0;
            auto splitAmount = std::min(num_cores, vs);
            for (auto i = 0; i < splitAmount; ++i) {
                outVec.emplace_back();
                end += (remain > 0) ? (length + !!(remain--)) : length;
                auto splitSize = end - begin;
                outVec[i].resize(splitSize);
                for (auto j = 0; j < splitSize; ++j) {
                    outVec[i][j] = static_cast<unsigned int>(begin + j);
                }
                begin = end;
            }
            return outVec;
        }
        //creates a vector of pairs, each pair contains a start and ending index to iterate over a very large single vector
        template<typename T> std::vector<std::pair<size_t, size_t>> splitVectorPairs(const std::vector<T>& v, size_t num_cores = 0) {
            if (num_cores == 0)
                num_cores = Engine::priv::threading::hardware_concurrency();

            std::vector<std::pair<size_t, size_t>> outVec;
            if (v.size() <= num_cores) {
                outVec.emplace_back(  std::make_pair(0, v.size() - 1)  );
                return outVec;
            }


            const auto vector_size = v.size();
            outVec.reserve(num_cores);

            size_t c = vector_size / num_cores;
            size_t remainder = vector_size % num_cores; /* Likely uses the result of the division. */

            size_t accumulator = 0;

            std::pair<size_t, size_t> res;
            size_t b;
            size_t e = (num_cores - remainder);
            for (size_t i = 0; i < std::min(num_cores, vector_size); ++i) {
                if (c == 0)
                    b = remainder - 1;
                else
                    b = accumulator + (c - 1);
                if (i == e) {
                    if (i != (num_cores - remainder)) {
                        ++accumulator;
                        ++b;
                    }
                    ++b;
                    ++e;
                }
                res = std::make_pair(accumulator, b );
                outVec.push_back(res);
                accumulator += c;
            }
            return outVec;
        }


        void waitForAll();
        void finalizeJob(std::function<void()>& job);
        void finalizeJob(std::function<void()>& job, std::function<void()>& then);
         
        template<typename Job, typename... ARGS> void addJobRef(Job& job, ARGS&&... args) {
            std::function<void()> job_ = std::bind(job, std::ref(std::forward<ARGS>(args))...);
            finalizeJob(job_);
        }
        template<typename Job, typename... ARGS> void addJob(Job& job, ARGS&&... args){
            std::function<void()> job_ = std::bind(job, std::forward<ARGS>(args)...);
            finalizeJob(job_);
        }
        template<typename Job> void addJob(Job& job){
            std::function<void()> job_ = job;
            finalizeJob(job_);
        }
        template<typename Job> void addJob(Job job) {
            std::function<void()> job_ = job;
            finalizeJob(job_);
        }
        template<typename Job, typename Then, typename... ARGS> void addJobWithPostCallbackRef(Job& job, Then& then, ARGS&&... args) {
            std::function<void()> job_ = std::bind(job, std::ref(std::forward<ARGS>(args))...);
            std::function<void()> then_ = then;
            finalizeJob(job_, then_);
        }
        template<typename Job, typename Then, typename... ARGS> void addJobWithPostCallback(Job& job, Then& then, ARGS&&... args){
            std::function<void()> job_ = std::bind(job, std::forward<ARGS>(args)...);
            std::function<void()> then_ = then;
            finalizeJob(job_, then_);
        }
        template<typename Job, typename Then> void addJobWithPostCallback(Job& job, Then& then){
            std::function<void()> job_ = job;
            std::function<void()> then_ = then;
            finalizeJob(job_, then_);
        }
    };
};

#endif