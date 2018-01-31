#pragma once
#ifndef ENGINE_ENGINE_SOUNDS_H
#define ENGINE_ENGINE_SOUNDS_H

#include <SFML/Audio.hpp>
#include <glm/vec3.hpp>
#include <string>
#include <unordered_map>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

class SoundQueue;
class SoundStatus final{public: enum Status{
    Playing,PlayingLooped,Paused,Stopped,Fresh
};};

class SoundData final{
    private:
        class impl; impl* m_i;
    public:
        SoundData();
        SoundData(std::string);
        ~SoundData();

        sf::SoundBuffer* getBuffer();
        std::string getFilename();
};
class SoundBaseClass{
    private:
        class impl; impl* m_i;
    public:
        SoundBaseClass(bool = true);
        virtual ~SoundBaseClass();

        SoundStatus::Status status();
        virtual void update(float dt);
        virtual void play(bool loop = false);
		virtual void loop(bool = true);
        virtual void pause();
        virtual void stop();
};
class SoundEffect: public SoundBaseClass{
    private:
        class impl; impl* m_i;
    public:
		SoundEffect(std::string = "",bool = false);
        ~SoundEffect();

        void loadFromFile(std::string);
        void update(float dt);
        void play(bool loop = false);
		void loop(bool = true);
        void pause();
        void stop();
};
class SoundMusic: public SoundBaseClass{
    private:
        class impl; impl* m_i;
    public:
		SoundMusic(std::string = "",bool = false);
        ~SoundMusic();

        void loadFromFile(std::string);
        void update(float dt);
        void play(bool loop = false);
		void loop(bool = true);
        void pause();
        void stop();
};
namespace Engine{
    namespace Sound{
        namespace Detail{
            class SoundManagement final{
                public:
					static std::vector<boost::shared_ptr<SoundBaseClass>> m_CurrentlyPlayingSounds;
					static std::vector<boost::shared_ptr<SoundQueue>> m_SoundQueues;

                    static std::unordered_map<std::string,boost::shared_ptr<SoundData>> m_SoundData;

                    static void init();
                    static void destruct();
                    static void update(float dt);

                    static void addSoundDataFromFile(std::string file);
                    static void addSoundDataFromFile(std::string name, std::string file);

            };
        };
    };
};

#endif