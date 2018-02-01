#include "Engine_SoundQueue.h"
#include "Engine_Sounds.h"
#include "Engine_Resources.h"
#include <vector>

#include <boost/shared_ptr.hpp>

using namespace std;
using namespace Engine;

class SoundQueue::impl{
    public:
		vector< boost::shared_ptr<SoundBaseClass> > m_Queue;
		float m_DelayInSeconds;
		float m_DelayTimer;
		bool m_IsDelayProcess;

		void _init(float _delay){
			m_DelayInSeconds = _delay;
			m_DelayTimer = 0;
			m_IsDelayProcess = false;
		}
		void _clear(){
			vector< boost::shared_ptr<SoundBaseClass> >::iterator it1;
			for(it1 = m_Queue.begin(); it1 != m_Queue.end();){
				(*it1).reset();
				it1 = m_Queue.erase(it1);
			}
			vector_clear(m_Queue);
		}
		void _destruct(){
            _clear();
		}
		void _dequeue(){
			if(m_Queue.size() > 0){
				auto it = m_Queue.begin();
				(*it).reset();
				m_Queue.erase(it);
				m_IsDelayProcess = true;
				//do we need to manually delete? i think so
			}
		}
		void _update(float dt){
			if(m_IsDelayProcess){
				m_DelayTimer += dt;
				if(m_DelayTimer > m_DelayInSeconds){
					m_IsDelayProcess = false;
					m_DelayTimer = 0;
				}
			}
			else{
				if(m_Queue.size() > 0){
					vector< boost::shared_ptr<SoundBaseClass> >::iterator it1;
					uint count = 0;
					for(it1 = m_Queue.begin(); it1 != m_Queue.end();){
						SoundBaseClass* s = (*it1).get();
						const SoundStatus::Status& stat = s->status();
						if(stat == SoundStatus::Fresh && count == 0){
							//play it
							s->play();
							s->update(dt);
							++it1;
						}
						else if(stat == SoundStatus::Playing || stat == SoundStatus::PlayingLooped && count == 0){
							s->update(dt);
							++it1;
						}
						else if(stat == SoundStatus::Stopped && count == 0){
							if(s->getLoopsLeft() == 0){
								//this sound has finished, remove it from the queue and start the delay process
								//do we need to manually delete? i think so
								(*it1).reset();
  								it1 = m_Queue.erase(it1);
								m_IsDelayProcess = true;
							}
							else{
								++it1;
							}
						}
						else{
							++it1;
						}
						count++;
					}
				}
			}
		}
};

SoundQueue::SoundQueue(float _delay):m_i(new impl()){
	m_i->_init(_delay);
	Sound::Detail::SoundManagement::m_SoundQueues.push_back( boost::shared_ptr<SoundQueue>(this) );
}
SoundQueue::~SoundQueue(){
	m_i->_destruct();
	SAFE_DELETE(m_i);
}
void SoundQueue::enqueueEffect(string file,uint loops){
	m_i->m_Queue.push_back( boost::make_shared<SoundEffect>(file,loops,true) );
}
void SoundQueue::enqueueMusic(string file,uint loops){
	m_i->m_Queue.push_back( boost::make_shared<SoundMusic>(file,loops,true) );
}
void SoundQueue::dequeue(){
	m_i->_dequeue();
}
void SoundQueue::update(float dt){
	m_i->_update(dt);
}
void SoundQueue::clear(){
	m_i->_clear();
}
bool SoundQueue::empty(){
	if(m_i->m_Queue.size() > 0) return false; return true;
}