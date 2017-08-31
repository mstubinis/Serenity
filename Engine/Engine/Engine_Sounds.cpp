#include "Engine_Sounds.h"
#include "Engine_Resources.h"

using namespace Engine;
using namespace std;

sf::SoundBuffer* Sound::Detail::SoundManagement::m_Buffer = NULL;

void Sound::Detail::SoundManagement::init(){
    m_Buffer = new sf::SoundBuffer();
}
void Sound::Detail::SoundManagement::destruct(){
    SAFE_DELETE(m_Buffer);
}
bool Sound::Detail::SoundManagement::isStopped(sf::SoundSource::Status status){
    if(status == sf::SoundSource::Status::Stopped) return true; return false;
}
bool Sound::Detail::SoundManagement::isPlaying(sf::SoundSource::Status status){
    if(status == sf::SoundSource::Status::Playing) return true; return false;
}
bool Sound::Detail::SoundManagement::isPaused(sf::SoundSource::Status status){
    if(status == sf::SoundSource::Status::Paused) return true; return false;
}
SoundEffectBasic::SoundEffectBasic(string& name,string file):EngineResource(name){ 
    s = nullptr;
}
SoundEffectBasic::~SoundEffectBasic(){
    SAFE_DELETE(s);
}
glm::vec3 SoundEffectBasic::getPosition(){ sf::Vector3f v = s->getPosition(); return glm::vec3(v.x,v.y,v.z); }
float SoundEffectBasic::getAttenuation(){ return s->getAttenuation(); }
float SoundEffectBasic::getPitch(){ return s->getPitch(); }
float SoundEffectBasic::getVolume(){ return s->getVolume(); }
float SoundEffectBasic::getMinDistance(){ return s->getMinDistance(); }
void SoundEffectBasic::setPosition(float x,float y,float z){ s->setPosition(x,y,z); }
void SoundEffectBasic::setPosition(glm::vec3 pos){ SoundEffectBasic::setPosition(pos.x,pos.y,pos.z); }
void SoundEffectBasic::setAttenuation(float a){ s->setAttenuation(a); }
void SoundEffectBasic::setVolume(float volume){ s->setVolume(volume); }
void SoundEffectBasic::setRelativeToListener(bool b){ s->setRelativeToListener(b); }
void SoundEffectBasic::setMinDistance(float d){ s->setMinDistance(d); }
SoundEffect::SoundEffect(string& name,string file,bool loop, glm::vec3 sourceOrigin):SoundEffectBasic(name,file){
    Sound::Detail::SoundManagement::m_Buffer->loadFromFile(file);
    s = new sf::Sound(*Sound::Detail::SoundManagement::m_Buffer);
    static_cast<sf::Sound*>(s)->setLoop(loop);
    static_cast<sf::Sound*>(s)->setPosition(sf::Vector3f(sourceOrigin.x,sourceOrigin.y,sourceOrigin.z));
}
SoundEffect::~SoundEffect(){
}
void SoundEffect::play(){ static_cast<sf::Sound*>(s)->play(); }
void SoundEffect::pause(){ static_cast<sf::Sound*>(s)->pause(); }
void SoundEffect::playAt(float seconds){ static_cast<sf::Sound*>(s)->setPlayingOffset(sf::seconds(seconds)); }
void SoundEffect::stop(){ static_cast<sf::Sound*>(s)->stop(); }
void SoundEffect::loop(bool loop){ static_cast<sf::Sound*>(s)->setLoop(loop); }
sf::SoundSource::Status SoundEffect::getStatus(){ return static_cast<sf::Sound*>(s)->getStatus(); }
bool SoundEffect::isStopped(){ return Sound::Detail::SoundManagement::isStopped(this->getStatus()); }
bool SoundEffect::isPlaying(){ return Sound::Detail::SoundManagement::isPlaying(this->getStatus()); }
bool SoundEffect::isPaused(){ return Sound::Detail::SoundManagement::isPaused(this->getStatus()); }
bool SoundEffect::isLooping(){ return static_cast<sf::Sound*>(s)->getLoop(); }
SoundMusic::SoundMusic(string& name,string file,bool loop):SoundEffectBasic(name,file){
    s = new sf::Music();
    static_cast<sf::Music*>(s)->openFromFile(file);
    static_cast<sf::Music*>(s)->setLoop(loop);
}
SoundMusic::~SoundMusic(){
}
void SoundMusic::play(){ static_cast<sf::Music*>(s)->play(); }
void SoundMusic::pause(){ static_cast<sf::Music*>(s)->pause(); }
void SoundMusic::playAt(float seconds){ static_cast<sf::Music*>(s)->setPlayingOffset(sf::seconds(seconds)); }
void SoundMusic::stop(){ static_cast<sf::Music*>(s)->stop(); }
void SoundMusic::loop(bool doLoop){ static_cast<sf::Music*>(s)->setLoop(doLoop); }
sf::SoundSource::Status SoundMusic::getStatus(){ return static_cast<sf::Music*>(s)->getStatus(); }
bool SoundMusic::isStopped(){ return Sound::Detail::SoundManagement::isStopped(this->getStatus()); }
bool SoundMusic::isPlaying(){ return Sound::Detail::SoundManagement::isPlaying(this->getStatus()); }
bool SoundMusic::isPaused(){ return Sound::Detail::SoundManagement::isPaused(this->getStatus()); }
bool SoundMusic::isLooping(){ return static_cast<sf::Music*>(s)->getLoop(); }
unsigned int SoundMusic::getChannelCount(){ return static_cast<sf::Music*>(s)->getChannelCount(); }
bool SoundMusic::isMono(){ if(getChannelCount() == 1) return true; return false; }
bool SoundMusic::isStereo(){ if(getChannelCount() == 2) return true; return false; }
void Sound::stop(string music){
    Engine::Resources::Detail::ResourceManagement::m_Sounds[music].get()->stop();
}
void Sound::play(string music, bool loop){
    Engine::Resources::Detail::ResourceManagement::m_Sounds[music].get()->play();
    Engine::Resources::Detail::ResourceManagement::m_Sounds[music].get()->loop(loop);
}
void Sound::playAt(string music, float seconds,bool loop){
    Engine::Resources::Detail::ResourceManagement::m_Sounds[music].get()->playAt(seconds);
    Engine::Resources::Detail::ResourceManagement::m_Sounds[music].get()->loop(loop);
}
void Sound::loop(string music, bool loop){
    Engine::Resources::Detail::ResourceManagement::m_Sounds[music].get()->loop(loop);
}
void Sound::setCurrentMusicAndPlay(string music){
    for(auto sound:Engine::Resources::Detail::ResourceManagement::m_Sounds){
        SoundMusic* music = dynamic_cast<SoundMusic*>(sound.second.get());
        if(music != NULL){
            if(music->isPlaying()){
                music->stop();
            }
        }
    }
    dynamic_cast<SoundMusic*>(Engine::Resources::Detail::ResourceManagement::m_Sounds[music].get())->play();
}
void Sound::setMasterMusicVolume(float volume){
    for(auto sound:Engine::Resources::Detail::ResourceManagement::m_Sounds){
        SoundMusic* music = dynamic_cast<SoundMusic*>(sound.second.get());
        if(music != NULL){
            music->setVolume(volume);
        }
    }
}
void Sound::setMasterEffectVolume(float volume){
    for(auto sound:Engine::Resources::Detail::ResourceManagement::m_Sounds){
        SoundEffect* effect = dynamic_cast<SoundEffect*>(sound.second.get());
        if(effect != NULL){
            effect->setVolume(volume);
        }
    }
}
