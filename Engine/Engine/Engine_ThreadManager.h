#pragma once
#ifndef ENGINE_THREAD_MANAGER_H
#define ENGINE_THREAD_MANAGER_H

#ifndef BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
#define BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
#endif
#ifndef BOOST_RESULT_OF_USE_DECLTYPE
#define BOOST_RESULT_OF_USE_DECLTYPE
#endif

#include "Engine_Utils.h"
#include <boost/thread/future.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <memory>

typedef boost::packaged_task<void>             boost_packed_task;
typedef boost::shared_ptr<boost_packed_task>   boost_packed_task_ptr;
typedef boost::function<void()>                boost_void_func;

namespace Engine{
    namespace epriv{
        class ThreadManager final{
            public:
                class impl; std::unique_ptr<impl> m_i;

                ThreadManager(const char* name,uint w, uint h);
                ~ThreadManager();

                void _update(const float& dt);

                const uint cores() const;
        };
        namespace threading{
            //splits vec into n subvectors of equal (or almost equal) number of elements in each split vector. if n is zero, then n will be equal to the number of cores your computer processor has.
            template<typename T> std::vector<std::vector<T>> splitVector(const std::vector<T>& v,uint n = 0){
                if(n == 0) n = Core::m_Engine->m_ThreadManager.cores();
                const uint& vs = v.size();
                std::vector<std::vector<T>> outVec;  uint length = vs / n;  uint remain = vs % n;  uint begin = 0;  uint end = 0;
                for (uint i = 0; i < std::min(n, vs); ++i){
                    end += (remain > 0) ? (length + !!(remain--)) : length;
                    outVec.emplace_back(v.begin() + begin, v.begin() + end);
                    begin = end;
                }
                return outVec;
            }

            void waitForAll();
            void finalizeJob( boost_packed_task_ptr&& task);
            void finalizeJob( boost_packed_task_ptr&& task, boost_void_func&& then_task);
            
            template<typename Job, typename... ARGS> void addJob(Job&& _job,ARGS&&... _args){
                auto job = boost::make_shared<boost_packed_task>(boost::bind(boost::move(_job), std::forward<ARGS>(_args)...));
                finalizeJob(boost::move(job));
            }
            template<typename Job> void addJob(Job&& _job){
                auto job = boost::make_shared<boost_packed_task>(boost::bind(boost::move(_job)));
                finalizeJob(boost::move(job));
            }    
            template<typename Job,typename Then, typename... ARGS> void addJobWithPostCallback(Job&& _job, Then&& _then,ARGS&&... _args){
                auto job = boost::make_shared<boost_packed_task>(boost::bind(boost::move(_job),std::forward<ARGS>(_args)...));
                finalizeJob(boost::move(job), boost::move(boost::bind(boost::move(_then))));
            }
            template<typename Job,typename Then> void addJobWithPostCallback(Job&& _job, Then&& _then){
                auto job = boost::make_shared<boost_packed_task>(boost::bind(boost::move(_job)));
                finalizeJob(boost::move(job), boost::move(boost::bind(boost::move(_then))));
            }
        };
    };
};

#endif