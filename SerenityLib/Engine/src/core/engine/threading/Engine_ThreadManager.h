#pragma once
#ifndef ENGINE_THREAD_MANAGER_H
#define ENGINE_THREAD_MANAGER_H

#ifndef BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
#define BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
#endif
#ifndef BOOST_RESULT_OF_USE_DECLTYPE
#define BOOST_RESULT_OF_USE_DECLTYPE
#endif

#include <memory>
#include <functional>
#include <utility>
#include <thread>

namespace Engine{
    namespace priv{
        class ThreadPool;
        class ThreadManager final{
            public:
                ThreadPool* m_ThreadPool;

                ThreadManager();
                ~ThreadManager();

                void cleanup();

                void _update(const double& dt);
        };
        namespace threading{

            //splits vec into n subvectors of equal (or almost equal) number of elements in each split vector. if n is zero, then n will be equal to the number of cores your computer processor has.
            template<typename T> std::vector<std::vector<T>> splitVector(const std::vector<T>& v, size_t num_cores = 0) {
                if (num_cores == 0)
                    num_cores = std::thread::hardware_concurrency();
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
                    num_cores = std::thread::hardware_concurrency();
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
                    num_cores = std::thread::hardware_concurrency();

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
            void finalizeJob(std::function<void()>& task);
            void finalizeJob(std::function<void()>& task, std::function<void()>& then_task);
         
            template<typename Job, typename... ARGS> void addJobRef(Job& _job, ARGS&&... _args) {
                std::function<void()> job = std::bind(_job, std::ref(std::forward<ARGS>(_args))...);
                finalizeJob(job);
            }
            template<typename Job, typename... ARGS> void addJob(Job& _job,ARGS&&... _args){
                std::function<void()> job = std::bind(_job, std::forward<ARGS>(_args)...);
                finalizeJob(job);
            }
            template<typename Job> void addJob(Job& _job){
                std::function<void()> job = std::bind(_job);
                finalizeJob(job);
            }
            template<typename Job, typename Then, typename... ARGS> void addJobWithPostCallbackRef(Job& _job, Then& _then, ARGS&&... _args) {
                std::function<void()> job = std::bind(_job, std::ref(std::forward<ARGS>(_args))...);
                std::function<void()> then = std::bind(_then);
                finalizeJob(job, then);
            }
            template<typename Job, typename Then, typename... ARGS> void addJobWithPostCallback(Job& _job, Then& _then,ARGS&&... _args){
                std::function<void()> job = std::bind(_job, std::forward<ARGS>(_args)...);
                std::function<void()> then = std::bind(_then);
                finalizeJob(job, then);
            }
            template<typename Job, typename Then> void addJobWithPostCallback(Job& _job, Then& _then){
                std::function<void()> job = std::bind(_job);
                std::function<void()> then = std::bind(_then);
                finalizeJob(job, then);
            }
        };
    };
};

#endif