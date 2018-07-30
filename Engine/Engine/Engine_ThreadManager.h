#pragma once
#ifndef ENGINE_THREAD_MANAGER_H
#define ENGINE_THREAD_MANAGER_H

#ifndef BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
#define BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION
#endif
#ifndef BOOST_RESULT_OF_USE_DECLTYPE
#define BOOST_RESULT_OF_USE_DECLTYPE
#endif

#include <boost/make_shared.hpp>
#include <boost/thread/future.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>
#include <SFML/System/Clock.hpp>

#include <memory>

typedef unsigned int uint;

typedef boost::packaged_task<void> boost_packed_task;

namespace Engine{
    namespace epriv{
        class ThreadManager final{
            public:
				class impl;
                std::unique_ptr<impl> m_i;

                ThreadManager(const char* name,uint w, uint h);
                ~ThreadManager();

                void _init(const char* name,uint w, uint h);
                void _update(const float& dt);

                const uint cores() const;
        };
        namespace threading{
            //splits vec into n subvectors of equal (or almost equal) number of elements in each split vector. if n is zero, then n will be equal to the number of cores your computer processor has.
            template<typename T> std::vector<std::vector<T>> splitVector(const std::vector<T>& vec,uint n = 0){
                if(n == 0) n = Core::m_Engine->m_ThreadManager->cores();
                std::vector<std::vector<T>> outVec;  uint length = vec.size() / n;  uint remain = vec.size() % n;  uint begin = 0;  uint end = 0;
                for (uint i = 0; i < std::min(n, vec.size()); ++i){
                    end += (remain > 0) ? (length + !!(remain--)) : length;
                    outVec.push_back(std::vector<T>(vec.begin() + begin, vec.begin() + end));  begin = end;
                }
                return outVec;
            }

            void waitForAll();
            void finalizeJob(const boost::shared_ptr<boost_packed_task>& task);
            void finalizeJob(const boost::shared_ptr<boost_packed_task>& task,const boost::function<void()>& then_task);
            
            template<typename T,typename U,typename V,typename W,typename X,typename Y> void addJob(void (*function_ptr)(T,U,V,W,X,Y),T a1,U a2,V a3,W a4,X a5,Y a6){
                boost::function<void()> j = boost::bind(function_ptr,a1,a2,a3,a4,a5,a6);
                auto job = boost::make_shared<boost_packed_task>(j);
                finalizeJob(job);
            }
            template<typename T,typename U,typename V,typename W,typename X> void addJob(void (*function_ptr)(T,U,V,W,X),T a1,U a2,V a3,W a4,X a5){
                boost::function<void()> j = boost::bind(function_ptr,a1,a2,a3,a4,a5);
                auto job = boost::make_shared<boost_packed_task>(j);
                finalizeJob(job);
            }
            template<typename T,typename U,typename V,typename W> void addJob(void (*function_ptr)(T,U,V,W),T a1,U a2,V a3,W a4){
                boost::function<void()> j = boost::bind(function_ptr,a1,a2,a3,a4);
                auto job = boost::make_shared<boost_packed_task>(j);
                finalizeJob(job);
            }
            template<typename T,typename U,typename V> void addJob(void (*function_ptr)(T,U,V),T a1,U a2,V a3){
                boost::function<void()> j = boost::bind(function_ptr,a1,a2,a3);
                auto job = boost::make_shared<boost_packed_task>(j);
                finalizeJob(job);
            }
            template<typename T,typename U> void addJob(void (*function_ptr)(T,U),T a1,U a2){
                boost::function<void()> j = boost::bind(function_ptr,a1,a2);
                auto job = boost::make_shared<boost_packed_task>(j);
                finalizeJob(job);
            }
            template<typename T> void addJob(void (*function_ptr)(T),T a1){
                boost::function<void()> j = boost::bind(function_ptr,a1);
                auto job = boost::make_shared<boost_packed_task>(j);
                finalizeJob(job);
            }
            static void addJob( void (*function_ptr)(void) ){
                boost::function<void()> j = boost::bind(function_ptr);
                auto job = boost::make_shared<boost_packed_task>(j);
                finalizeJob(job);
            }


            template<typename T, typename U, typename V, typename W,typename X,typename Y,typename Z> void addJob(T& functorJob,U& a1,V& a2,W& a3,X& a4,Y& a5,Z& a6){
                boost::function<void()> j = boost::bind(functorJob,a1,a2,a3,a4,a5,a6);
                auto job = boost::make_shared<boost_packed_task>(j);
                finalizeJob(job);
            }
            template<typename T, typename U, typename V, typename W,typename X,typename Y> void addJob(T& functorJob,U& a1,V& a2,W& a3,X& a4,Y& a5){
                boost::function<void()> j = boost::bind(functorJob,a1,a2,a3,a4,a5);
                auto job = boost::make_shared<boost_packed_task>(j);
                finalizeJob(job);
            }
            template<typename T, typename U, typename V, typename W,typename X> void addJob(T& functorJob,U& a1,V& a2,W& a3,X& a4){
                boost::function<void()> j = boost::bind(functorJob,a1,a2,a3,a4);
                auto job = boost::make_shared<boost_packed_task>(j);
                finalizeJob(job);
            }
            template<typename T, typename U, typename V, typename W> void addJob(T& functorJob,U& a1,V& a2,W& a3){
                boost::function<void()> j = boost::bind(functorJob,a1,a2,a3);
                auto job = boost::make_shared<boost_packed_task>(j);
                finalizeJob(job);
            }
            template<typename T, typename U, typename V> void addJob(T& functorJob,U& a1,V& a2){
                boost::function<void()> j = boost::bind(functorJob,a1,a2);
                auto job = boost::make_shared<boost_packed_task>(j);
                finalizeJob(job);
            }
            template<typename T, typename U> void addJob(T& functorJob,U& a1){
                boost::function<void()> j = boost::bind(functorJob,a1);
                auto job = boost::make_shared<boost_packed_task>(j);
                finalizeJob(job);
            }
            template<typename T> void addJob(T& functorJob){
                boost::function<void()> j = boost::bind(functorJob);
                auto job = boost::make_shared<boost_packed_task>(j);
                finalizeJob(job);
            }


            template<typename Job, typename Callback,typename U, typename V, typename W,typename X,typename Y,typename Z> void addJobWithPostCallback(Job& functorJob,Callback& postCallbackFunctor,U& a1,V& a2,W& a3,X& a4,Y& a5,Z& a6){
                boost::function<void()> then = boost::bind(postCallbackFunctor);
                auto job = boost::make_shared<boost_packed_task>(boost::bind(functorJob,a1,a2,a3,a4,a5,a6));
                finalizeJob(job,then);
            }
            template<typename Job, typename Callback,typename U, typename V, typename W,typename X,typename Y> void addJobWithPostCallback(Job& functorJob,Callback& postCallbackFunctor,U& a1,V& a2,W& a3,X& a4,Y& a5){
                boost::function<void()> then = boost::bind(postCallbackFunctor);
                auto job = boost::make_shared<boost_packed_task>(boost::bind(functorJob,a1,a2,a3,a4,a5));		
                finalizeJob(job,then);
            }
            template<typename Job, typename Callback,typename U, typename V, typename W,typename X> void addJobWithPostCallback(Job& functorJob,Callback& postCallbackFunctor,U& a1,V& a2,W& a3,X& a4){
                boost::function<void()> then = boost::bind(postCallbackFunctor);
                auto job = boost::make_shared<boost_packed_task>(boost::bind(functorJob,a1,a2,a3,a4));	
                finalizeJob(job,then);
            }
            template<typename Job,typename Callback,typename U, typename V, typename W> void addJobWithPostCallback(Job& functorJob,Callback& postCallbackFunctor,U& a1,V& a2,W& a3){
                boost::function<void()> then = boost::bind(postCallbackFunctor);
                auto job = boost::make_shared<boost_packed_task>(boost::bind(functorJob,a1,a2,a3));	
                finalizeJob(job,then);
            }
            template<typename Job, typename Callback, typename U,typename V> void addJobWithPostCallback(Job& functorJob,Callback& postCallbackFunctor,U& a1,V& a2){
                boost::function<void()> then = boost::bind(postCallbackFunctor);
                auto job = boost::make_shared<boost_packed_task>(boost::bind(functorJob,a1,a2));
                finalizeJob(job,then);
            }
            template<typename Job,typename Callback, typename U> void addJobWithPostCallback(Job& functorJob,Callback& postCallbackFunctor,U& a1){
                boost::function<void()> then = boost::bind(postCallbackFunctor);
                auto job = boost::make_shared<boost_packed_task>(boost::bind(functorJob,a1));	
                finalizeJob(job,then);
            }
            template<typename Job,typename Callback> void addJobWithPostCallback(Job& functorJob,Callback& postCallbackFunctor){
                boost::function<void()> then = boost::bind(postCallbackFunctor);
                auto job = boost::make_shared<boost_packed_task>(boost::bind(functorJob));	
                finalizeJob(job,then);
            }

            static void addJobWithPostCallback(boost::function<void()>& job, boost::function<void()>& callback){
                auto _job = boost::make_shared<boost_packed_task>(boost::bind(job));
                finalizeJob(_job,callback);
            }

        };
    };
};

#endif