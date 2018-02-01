#pragma once
#ifndef ENGINE_ENGINE_SOUNDS_H
#define ENGINE_ENGINE_SOUNDS_H

#include <SFML/Audio.hpp>
#include <glm/vec3.hpp>
#include <memory>
#include <string>
#include <unordered_map>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

typedef unsigned int uint;

namespace Engine{ namespace Sound{ namespace Detail{ class SoundManagement; };};};

class SoundQueue;
class SoundStatus final{public: enum Status{
    Playing,PlayingLooped,Paused,Stopped,Fresh
};};

class SoundData final{
	friend class ::Engine::Sound::Detail::SoundManagement;
    private:
        class impl; std::unique_ptr<impl> m_i;
    public:
        SoundData(bool music = false);
        SoundData(std::string,bool music = false);
        ~SoundData();

		void buildBuffer();
        sf::SoundBuffer* getBuffer();
        std::string getFilename();
		float getVolume();
		void setVolume(float);
};
class SoundBaseClass{
	friend class ::Engine::Sound::Detail::SoundManagement;
    private:
        class impl; std::unique_ptr<impl> m_i;
    public:
        SoundBaseClass(uint loops);
        virtual ~SoundBaseClass();

        SoundStatus::Status status();
        virtual void update(float dt);
        virtual void play(uint loop);
		virtual void play();
        virtual void pause();
        virtual void stop();
		virtual void restart();
		uint getLoopsLeft();

		virtual float getAttenuation();
		virtual glm::vec3 getPosition();
		virtual void setPosition(float,float,float);
		virtual void setPosition(glm::vec3);
		virtual float getVolume();
		virtual void setVolume(float);
		virtual float getPitch();
		virtual void setPitch(float);
};
class SoundEffect: public SoundBaseClass{
    friend class ::Engine::Sound::Detail::SoundManagement;
    private:
        class impl; std::unique_ptr<impl> m_i;
    public:
		SoundEffect(std::string = "",uint loops = 1,bool = false);
		SoundEffect(SoundData*,uint loops = 1,bool = false);
        ~SoundEffect();

        void loadFromFile(std::string);
        void update(float dt);
        void play(uint loop);
		void play();
        void pause();
        void stop();
		void restart();

		float getAttenuation();
		glm::vec3 getPosition();
		void setPosition(float,float,float);
		void setPosition(glm::vec3);
		float getVolume();
	    void setVolume(float);
		float getPitch();
		void setPitch(float);
};
class SoundMusic: public SoundBaseClass{
	friend class ::Engine::Sound::Detail::SoundManagement;
    private:
        class impl; std::unique_ptr<impl> m_i;
    public:
		SoundMusic(std::string = "",uint loops = 1,bool = false);
        ~SoundMusic();

        void loadFromFile(std::string);
        void update(float dt);
        void play(uint loop);
		void play();
        void pause();
        void stop();
		void restart();

		float getAttenuation();
		glm::vec3 getPosition();
		void setPosition(float,float,float);
		void setPosition(glm::vec3);
		float getVolume();
	    void setVolume(float);
		float getPitch();
		void setPitch(float);
};
namespace Engine{
    namespace Sound{
        namespace Detail{
            class SoundManagement final{
				friend class ::SoundBaseClass::impl;
				friend class ::SoundMusic::impl;
				friend class ::SoundEffect::impl;
				friend class ::SoundBaseClass;
				friend class ::SoundMusic;
				friend class ::SoundEffect;
                public:
					static void _updateSoundStatus(SoundBaseClass*,SoundStatus::Status,sf::SoundSource::Status);

					static std::vector<boost::shared_ptr<SoundBaseClass>> m_CurrentlyPlayingSounds;
					static std::vector<boost::shared_ptr<SoundQueue>> m_SoundQueues;

                    static std::unordered_map<std::string,boost::shared_ptr<SoundData>> m_SoundData;

                    static void init();
                    static void destruct();
                    static void update(float dt);

                    static void addSoundDataFromFile(std::string file,bool music = false);
                    static void addSoundDataFromFile(std::string name, std::string file,bool music = false);
            };
        };
		SoundData* getSound(std::string nameOrFile);
		void addSound(std::string file,std::string name = "");
		void playEffect(std::string,uint loops = 1);
		void playMusic(std::string,uint loops = 1);
    };
};

#endif