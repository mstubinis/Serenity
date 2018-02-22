#pragma once
#ifndef ENGINE_ENGINE_SOUNDS_H
#define ENGINE_ENGINE_SOUNDS_H

#include "Engine_ResourceBasic.h"
#include <SFML/Audio.hpp>
#include <glm/vec3.hpp>
#include <memory>

typedef unsigned int uint;

class SoundStatus final{public: enum Status{
    Playing,PlayingLooped,Paused,Stopped,Fresh
};};

class SoundData final: public EngineResource{
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
    friend class ::Engine::impl::SoundManager;
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
class SoundQueue final{
    private:
        class impl; std::unique_ptr<impl> m_i;
    public:
        SoundQueue(float delay = 0.5f);
        ~SoundQueue();

        void enqueueEffect(std::string,uint loops = 1);
        void enqueueMusic(std::string,uint loops = 1);
        void dequeue();
        void update(float dt);
        void clear();
        bool empty();
};

namespace Engine{
	namespace impl{
		class SoundManager final{
            friend class ::SoundBaseClass;  friend class ::SoundBaseClass::impl;
		    private:
				class impl;
		    public:
				std::unique_ptr<impl> m_i;

				SoundManager();
				~SoundManager();

				void _update(float dt);
		};
	};
    namespace Sound{
        void playEffect(std::string,uint loops = 1);
        void playMusic(std::string,uint loops = 1);
    };
};
#endif