#include "Engine_Sounds.h"
#include "Engine_Resources.h"

using namespace Engine;
using namespace std;

std::unordered_map<std::string,boost::shared_ptr<SoundData>> Sound::Detail::SoundManagement::m_SoundData;

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
SoundData::SoundData():m_i(new impl){
	m_i->init();
}
SoundData::SoundData(string file):m_i(new impl){
	m_i->loadFromFile(file);
}
SoundData::~SoundData(){

}
sf::SoundBuffer* SoundData::getBuffer(){ return m_i->m_Buffer; }
std::string SoundData::getFilename(){ return m_i->m_File; }



class SoundBaseClass::impl{
    public:
		sf::SoundSource* m_SoundSource;
};
SoundBaseClass::SoundBaseClass():m_i(new impl){}
SoundBaseClass::~SoundBaseClass(){}
sf::SoundSource* SoundBaseClass::getPtr(){ return m_i->m_SoundSource; }
void SoundBaseClass::setPtr(sf::SoundSource* s){ m_i->m_SoundSource = s; }
void SoundBaseClass::play(){}
void SoundBaseClass::pause(){}
void SoundBaseClass::stop(){}

class SoundEffect::impl{
    public:
		sf::Sound* m_SoundEffect;
};
SoundEffect::SoundEffect():SoundBaseClass(),m_i(new impl){
	m_i->m_SoundEffect = new sf::Sound();
	setPtr(m_i->m_SoundEffect);
}
SoundEffect::~SoundEffect(){
	delete m_i->m_SoundEffect;
	m_i->m_SoundEffect = nullptr;
}
void SoundEffect::loadFromFile(string file){
	if(!Sound::Detail::SoundManagement::m_SoundData.count(file)){
		Sound::Detail::SoundManagement::addSoundDataFromFile(file);
	}
	m_i->m_SoundEffect->setBuffer( *(Sound::Detail::SoundManagement::m_SoundData.at(file).get()->getBuffer()) );
}
void SoundEffect::play(){
	m_i->m_SoundEffect->play();
}
void SoundEffect::pause(){
	m_i->m_SoundEffect->pause();
}
void SoundEffect::stop(){
	m_i->m_SoundEffect->stop();
}



class SoundMusic::impl{
    public:
		sf::Music* m_SoundEffect;
};
SoundMusic::SoundMusic():SoundBaseClass(),m_i(new impl){
	m_i->m_SoundEffect = new sf::Music();
	setPtr(m_i->m_SoundEffect);
}
SoundMusic::~SoundMusic(){
	delete m_i->m_SoundEffect;
	m_i->m_SoundEffect = nullptr;
}
void SoundMusic::loadFromFile(string file){
	if (!m_i->m_SoundEffect->openFromFile(file)){
		// error...
	}
}
void SoundMusic::play(){
	m_i->m_SoundEffect->play();
}
void SoundMusic::pause(){
	m_i->m_SoundEffect->pause();
}
void SoundMusic::stop(){
	m_i->m_SoundEffect->stop();
}


void Sound::Detail::SoundManagement::init(){

}
void Sound::Detail::SoundManagement::destruct(){
	for (auto it = m_SoundData.begin();it != m_SoundData.end(); ++it ) it->second.reset();
}
void Sound::Detail::SoundManagement::update(float dt){
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