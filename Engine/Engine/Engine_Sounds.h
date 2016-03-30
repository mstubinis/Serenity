#ifndef ENGINE_ENGINE_SOUNDS_H
#define ENGINE_ENGINE_SOUNDS_H

#include <SFML/Audio.hpp>
#include <glm/glm.hpp>
#include "Engine_Resources.h"

class SoundEffectBasic{
	protected:
		sf::SoundSource* s;
	public:
		SoundEffectBasic(std::string file);
		virtual ~SoundEffectBasic();

		glm::vec3 getPosition();
		float getAttenuation();
		float getPitch();
		float getVolume();
		float getMinDistance();

		virtual void play(){}
		virtual void pause(){}
		virtual void playAt(float seconds){}
		virtual void stop(){}
		virtual void loop(bool = true){}

		virtual void setPosition(float,float,float);
		virtual void setPosition(glm::vec3);
		virtual void setVolume(float);
		virtual void setAttenuation(float);
		virtual void setRelativeToListener(bool);
		virtual void setMinDistance(float);

		virtual bool isStopped(){ return false; }
		virtual bool isPlaying(){ return false; }
		virtual bool isPaused(){ return false; }
		virtual bool isLooping(){ return false; }

		virtual unsigned int getChannelCount(){ return 0; }
		virtual bool isMono(){ return false; }
		virtual bool isStereo(){ return false; }
};
class SoundEffect: public SoundEffectBasic{
	public:
		SoundEffect(std::string file);
		virtual ~SoundEffect();

		virtual void play();
		virtual void pause();
		virtual void playAt(float seconds);
		virtual void stop();
		virtual void loop(bool = true);

		virtual sf::SoundSource::Status getStatus();
		virtual bool isStopped();
		virtual bool isPlaying();
		virtual bool isPaused();
		virtual bool isLooping();
};
class SoundMusic: public SoundEffectBasic{
	public:
		SoundMusic(std::string file);
		virtual ~SoundMusic();

		virtual void play();
		virtual void pause();
		virtual void playAt(float seconds);
		virtual void stop();
		virtual void loop(bool = true);

		virtual sf::SoundSource::Status getStatus();
		virtual bool isStopped();
		virtual bool isPlaying();
		virtual bool isPaused();
		virtual bool isLooping();

		virtual unsigned int getChannelCount();
		virtual bool isMono();
		virtual bool isStereo();
};

namespace Engine{
	namespace Sound{
		namespace Detail{
			class SoundManagement final{
				public:
					static sf::SoundBuffer* m_Buffer;

					static void init();
					static void destruct();

					static bool isStopped(sf::SoundSource::Status);
					static bool isPlaying(sf::SoundSource::Status);
					static bool isPaused(sf::SoundSource::Status);
			};
		};
		static void loop(std::string music, bool loop = true){
			Engine::Resources::Detail::ResourceManagement::m_Sounds[music].get()->loop(loop);
		}
		static void setCurrentMusicAndPlay(std::string music){
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

		static void setMasterMusicVolume(float volume){
			for(auto sound:Engine::Resources::Detail::ResourceManagement::m_Sounds){
				SoundMusic* music = dynamic_cast<SoundMusic*>(sound.second.get());
				if(music != NULL){
					music->setVolume(volume);
				}
			}
		}
		static void setMasterEffectVolume(float volume){
			for(auto sound:Engine::Resources::Detail::ResourceManagement::m_Sounds){
				SoundEffect* effect = dynamic_cast<SoundEffect*>(sound.second.get());
				if(effect != NULL){
					effect->setVolume(volume);
				}
			}
		}
	};
};

#endif