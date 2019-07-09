#pragma once
#ifndef ENGINE_ENGINE_SOUNDS_H
#define ENGINE_ENGINE_SOUNDS_H

#include <core/engine/resources/Engine_Resources.h>
#include <SFML/Audio.hpp>
#include <glm/vec3.hpp>
#include <memory>

class SoundBaseClass;
class SoundEffect;
class SoundMusic;
class SoundQueue;
namespace Engine{
namespace epriv{
    class SoundManager;
};
};

struct SoundStatus final{enum Status{
    Playing,PlayingLooped,Paused,Stopped,Fresh
};};

class SoundData final: public EngineResource{
    friend class ::Engine::epriv::SoundManager;
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
    friend class ::Engine::epriv::SoundManager;
    private:
        class impl; std::unique_ptr<impl> m_i;
    public:
        SoundBaseClass(uint loops);
        virtual ~SoundBaseClass();

        SoundStatus::Status status();
        virtual void update(const double& dt);
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
    friend class ::Engine::epriv::SoundManager;
    private:
        class impl; std::unique_ptr<impl> m_i;
    public:
        SoundEffect(Handle&,uint loops = 1,bool = false);
        SoundEffect(SoundData*,uint loops = 1,bool = false);
        ~SoundEffect();

        void update(const double& dt);
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
    friend class ::Engine::epriv::SoundManager;
    private:
        class impl; std::unique_ptr<impl> m_i;
    public:
        SoundMusic(Handle&,uint loops = 1,bool = false);
        ~SoundMusic();

        void update(const double& dt);
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
    friend class ::Engine::epriv::SoundManager;
    private:
        class impl; std::unique_ptr<impl> m_i;
    public:
        SoundQueue(float delay = 0.5f);
        ~SoundQueue();

        void enqueueEffect(Handle&,uint loops = 1);
        void enqueueMusic(Handle&,uint loops = 1);
        void dequeue();
        void update(const double& dt);
        void clear();
        bool empty();
};

namespace Engine{
namespace epriv{
    class SoundManager final{
        friend class ::SoundBaseClass;
        public:
            class impl; std::unique_ptr<impl> m_i;

            SoundManager(const char* name,uint w,uint h);
            ~SoundManager();

            void _update(const double& dt);
    };
};
namespace Sound{
    void playEffect(Handle&,uint loops = 1);
    void playMusic(Handle&,uint loops = 1);
};
};
#endif