#pragma once
#ifndef ENGINE_THREAD_MANAGER_H
#define ENGINE_THREAD_MANAGER_H

#ifndef BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
#define BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
#endif
#ifndef BOOST_RESULT_OF_USE_DECLTYPE
#define BOOST_RESULT_OF_USE_DECLTYPE
#endif

#include <core/engine/utils/Utils.h>
#include <memory>
#include <functional>

namespace Engine{
namespace epriv{
    class ThreadPool;
    class ThreadManager final{
        public:
            ThreadPool* m_ThreadPool;

            ThreadManager();
            ~ThreadManager();

            void _update(const double& dt);

            const uint cores() const;
    };
namespace threading{

    //splits vec into n subvectors of equal (or almost equal) number of elements in each split vector. if n is zero, then n will be equal to the number of cores your computer processor has.
    template<typename T> std::vector<std::vector<T>> splitVector(const std::vector<T>& v, uint n = 0) {
        if (n == 0)
            n = Core::m_Engine->m_ThreadManager.cores();
        const uint& vs = v.size();
        std::vector<std::vector<T>> outVec;
        uint length = vs / n;
        uint remain = vs % n;
        uint begin = 0;
        uint end = 0;
        for (uint i = 0; i < std::min(n, vs); ++i) {
            end += (remain > 0) ? (length + !!(remain--)) : length;
            outVec.emplace_back(v.begin() + begin, v.begin() + end);
            begin = end;
        }
        return outVec;
    }
    //splits vec into n subvectors of equal (or almost equal) number of elements in each split vector. if n is zero, then n will be equal to the number of cores your computer processor has.
    template<typename T> std::vector<std::vector<uint>> splitVectorIndices(const std::vector<T>& v, uint n = 0) {
        if (n == 0) 
            n = Core::m_Engine->m_ThreadManager.cores();
        const uint& vs = v.size();
        std::vector<std::vector<uint>> outVec;
        uint length = vs / n;
        uint remain = vs % n;
        uint begin = 0;
        uint end = 0;
        uint splitAmount = std::min(n, vs);
        for (uint i = 0; i < splitAmount; ++i) {
            outVec.emplace_back();
            end += (remain > 0) ? (length + !!(remain--)) : length;
            uint splitSize = end - begin;
            outVec[i].resize(splitSize);
            for (uint j = 0; j < splitSize; ++j) {
                outVec[i][j] = begin + j;
            }
            begin = end;
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