#include "core/engine/Engine_ThreadManager.h"
#include "core/engine/Engine_Utils.h"

#include <boost/asio/io_service.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/atomic.hpp>
#include <boost/thread/thread.hpp>

using namespace Engine;
using namespace std;

typedef boost::shared_future<void> boost_shared_fut;
typedef boost::asio::io_service boost_asio_service;

epriv::ThreadManager::impl* threadManager;

struct emptyFunctor{ void operator()() const {}};
struct EngineCallback{
    boost_shared_fut fut;
    boost_void_func cbk;
    EngineCallback(){ 
        emptyFunctor e; 
        cbk = boost::bind<void>(e); 
    }
    EngineCallback(boost_shared_fut _fut){ 
        emptyFunctor e;
        fut = _fut;
        cbk = boost::bind<void>(e); 
    }
    EngineCallback(boost_shared_fut _fut, boost_void_func _cbk){ 
        fut = _fut;
        cbk = _cbk; 
    }
    ~EngineCallback(){}
};

class epriv::ThreadManager::impl final{
    public:
        boost::thread_group                 m_Threads;
        boost_asio_service                  m_IOService;
        uint                                m_Cores;
        boost_asio_service::work*           m_Work;
        vector<EngineCallback>              m_Callbacks;
        void _init(const char* name, uint& w, uint& h){
            m_Cores = boost::thread::hardware_concurrency(); if(m_Cores==0) m_Cores=1;
            m_Work = new boost_asio_service::work(m_IOService);
            for(uint i = 0; i < m_Cores; ++i){
                m_Threads.create_thread(boost::bind(&boost_asio_service::run, &m_IOService));
            }
        }
        void _destruct(){
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
        void _update(const float& dt){      
            _clearDoneCallbacks();
        }
};

epriv::ThreadManager::ThreadManager(const char* name, uint w, uint h):m_i(new impl){ m_i->_init(name,w,h); threadManager = m_i.get(); }
epriv::ThreadManager::~ThreadManager(){ m_i->_destruct(); }
void epriv::ThreadManager::_update(const float& dt){ m_i->_update(dt); }
const uint epriv::ThreadManager::cores() const{ return threadManager->m_Cores; }
void epriv::threading::finalizeJob( boost_packed_task_ptr&& task){
    auto& mgr = *threadManager;
    mgr.m_Callbacks.emplace_back( std::move(boost_shared_fut( task->get_future() )) );
    mgr.m_IOService.post(boost::bind(&boost_packed_task::operator(), task));
}
void epriv::threading::finalizeJob( boost_packed_task_ptr&& task, boost_void_func&& then_task){
    auto& mgr = *threadManager;
    EngineCallback e(std::move(boost_shared_fut( task->get_future())));
    e.cbk = boost::bind<void>(then_task); 
    mgr.m_Callbacks.push_back( std::move(e) );
    mgr.m_IOService.post(boost::bind(&boost_packed_task::operator(), task));
}
void epriv::threading::waitForAll(){ 
    if(threadManager->m_Callbacks.size() > 0){
        for(auto& callback: threadManager->m_Callbacks){
            callback.fut.wait();
        }
        threadManager->_clearDoneCallbacks();
    }
}