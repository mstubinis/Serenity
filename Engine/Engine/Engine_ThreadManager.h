#pragma once
#ifndef ENGINE_THREAD_MANAGER_H
#define ENGINE_THREAD_MANAGER_H

#define BOOST_THREAD_PROVIDES_FUTURE
#define BOOST_THREAD_PROVIDES_FUTURE_CONTINUATION

#include <boost/make_shared.hpp>
#include <boost/thread/future.hpp>
#include <boost/bind.hpp>
#include <SFML/System/Clock.hpp>

#include <memory>

typedef unsigned int uint;

class Mesh;

typedef boost::packaged_task<void> boost_packed_task;

struct emptyFunctor{void operator()() const {}};

namespace Engine{
	namespace epriv{
		class ThreadManager final{
		    private:
				class impl;
		    public:
				std::unique_ptr<impl> m_i;

				ThreadManager(const char* name,uint w, uint h);
				~ThreadManager();

				static emptyFunctor emptyTask;

				void _init(const char* name,uint w, uint h);
				const uint cores() const;
				const uint numJobs() const;
		};
		namespace threading{
			//splits vec into n subvectors of equal (or almost equal) number of elements in each split vector. if n is zero, then n will be equal to the number of cores your computer processor has.
			template<typename T> std::vector<std::vector<T>> splitVector(const std::vector<T>& vec,uint n = 0){

				if(n == 0)
					n = Core::m_Engine->m_ThreadManager->cores();

				std::vector<std::vector<T>> outVec;
				uint length = vec.size() / n;
				uint remain = vec.size() % n;
				uint begin = 0;  uint end = 0;
				for (uint i = 0; i < std::min(n, vec.size()); ++i){
					end += (remain > 0) ? (length + !!(remain--)) : length;
					outVec.push_back(std::vector<T>(vec.begin() + begin, vec.begin() + end));
					begin = end;
				}
				return outVec;
			}

			void waitForAll();
			void finalizeJob(boost::shared_ptr<boost_packed_task>& task);

			template<typename T,typename U,typename V,typename W,typename X,typename Y> void addJob(void (*ptr)(T,U,V,W,X,Y),T a1,U a2,V a3,W a4,X a5,Y a6){
				auto task = boost::make_shared<boost_packed_task>(boost::bind(ptr,a1,a2,a3,a4,a5,a6)); finalizeJob(task);
			}
			template<typename T,typename U,typename V,typename W,typename X> void addJob(void (*ptr)(T,U,V,W,X),T a1,U a2,V a3,W a4,X a5){
				auto task = boost::make_shared<boost_packed_task>(boost::bind(ptr,a1,a2,a3,a4,a5)); finalizeJob(task);
			}
			template<typename T,typename U,typename V,typename W> void addJob(void (*ptr)(T,U,V,W),T a1,U a2,V a3,W a4){
				auto task = boost::make_shared<boost_packed_task>(boost::bind(ptr,a1,a2,a3,a4)); finalizeJob(task);
			}
			template<typename T,typename U,typename V> void addJob(void (*ptr)(T,U,V),T a1,U a2,V a3){
				auto task = boost::make_shared<boost_packed_task>(boost::bind(ptr,a1,a2,a3)); finalizeJob(task);
			}
			template<typename T,typename U> void addJob(void (*ptr)(T,U),T a1,U a2){
				auto task = boost::make_shared<boost_packed_task>(boost::bind(ptr,a1,a2)); finalizeJob(task);
			}
			template<typename T> void addJob(void (*ptr)(T),T a1){
				auto task = boost::make_shared<boost_packed_task>(boost::bind(ptr,a1)); finalizeJob(task);
			}
			static void addJob( void (*ptr)(void) ){
				auto task = boost::make_shared<boost_packed_task>(boost::bind(ptr)); finalizeJob(task);
			}

			template<typename T, typename U, typename V, typename W,typename X,typename Y,typename Z> void addJob(T& functor,U a1,V a2,W a3,X a4,Y a5,Z a6){
				auto task = boost::make_shared<boost_packed_task>(boost::bind(functor,a1,a2,a3,a4,a5,a6)); finalizeJob(task);
			}
			template<typename T, typename U, typename V, typename W,typename X,typename Y> void addJob(T& functor,U a1,V a2,W a3,X a4,Y a5){
				auto task = boost::make_shared<boost_packed_task>(boost::bind(functor,a1,a2,a3,a4,a5)); finalizeJob(task);
			}
			template<typename T, typename U, typename V, typename W,typename X> void addJob(T& functor,U a1,V a2,W a3,X a4){
				auto task = boost::make_shared<boost_packed_task>(boost::bind(functor,a1,a2,a3,a4)); finalizeJob(task);
			}
			template<typename T, typename U, typename V, typename W> void addJob(T& functor,U a1,V a2,W a3){
				auto task = boost::make_shared<boost_packed_task>(boost::bind(functor,a1,a2,a3)); finalizeJob(task);
			}
			template<typename T, typename U, typename V> void addJob(T& functor,U a1,V a2){
				auto task = boost::make_shared<boost_packed_task>(boost::bind(functor,a1,a2)); finalizeJob(task);
			}
			template<typename T, typename U> void addJob(T& functor,U a1){
				auto task = boost::make_shared<boost_packed_task>(boost::bind(functor,a1)); finalizeJob(task);
			}
			template<typename T> void addJob(T& functor){
				auto task = boost::make_shared<boost_packed_task>(boost::bind(functor)); finalizeJob(task);
			}
		};
	};
};

#endif