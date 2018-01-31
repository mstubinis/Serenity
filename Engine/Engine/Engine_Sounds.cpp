#include "Engine_Sounds.h"
#include "Engine_Resources.h"
#include "Engine_SoundQueue.h"

using namespace Engine;
using namespace std;

vector<boost::shared_ptr<SoundBaseClass>> Sound::Detail::SoundManagement::m_CurrentlyPlayingSounds;
vector<boost::shared_ptr<SoundQueue>> Sound::Detail::SoundManagement::m_SoundQueues;
unordered_map<string,boost::shared_ptr<SoundData>> Sound::Detail::SoundManagement::m_SoundData;

void Sound::Detail::SoundManagement::init(){

}
void Sound::Detail::SoundManagement::destruct(){
    for (auto it = m_SoundData.begin();it != m_SoundData.end(); ++it ) it->second.reset();
    for(auto q:m_SoundQueues){ 
		q.reset();
	}
    for(auto s:m_CurrentlyPlayingSounds){ 
		s.reset();
	}
	vector_clear(m_SoundQueues);
	vector_clear(m_CurrentlyPlayingSounds);
}
void Sound::Detail::SoundManagement::update(float dt){
    vector<boost::shared_ptr<SoundBaseClass>>::iterator it;
    for(it = m_CurrentlyPlayingSounds.begin(); it != m_CurrentlyPlayingSounds.end();){
		SoundBaseClass* s = (*it).get();
        if(s->status() == SoundStatus::Stopped){
            it = m_CurrentlyPlayingSounds.erase(it);
        }
        else{
            s->update(dt);
            ++it;
        }
    }
    vector<boost::shared_ptr<SoundQueue>>::iterator it1;
    for(it1 = m_SoundQueues.begin(); it1 != m_SoundQueues.end();){
		SoundQueue* s = (*it1).get();
		if(s->empty()){
			it1 = m_SoundQueues.erase(it1);
		}
		else{
			s->update(dt);
			++it1;
		}
	}
}
void Sound::Detail::SoundManagement::addSoundDataFromFile(string file){
    if(!m_SoundData.count(file)){
        m_SoundData.emplace(file,boost::make_shared<SoundData>(file));
    }
}
void Sound::Detail::SoundManagement::addSoundDataFromFile(string name,string file){
    if(!m_SoundData.count(name)){
        m_SoundData.emplace(name,boost::make_shared<SoundData>(file));
    }
}


class SoundData::impl{
    public:
        sf::SoundBuffer* m_Buffer;
        std::string m_File;

        void init(){
            m_Buffer = new sf::SoundBuffer();
            m_File = "";
        }
        void loadFromFile(string file){
            init();
            m_Buffer->loadFromFile(file);
            m_File = file;
        }

};
SoundData::SoundData():m_i(new impl()){
    m_i->init();
}
SoundData::SoundData(string file):m_i(new impl()){
    m_i->loadFromFile(file);
}
SoundData::~SoundData(){
	SAFE_DELETE(m_i->m_Buffer);
	SAFE_DELETE(m_i);
}
sf::SoundBuffer* SoundData::getBuffer(){ return m_i->m_Buffer; }
std::string SoundData::getFilename(){ return m_i->m_File; }



class SoundBaseClass::impl{
    public:
        SoundStatus::Status m_Status;
};
SoundBaseClass::SoundBaseClass(bool n):m_i(new impl()){
    m_i->m_Status = SoundStatus::Fresh;
}
SoundBaseClass::~SoundBaseClass(){
    m_i->m_Status = SoundStatus::Stopped;
	SAFE_DELETE(m_i);
}
SoundStatus::Status SoundBaseClass::status(){ return m_i->m_Status; }
void SoundBaseClass::play(bool loop){ if(loop) m_i->m_Status = SoundStatus::PlayingLooped; else m_i->m_Status = SoundStatus::Playing; }
void SoundBaseClass::pause(){ m_i->m_Status = SoundStatus::Paused; }
void SoundBaseClass::stop(){ m_i->m_Status = SoundStatus::Stopped; }
void SoundBaseClass::update(float dt){}
void SoundBaseClass::loop(bool loop){ SoundBaseClass::play(loop); }

class SoundEffect::impl{
    public:
        sf::Sound m_Sound;

        void _init(SoundBaseClass* s,string file,bool queue){
            if(file != "") _loadFromFile(file);
			if(queue == false){ 
				Sound::Detail::SoundManagement::m_CurrentlyPlayingSounds.push_back( boost::shared_ptr<SoundBaseClass>(s) );
                s->play();
			}
        }
        void _loadFromFile(string& file){
            if(!Sound::Detail::SoundManagement::m_SoundData.count(file)){
                Sound::Detail::SoundManagement::addSoundDataFromFile(file);
            }
            m_Sound.setBuffer( *(Sound::Detail::SoundManagement::m_SoundData.at(file).get()->getBuffer()) );
        }
        void _update(float dt,SoundBaseClass* super){
            if(m_Sound.getStatus() == sf::SoundSource::Status::Stopped){
                super->stop();
            }
        }
		void _play(bool& loop,SoundBaseClass* super){
			m_Sound.play();
			if(loop){
				m_Sound.setLoop(true);
			}
		}
		void _pause(SoundBaseClass* super){
			m_Sound.pause();
		}
		void _stop(SoundBaseClass* super){
			m_Sound.stop();
		}
		void _loop(bool& loop,SoundBaseClass* super){
			if(loop){
				m_Sound.setLoop(true);
			}
			else{
				m_Sound.setLoop(false);
			}
		}
};
SoundEffect::SoundEffect(string file,bool queue):SoundBaseClass(),m_i(new impl()){
    m_i->_init(this,file,queue);
}
SoundEffect::~SoundEffect(){
	SAFE_DELETE(m_i);
}
void SoundEffect::loadFromFile(string file){
    m_i->_loadFromFile(file);
}
void SoundEffect::play(bool loop){
	SoundStatus::Status stat = status();
	if(stat == SoundStatus::Playing || stat == SoundStatus::PlayingLooped) return;
	SoundBaseClass::play(loop);
	m_i->_play(loop,this);
}
void SoundEffect::pause(){
	if(status() == SoundStatus::Paused) return;
	SoundBaseClass::pause();
	m_i->_pause(this);
}
void SoundEffect::stop(){
	if(status() == SoundStatus::Stopped) return;
	SoundBaseClass::stop();
	m_i->_stop(this);
}
void SoundEffect::update(float dt){
    m_i->_update(dt,this);
}
void SoundEffect::loop(bool loop){
	SoundBaseClass::play(loop);
	m_i->_loop(loop,this);
}

class SoundMusic::impl{
    public:
        sf::Music m_Sound;
        std::string m_File;

        void _init(SoundBaseClass* s,string file,bool queue){
            m_File = "";
            if(file != "") _loadFromFile(file);
			if(queue == false){ 
				Sound::Detail::SoundManagement::m_CurrentlyPlayingSounds.push_back( boost::shared_ptr<SoundBaseClass>(s) );
                s->play();
			}
        }
        void _loadFromFile(string& file){
            if (!m_Sound.openFromFile(file)){
                // error...
            }
            else{
                //good
                m_File = file;
            }
        }
        void _update(float dt,SoundBaseClass* super){
            if(m_Sound.getStatus() == sf::SoundSource::Status::Stopped){
                super->stop();
            }
        }
		void _play(bool& loop,SoundBaseClass* super){
			m_Sound.play();
			if(loop){
				m_Sound.setLoop(true);
			}
		}
		void _pause(SoundBaseClass* super){
			m_Sound.pause();
		}
		void _stop(SoundBaseClass* super){
			m_Sound.stop();
		}
		void _loop(bool& loop,SoundBaseClass* super){
			if(loop){
				m_Sound.setLoop(true);
			}
			else{
				m_Sound.setLoop(false);
			}
		}
};
SoundMusic::SoundMusic(string file,bool queue):SoundBaseClass(),m_i(new impl()){
    m_i->_init(this,file,queue);
}
SoundMusic::~SoundMusic(){
    SAFE_DELETE(m_i);
}
void SoundMusic::loadFromFile(string file){
    m_i->_loadFromFile(file);
}
void SoundMusic::play(bool loop){
	SoundStatus::Status stat = status();
	if(stat == SoundStatus::Playing || stat == SoundStatus::PlayingLooped) return;
	SoundBaseClass::play(loop);
	m_i->_play(loop,this);
}
void SoundMusic::pause(){
	if(status() == SoundStatus::Paused) return;
	SoundBaseClass::pause();
	m_i->_pause(this);
}
void SoundMusic::stop(){
	if(status() == SoundStatus::Stopped) return;
	SoundBaseClass::stop();
    m_i->_stop(this);
}
void SoundMusic::update(float dt){
    m_i->_update(dt,this);
}
void SoundMusic::loop(bool loop){
	SoundBaseClass::play(loop);
    m_i->_loop(loop,this);
}