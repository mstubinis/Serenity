#include <boost/asio/io_service.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/move/move.hpp>
#include <boost/atomic.hpp>
#include <boost/thread/thread.hpp>

#include "Engine.h"
#include "Engine_ThreadManager.h"
#include "Engine_Utils.h"

#include <iostream>

using namespace Engine;
using namespace std;

epriv::ThreadManager* threadManager;

struct emptyFunctor{ void operator()() const {}};
struct EngineCallback{
    boost::shared_future<void> fut;
    boost::function<void()> cbk;
    EngineCallback(){ emptyFunctor e; cbk = boost::bind<void>(e); }
    ~EngineCallback(){}
};


class epriv::ThreadManager::impl final{
    public:
        boost::thread_group                 m_Threads;
        boost::asio::io_service             m_IOService;
        uint                                m_Cores;
        boost::asio::io_service::work*      m_Work;
		vector<EngineCallback>              m_Callbacks;
        void _init(const char* name, uint& w, uint& h,ThreadManager* super){
            m_Cores = boost::thread::hardware_concurrency(); if(m_Cores==0) m_Cores=1;
            m_Work = new boost::asio::io_service::work(m_IOService);
            for(uint i = 0; i < m_Cores; ++i){
                m_Threads.create_thread(boost::bind(&boost::asio::io_service::run, &m_IOService));
            }
        }
        void _postInit(const char* name, uint& w, uint& h,ThreadManager* super){
        }
        void _destruct(ThreadManager* super){
            delete(m_Work);
            m_IOService.stop();
            m_Threads.join_all();
        }
        void _clearDoneCallbacks(){
            if(m_Callbacks.size() == 0) return;
            for(auto it = m_Callbacks.begin(); it != m_Callbacks.end();){ 
                EngineCallback& fut = (*it); 
                if(fut.fut.is_ready()){
                    fut.cbk();
                    it = m_Callbacks.erase(it);
                } 
                else{ ++it; } 
            }
        }
        void _update(const float& dt, epriv::ThreadManager* super){      
            _clearDoneCallbacks();
        }
};

epriv::ThreadManager::ThreadManager(const char* name, uint w, uint h):m_i(new impl){ m_i->_init(name,w,h,this); }
epriv::ThreadManager::~ThreadManager(){ m_i->_destruct(this); }
void epriv::ThreadManager::_init(const char* name, uint w, uint h){ 
    m_i->_postInit(name,w,h,this); 
    threadManager = epriv::Core::m_Engine->m_ThreadManager;
}
void epriv::ThreadManager::_update(const float& dt){ m_i->_update(dt,this); }
const uint epriv::ThreadManager::cores() const{ return threadManager->m_i->m_Cores; }
void epriv::threading::finalizeJob(boost::shared_ptr<boost_packed_task>& task){
    boost::unique_future<void> future = task->get_future();
    EngineCallback e;
    e.fut = boost::move(future);
    threadManager->m_i->m_Callbacks.push_back(e);
    threadManager->m_i->m_IOService.post(boost::bind(&boost_packed_task::operator(), task));
}
void epriv::threading::finalizeJob(boost::shared_ptr<boost_packed_task>& task, boost::function<void()>& then_task){
    boost::unique_future<void> future = task->get_future();
    EngineCallback e;
    e.fut = boost::move(future);
    e.cbk = boost::bind<void>(then_task);
    threadManager->m_i->m_Callbacks.push_back(e);
    threadManager->m_i->m_IOService.post(boost::bind(&boost_packed_task::operator(), task));
}
void epriv::threading::waitForAll(){ 
	if(threadManager->m_i->m_Callbacks.size() > 0){
		for(auto callback: threadManager->m_i->m_Callbacks){
			callback.fut.wait();
			//if(!callback.fut.is_ready()){
			//	return;
			//}
		}
		threadManager->m_i->_clearDoneCallbacks();
	}
}