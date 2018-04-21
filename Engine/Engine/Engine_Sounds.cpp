#include "Engine.h"
#include "Engine_Sounds.h"
#include "Engine_Resources.h"

#include <glm/glm.hpp>

using namespace Engine;
using namespace std;

class SoundData::impl final{
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
class SoundBaseClass::impl final{
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
class SoundQueue::impl final{
    public:
        vector<SoundBaseClass*> m_Queue;
        float m_DelayInSeconds;
        float m_DelayTimer;
        bool m_IsDelayProcess;

        void _init(float _delay){
            m_DelayInSeconds = _delay;
            m_DelayTimer = 0;
            m_IsDelayProcess = false;
        }
        void _clear(){
            for(auto it1 = m_Queue.begin(); it1 != m_Queue.end();){
                delete (*it1);
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
                delete (*it);
                it = m_Queue.erase(it);
                m_IsDelayProcess = true;
                //do we need to manually delete? i dont think so
            }
        }
        void _update(const float& dt){
            if(m_IsDelayProcess == true){
                m_DelayTimer += dt;
                if(m_DelayTimer > m_DelayInSeconds){
                    m_IsDelayProcess = false;
                    m_DelayTimer = 0;
                }
            }
            else{
                if(m_Queue.size() > 0){
                    auto it = m_Queue.begin();
                    SoundBaseClass* s = (*it);
                    const SoundStatus::Status& stat = s->status();
                    if(stat == SoundStatus::Fresh){
                        //play it
                        s->play();
                        //s->update(dt);
                    }
                    else if(stat == SoundStatus::Playing || stat == SoundStatus::PlayingLooped){
                        s->update(dt);
                    }
                    else if(stat == SoundStatus::Stopped){
                        if(s->getLoopsLeft() <= 1){
                            //this sound has finished, remove it from the queue and start the delay process
                            delete (*it);
                            it = m_Queue.erase(it);
                            m_IsDelayProcess = true;
                        }
                    }
                }
            }
        }
};
class Engine::epriv::SoundManager::impl final{
    public:
        vector<SoundBaseClass*> m_CurrentlyPlayingSounds;
        vector<SoundQueue*> m_SoundQueues;

        void _init(const char* name,uint& w,uint& h){
        }
        void _postInit(const char* name,uint& w,uint& h){
        }
        void _destruct(){
            for(auto q:m_SoundQueues){ delete q; }
            for(auto s:m_CurrentlyPlayingSounds){ delete s; }
            vector_clear(m_SoundQueues);
            vector_clear(m_CurrentlyPlayingSounds);
        }
        void _updateSoundStatus(SoundBaseClass* sound,sf::SoundSource::Status sfStatus){
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
        void _update(const float& dt){
            for(auto it = m_CurrentlyPlayingSounds.begin(); it != m_CurrentlyPlayingSounds.end();){
                SoundBaseClass* s = (*it);
                s->update(dt);
                if(s->status() == SoundStatus::Stopped){
                    it = m_CurrentlyPlayingSounds.erase(it);
                }
                else{
                    ++it;
                }
            }
            for(auto it1 = m_SoundQueues.begin(); it1 != m_SoundQueues.end();){
                SoundQueue* s = (*it1);
                s->update(dt);
                if(s->empty()){
                    it1 = m_SoundQueues.erase(it1);
                }
                else{
                    ++it1;
                }
            }
        }
};
class SoundEffect::impl final{
    public:	
        sf::Sound m_Sound;

        void _init(SoundBaseClass* s,Handle& handle,bool queue){
            SoundData* data = Engine::Resources::getSoundData(handle);
            _init(s,data,queue);
        }
        void _init(SoundBaseClass* s,SoundData* data,bool queue){
            if(data->getBuffer() == nullptr){
                data->buildBuffer();
            }
            m_Sound.setBuffer( *(data->getBuffer()) );
            s->setVolume( data->getVolume() );

            m_Sound.setBuffer( *(data->getBuffer()) );
            if(queue == false){
                epriv::Core::m_Engine->m_SoundManager->m_i->m_CurrentlyPlayingSounds.push_back(s);
                s->play();
            }
        }
        void _update(const float& dt,SoundBaseClass* super){
            epriv::Core::m_Engine->m_SoundManager->m_i->_updateSoundStatus(super,m_Sound.getStatus());
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
class SoundMusic::impl final{
    public:
        sf::Music m_Sound;
        void _init(SoundBaseClass* s,Handle& handle,bool queue){
            SoundData* data = Resources::getSoundData(handle);
            if (!m_Sound.openFromFile(data->name())){
                // error...
            }
            else{
                s->setVolume( data->getVolume() );
            }
            if(queue == false){ 
                epriv::Core::m_Engine->m_SoundManager->m_i->m_CurrentlyPlayingSounds.push_back(s);
                s->play();
            }
        }
        void _update(const float& dt,SoundBaseClass* super){
            epriv::Core::m_Engine->m_SoundManager->m_i->_updateSoundStatus(super,m_Sound.getStatus());
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
SoundData::SoundData(bool music):m_i(new impl){ m_i->_init(music); }
SoundData::SoundData(string file,bool music):m_i(new impl){ m_i->_loadFromFile(file,music); }
SoundData::~SoundData(){ m_i->_destruct(); }
sf::SoundBuffer* SoundData::getBuffer(){ return m_i->m_Buffer; }
string SoundData::getFilename(){ return m_i->m_File; }
float SoundData::getVolume(){ return m_i->m_Volume; }
void SoundData::setVolume(float v){ m_i->m_Volume = glm::clamp(v,0.0f,100.0f); }
void SoundData::buildBuffer(){ m_i->_buildBuffer(); }


SoundBaseClass::SoundBaseClass(uint loops):m_i(new impl){ m_i->_init(loops); }
SoundBaseClass::~SoundBaseClass(){ m_i->_destruct(); }
SoundStatus::Status SoundBaseClass::status(){ return m_i->m_Status; }
void SoundBaseClass::play(uint loop){ if(loop != 1) m_i->m_Status = SoundStatus::PlayingLooped; else m_i->m_Status = SoundStatus::Playing; m_i->m_Loops = loop; }
void SoundBaseClass::play(){ if(m_i->m_Loops != 1) m_i->m_Status = SoundStatus::PlayingLooped; else m_i->m_Status = SoundStatus::Playing; }
void SoundBaseClass::pause(){ m_i->m_Status = SoundStatus::Paused; }
void SoundBaseClass::stop(){ m_i->m_Status = SoundStatus::Stopped; }
void SoundBaseClass::update(const float& dt){}
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


SoundEffect::SoundEffect(Handle& handle,uint loops,bool queue):SoundBaseClass(loops),m_i(new impl){ m_i->_init(this,handle,queue); }
SoundEffect::SoundEffect(SoundData* buffer,uint loops,bool queue):SoundBaseClass(loops),m_i(new impl){ m_i->_init(this,buffer,queue); }
SoundEffect::~SoundEffect(){}
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
void SoundEffect::update(const float& dt){
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


SoundMusic::SoundMusic(Handle& handle,uint loops,bool queue):SoundBaseClass(loops),m_i(new impl){ m_i->_init(this,handle,queue); }
SoundMusic::~SoundMusic(){}
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
void SoundMusic::update(const float& dt){
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


SoundQueue::SoundQueue(float _delay):m_i(new impl){
    m_i->_init(_delay);
    epriv::Core::m_Engine->m_SoundManager->m_i->m_SoundQueues.push_back(this);
}
SoundQueue::~SoundQueue(){ m_i->_destruct(); }
void SoundQueue::enqueueEffect(Handle& handle,uint loops){ m_i->m_Queue.push_back( new SoundEffect(handle,loops,true) ); }
void SoundQueue::enqueueMusic(Handle& handle,uint loops){ m_i->m_Queue.push_back( new SoundMusic(handle,loops,true) ); }
void SoundQueue::dequeue(){ m_i->_dequeue(); }
void SoundQueue::update(const float& dt){ m_i->_update(dt); }
void SoundQueue::clear(){ m_i->_clear(); }
bool SoundQueue::empty(){ if(m_i->m_Queue.size() > 0) return false; return true; }

epriv::SoundManager::SoundManager(const char* name,uint w,uint h):m_i(new impl){ m_i->_init(name,w,h); }
epriv::SoundManager::~SoundManager(){ m_i->_destruct(); }
void epriv::SoundManager::_init(const char* name,uint w,uint h){ m_i->_postInit(name,w,h); }
void epriv::SoundManager::_update(const float& dt){ m_i->_update(dt); }

void Engine::Sound::playEffect(Handle& handle,uint loops){
    SoundEffect* e = new SoundEffect(handle,loops,false);
}
void Engine::Sound::playMusic(Handle& handle,uint loops){
    SoundMusic* e = new SoundMusic(handle,loops,false);
}