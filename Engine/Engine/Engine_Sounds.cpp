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
    for(auto it = m_CurrentlyPlayingSounds.begin(); it != m_CurrentlyPlayingSounds.end();){
		SoundBaseClass* s = (*it).get();
		s->update(dt);
        if(s->status() == SoundStatus::Stopped){
            it = m_CurrentlyPlayingSounds.erase(it);
        }
        else{      
            ++it;
        }
    }
    for(auto it1 = m_SoundQueues.begin(); it1 != m_SoundQueues.end();){
		SoundQueue* s = (*it1).get();
		s->update(dt);
		if(s->empty()){
			it1 = m_SoundQueues.erase(it1);
		}
		else{
			++it1;
		}
	}
}
void Sound::Detail::SoundManagement::addSoundDataFromFile(string file,bool music){
    if(!m_SoundData.count(file)){
        m_SoundData.emplace(file,boost::make_shared<SoundData>(file,music));
    }
}
void Sound::Detail::SoundManagement::addSoundDataFromFile(string name,string file,bool music){
    if(!m_SoundData.count(name)){
        m_SoundData.emplace(name,boost::make_shared<SoundData>(file,music));
    }
}

class SoundData::impl{
	friend class ::Engine::Sound::Detail::SoundManagement;
    public:
        sf::SoundBuffer* m_Buffer;
        std::string m_File;
		float m_Volume;

        void _init(bool music){
            m_Buffer = nullptr;
			if(music == false)
				m_Buffer = new sf::SoundBuffer();
            m_File = "";
			m_Volume = 100;
        }
		void _destruct(){
			SAFE_DELETE(m_Buffer);
		}
		void _buildBuffer(){
			m_Buffer = new sf::SoundBuffer();
			m_Buffer->loadFromFile(m_File);
		}
        void _loadFromFile(string file,bool music){
            _init(music);
			if(music == false)
                m_Buffer->loadFromFile(file);
            m_File = file;
        }

};
SoundData::SoundData(bool music):m_i(new impl){
    m_i->_init(music);
}
SoundData::SoundData(string file,bool music):m_i(new impl){
    m_i->_loadFromFile(file,music);
}
SoundData::~SoundData(){
	m_i->_destruct();
}
sf::SoundBuffer* SoundData::getBuffer(){ return m_i->m_Buffer; }
string SoundData::getFilename(){ return m_i->m_File; }
float SoundData::getVolume(){ return m_i->m_Volume; }
void SoundData::setVolume(float v){ m_i->m_Volume = glm::clamp(v,0.0f,100.0f); }
void SoundData::buildBuffer(){
	m_i->_buildBuffer();
}


class SoundBaseClass::impl{
	friend class ::Engine::Sound::Detail::SoundManagement;
    public:
        SoundStatus::Status m_Status;
		uint m_Loops;
		uint m_CurrentLoop;

		void _init(uint& loops){
			m_Status = SoundStatus::Fresh;
			m_Loops = loops;
			m_CurrentLoop = 0;
		}
		void _destruct(){
			m_Status = SoundStatus::Stopped;
		}
};
SoundBaseClass::SoundBaseClass(uint loops):m_i(new impl){
    m_i->_init(loops);
}
SoundBaseClass::~SoundBaseClass(){
	m_i->_destruct();
}
SoundStatus::Status SoundBaseClass::status(){ return m_i->m_Status; }
void SoundBaseClass::play(uint loop){ if(loop != 1) m_i->m_Status = SoundStatus::PlayingLooped; else m_i->m_Status = SoundStatus::Playing; m_i->m_Loops = loop; }
void SoundBaseClass::play(){ if(m_i->m_Loops != 1) m_i->m_Status = SoundStatus::PlayingLooped; else m_i->m_Status = SoundStatus::Playing; }
void SoundBaseClass::pause(){ m_i->m_Status = SoundStatus::Paused; }
void SoundBaseClass::stop(){ m_i->m_Status = SoundStatus::Stopped; }
void SoundBaseClass::update(float dt){}
float SoundBaseClass::getAttenuation(){ return 0; }
glm::vec3 SoundBaseClass::getPosition(){ return glm::vec3(0); }
void SoundBaseClass::setPosition(float,float,float){}
void SoundBaseClass::setPosition(glm::vec3){}
void SoundBaseClass::setVolume(float v){}
float SoundBaseClass::getVolume(){ return 0; }
uint SoundBaseClass::getLoopsLeft(){ return m_i->m_Loops - m_i->m_CurrentLoop; }
void SoundBaseClass::restart(){}
float SoundBaseClass::getPitch(){ return 0; }
void SoundBaseClass::setPitch(float p){}


class SoundEffect::impl{
	friend class ::Engine::Sound::Detail::SoundManagement;
    public:	
        sf::Sound m_Sound;

        void _init(SoundBaseClass* s,string file,bool queue){
            if(file != "") _loadFromFile(s,file);
			if(queue == false){ 
				Sound::Detail::SoundManagement::m_CurrentlyPlayingSounds.push_back( boost::shared_ptr<SoundBaseClass>(s) );
				s->play();
			}
        }
        void _init(SoundBaseClass* s,SoundData* buffer,bool queue){
			m_Sound.setBuffer( *(buffer->getBuffer()) );
			if(queue == false){
				Sound::Detail::SoundManagement::m_CurrentlyPlayingSounds.push_back( boost::shared_ptr<SoundBaseClass>(s) );
                s->play();
			}
        }
        void _loadFromFile(SoundBaseClass* s,string& file){
            if(!Sound::Detail::SoundManagement::m_SoundData.count(file)){
                Sound::Detail::SoundManagement::addSoundDataFromFile(file,file,false);
            }
			SoundData* data = (Sound::Detail::SoundManagement::m_SoundData.at(file).get());
			if(data->getBuffer() == nullptr){
				data->buildBuffer();
			}
            m_Sound.setBuffer( *(data->getBuffer()) );
			s->setVolume( data->getVolume() );
        }
        void _update(float dt,SoundBaseClass* super){
			Sound::Detail::SoundManagement::_updateSoundStatus(super,super->status(),m_Sound.getStatus());
        }
		void _play(SoundBaseClass* super){
			m_Sound.play();
		}
		void _pause(SoundBaseClass* super){
			m_Sound.pause();
		}
		void _stop(SoundBaseClass* super){
			m_Sound.stop();
		}
};
SoundEffect::SoundEffect(string file,uint loops,bool queue):SoundBaseClass(loops),m_i(new impl){
    m_i->_init(this,file,queue);
}
SoundEffect::SoundEffect(SoundData* buffer,uint loops,bool queue):SoundBaseClass(loops),m_i(new impl){
    m_i->_init(this,buffer,queue);
}
SoundEffect::~SoundEffect(){
}
void SoundEffect::loadFromFile(string file){
    m_i->_loadFromFile(this,file);
}
void SoundEffect::play(uint loop){
	SoundBaseClass::play(loop);
	m_i->_play(this);
}
void SoundEffect::play(){
	SoundBaseClass::play();
	m_i->_play(this);
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
void SoundEffect::restart(){ 
	m_i->m_Sound.setPlayingOffset(sf::Time::Zero); 
}
void SoundEffect::update(float dt){
    m_i->_update(dt,this);
}
glm::vec3 SoundEffect::getPosition(){
	sf::Vector3f v = m_i->m_Sound.getPosition();
	return glm::vec3(v.x,v.y,v.z);
}
float SoundEffect::getAttenuation(){
	return m_i->m_Sound.getAttenuation();
}
void SoundEffect::setPosition(float x,float y,float z){
	m_i->m_Sound.setPosition(x,y,z);
}
void SoundEffect::setPosition(glm::vec3 pos){
	m_i->m_Sound.setPosition(pos.x,pos.y,pos.z);
}
void SoundEffect::setVolume(float v){ m_i->m_Sound.setVolume(v); }
float SoundEffect::getVolume(){ return m_i->m_Sound.getVolume(); }
float SoundEffect::getPitch(){ return m_i->m_Sound.getPitch(); }
void SoundEffect::setPitch(float p){ m_i->m_Sound.setPitch(p); }


class SoundMusic::impl{
	friend class ::Engine::Sound::Detail::SoundManagement;
    public:
        sf::Music m_Sound;
        std::string m_File;

        void _init(SoundBaseClass* s,string file,bool queue){
            m_File = "";
            if(file != "") _loadFromFile(s,file);
			if(queue == false){ 
				Sound::Detail::SoundManagement::m_CurrentlyPlayingSounds.push_back( boost::shared_ptr<SoundBaseClass>(s) );
                s->play();
			}
        }
        void _loadFromFile(SoundBaseClass* s,string& file){
            if (!m_Sound.openFromFile(file)){
                // error...
            }
            else{
                //good
				if(!Sound::Detail::SoundManagement::m_SoundData.count(file)){
					Sound::Detail::SoundManagement::addSoundDataFromFile(file,file,true);
				}
				SoundData* data = (Sound::Detail::SoundManagement::m_SoundData.at(file).get());
				s->setVolume( data->getVolume() );
                m_File = file;
            }
        }
        void _update(float dt,SoundBaseClass* super){
			Sound::Detail::SoundManagement::_updateSoundStatus(super,super->status(),m_Sound.getStatus());
        }
		void _play(SoundBaseClass* super){
			m_Sound.play();
		}
		void _pause(SoundBaseClass* super){
			m_Sound.pause();
		}
		void _stop(SoundBaseClass* super){
			m_Sound.stop();
		}
};
SoundMusic::SoundMusic(string file,uint loops,bool queue):SoundBaseClass(loops),m_i(new impl){
    m_i->_init(this,file,queue);
}
SoundMusic::~SoundMusic(){
}
void SoundMusic::loadFromFile(string file){
    m_i->_loadFromFile(this,file);
}
void SoundMusic::play(uint loop){
	SoundBaseClass::play(loop);
	m_i->_play(this);
}
void SoundMusic::play(){
	SoundBaseClass::play();
	m_i->_play(this);
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
void SoundMusic::restart(){ 
	m_i->m_Sound.setPlayingOffset(sf::Time::Zero);
}
void SoundMusic::update(float dt){
    m_i->_update(dt,this);
}
glm::vec3 SoundMusic::getPosition(){
	sf::Vector3f v = m_i->m_Sound.getPosition();
	return glm::vec3(v.x,v.y,v.z);
}
float SoundMusic::getAttenuation(){
	return m_i->m_Sound.getAttenuation();
}
void SoundMusic::setPosition(float x,float y,float z){
	m_i->m_Sound.setPosition(x,y,z);
}
void SoundMusic::setPosition(glm::vec3 pos){
	m_i->m_Sound.setPosition(pos.x,pos.y,pos.z);
}
void SoundMusic::setVolume(float v){ m_i->m_Sound.setVolume(v); }
float SoundMusic::getVolume(){ return m_i->m_Sound.getVolume(); }
float SoundMusic::getPitch(){ return m_i->m_Sound.getPitch(); }
void SoundMusic::setPitch(float p){ m_i->m_Sound.setPitch(p); }

SoundData* Engine::Sound::getSound(string nameOrFile){
    if(Sound::Detail::SoundManagement::m_SoundData.count(nameOrFile)){
		return Sound::Detail::SoundManagement::m_SoundData.at(nameOrFile).get();
    }
	return nullptr;
}
void Engine::Sound::addSound(string file,string name){
	if (name == ""){
		if(!Sound::Detail::SoundManagement::m_SoundData.count(file)){
			Sound::Detail::SoundManagement::addSoundDataFromFile(file);
		}
	}
	else{
		if(!Sound::Detail::SoundManagement::m_SoundData.count(name)){
			Sound::Detail::SoundManagement::addSoundDataFromFile(name,file);
		}
	}
}
void Engine::Sound::playEffect(string nameOrFile,uint loops){
	SoundEffect* e = new SoundEffect(nameOrFile,loops,false);
}
void Engine::Sound::playMusic(string nameOrFile,uint loops){
	SoundMusic* e = new SoundMusic(nameOrFile,loops,false);
}


void Sound::Detail::SoundManagement::_updateSoundStatus(SoundBaseClass* sound,SoundStatus::Status status,sf::SoundSource::Status sfStatus){
	if(sfStatus == sf::SoundSource::Status::Stopped){
		if(sound->m_i->m_Loops != 1 && sound->m_i->m_Loops != 0){//handle the looping logic
			if(sound->getLoopsLeft() >= 2){
				sound->m_i->m_CurrentLoop++;
				sound->play(sound->m_i->m_Loops); //apparently playing the sound when it is stopped restarts it (sfml internally)
				//sound->restart();
			}
			else{
				sound->stop();
			}
		}
		else if(sound->m_i->m_Loops == 1){//only once
			sound->stop();
		}
		else{//endless loop (sound will have to be stoped manually by the user to end an endless loop)
			sound->play(sound->m_i->m_Loops); //apparently playing the sound when it is stopped restarts it (sfml internally)
		}
	}
}