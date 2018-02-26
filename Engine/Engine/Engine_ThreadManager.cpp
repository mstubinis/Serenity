#include <boost/asio/io_service.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/asio.hpp>
#include <boost/move/move.hpp>
#include <boost/atomic.hpp>
#include <boost/thread/thread.hpp>

#include "Engine.h"
#include "Engine_ThreadManager.h"

#include <iostream>

using namespace Engine;
using namespace std;

epriv::ThreadManager* threadManager;

class epriv::ThreadManager::impl final{
    public:
        boost::thread_group                            m_ThreadGroup;
		boost::asio::io_service                        m_IOService;
		uint                                           m_NumCores;
		//boost::atomic<uint>                          m_NumJobs;
		boost::asio::io_service::work*                 m_WorkControl;
		vector<boost::shared_future<void> >            m_PendingData; // vector of futures

		void _init(const char* name, uint& w, uint& h,ThreadManager* super){
			m_NumCores = boost::thread::hardware_concurrency(); if(m_NumCores == 0) m_NumCores = 1;
			m_WorkControl = new boost::asio::io_service::work(m_IOService);
			for(uint i = 0; i < m_NumCores; ++i){
				m_ThreadGroup.create_thread(boost::bind(&boost::asio::io_service::run, &m_IOService));
			}
		}
		void _postInit(const char* name, uint& w, uint& h,ThreadManager* super){
		}
		void _destruct(ThreadManager* super){
			delete(m_WorkControl);
			m_IOService.stop();
			m_ThreadGroup.join_all();
		}
};

epriv::ThreadManager::ThreadManager(const char* name, uint w, uint h):m_i(new impl){ m_i->_init(name,w,h,this); }
epriv::ThreadManager::~ThreadManager(){ m_i->_destruct(this); }
void epriv::ThreadManager::_init(const char* name, uint w, uint h){ 
	m_i->_postInit(name,w,h,this); 
	threadManager = epriv::Core::m_Engine->m_ThreadManager;
}
const uint epriv::ThreadManager::cores() const{ return threadManager->m_i->m_NumCores; }
//const uint epriv::ThreadManager::numJobs() const{ return threadManager->m_i->m_NumJobs; }

void epriv::threading::finalizeJob(boost::shared_ptr<boost::packaged_task<void>>& task){
	boost::shared_future<void> fut(task->get_future());
	threadManager->m_i->m_PendingData.push_back(fut);
	threadManager->m_i->m_IOService.post(boost::bind(&boost::packaged_task<void>::operator(), task));
}
void epriv::threading::waitForAll(){
	boost::wait_for_all(threadManager->m_i->m_PendingData.begin(), threadManager->m_i->m_PendingData.end());
}