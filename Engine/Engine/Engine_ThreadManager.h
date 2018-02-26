#pragma once
#ifndef ENGINE_THREAD_MANAGER_H
#define ENGINE_THREAD_MANAGER_H

#include <boost/make_shared.hpp>
#include <boost/thread/future.hpp>
#include <boost/bind.hpp>
#include <SFML/System/Clock.hpp>

#include <memory>

typedef unsigned int uint;

class Mesh;

namespace Engine{
	namespace epriv{
		class ThreadManager final{
		    private:
				class impl;
		    public:
				std::unique_ptr<impl> m_i;

				ThreadManager(const char* name,uint w, uint h);
				~ThreadManager();

				void _init(const char* name,uint w, uint h);
				const uint cores() const;
				//const uint numJobs() const;
		};
		namespace threading{
	
			void waitForAll();
			void finalizeJob(boost::shared_ptr<boost::packaged_task<void>>& task);

			template<typename T,typename U,typename V,typename W,typename X,typename Y> void addJob(void (*ptr)(T,U,V,W,X,Y),T a1,U a2,V a3,W a4,X a5,Y a6){
				auto task = boost::make_shared<boost::packaged_task<void>>(boost::bind(ptr,a1,a2,a3,a4,a5,a6)); epriv::threading::finalizeJob(task);
			}
			template<typename T,typename U,typename V,typename W,typename X> void addJob(void (*ptr)(T,U,V,W,X),T a1,U a2,V a3,W a4,X a5){
				auto task = boost::make_shared<boost::packaged_task<void>>(boost::bind(ptr,a1,a2,a3,a4,a5)); epriv::threading::finalizeJob(task);
			}
			template<typename T,typename U,typename V,typename W> void addJob(void (*ptr)(T,U,V,W),T a1,U a2,V a3,W a4){
				auto task = boost::make_shared<boost::packaged_task<void>>(boost::bind(ptr,a1,a2,a3,a4)); epriv::threading::finalizeJob(task);
			}
			template<typename T,typename U,typename V> void addJob(void (*ptr)(T,U,V),T a1,U a2,V a3){
				auto task = boost::make_shared<boost::packaged_task<void>>(boost::bind(ptr,a1,a2,a3)); epriv::threading::finalizeJob(task);
			}
			template<typename T,typename U> void addJob(void (*ptr)(T,U),T a1,U a2){
				auto task = boost::make_shared<boost::packaged_task<void>>(boost::bind(ptr,a1,a2)); epriv::threading::finalizeJob(task);
			}
			template<typename T> void addJob(void (*ptr)(T),T a1){
				auto task = boost::make_shared<boost::packaged_task<void>>(boost::bind(ptr,a1)); epriv::threading::finalizeJob(task);
			}
			static void addJob( void (*ptr)(void) ){
				auto task = boost::make_shared<boost::packaged_task<void>>(boost::bind(ptr)); epriv::threading::finalizeJob(task);
			}

			template<typename T, typename U, typename V, typename W,typename X,typename Y,typename Z> void addJob(T functor,U a1,V a2,W a3,X a4,Y a5,Z a6){
				auto task = boost::make_shared<boost::packaged_task<void>>(boost::bind(functor,a1,a2,a3,a4,a5,a6)); epriv::threading::finalizeJob(task);
			}
			template<typename T, typename U, typename V, typename W,typename X,typename Y> void addJob(T functor,U a1,V a2,W a3,X a4,Y a5){
				auto task = boost::make_shared<boost::packaged_task<void>>(boost::bind(functor,a1,a2,a3,a4,a5)); epriv::threading::finalizeJob(task);
			}
			template<typename T, typename U, typename V, typename W,typename X> void addJob(T functor,U a1,V a2,W a3,X a4){
				auto task = boost::make_shared<boost::packaged_task<void>>(boost::bind(functor,a1,a2,a3,a4)); epriv::threading::finalizeJob(task);
			}
			template<typename T, typename U, typename V, typename W> void addJob(T functor,U a1,V a2,W a3){
				auto task = boost::make_shared<boost::packaged_task<void>>(boost::bind(functor,a1,a2,a3)); epriv::threading::finalizeJob(task);
			}
			template<typename T, typename U, typename V> void addJob(T functor,U a1,V a2){
				auto task = boost::make_shared<boost::packaged_task<void>>(boost::bind(functor,a1,a2)); epriv::threading::finalizeJob(task);
			}
			template<typename T, typename U> void addJob(T functor,U a1){
				auto task = boost::make_shared<boost::packaged_task<void>>(boost::bind(functor,a1)); epriv::threading::finalizeJob(task);
			}
			template<typename T> void addJob(T functor){
				auto task = boost::make_shared<boost::packaged_task<void>>(boost::bind(functor)); epriv::threading::finalizeJob(task);
			}
		};
	};
};

#endif