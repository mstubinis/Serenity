#include "Engine_Sounds.h"
#include "Engine_Resources.h"

using namespace Engine;

sf::SoundBuffer* Sound::Detail::SoundManagement::m_Buffer = NULL;

SoundEffect::SoundEffect(std::string file){
	Sound::Detail::SoundManagement::m_Buffer->loadFromFile(file);
	m_Sound->setBuffer(*Sound::Detail::SoundManagement::m_Buffer);
}
SoundEffect::~SoundEffect(){
	SAFE_DELETE(m_Sound);
}
void SoundEffect::play(){ m_Sound->play(); }
void SoundEffect::playAt(float seconds){ m_Sound->setPlayingOffset(sf::seconds(seconds)); }
void SoundEffect::stop(){ m_Sound->stop(); }
void SoundEffect::pause(){ m_Sound->pause(); }


void Engine::Sound::Detail::SoundManagement::init(){
	m_Buffer = new sf::SoundBuffer();
}
void Engine::Sound::Detail::SoundManagement::destruct(){
	SAFE_DELETE(m_Buffer);
}