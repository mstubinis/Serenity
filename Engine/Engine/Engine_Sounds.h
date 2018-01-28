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


class SoundData{
	private:
		class impl; std::unique_ptr<impl> m_i;
	public:
		SoundData();
		SoundData(std::string);
		~SoundData();

		sf::SoundBuffer* getBuffer();
		std::string getFilename();
};


class SoundBaseClass{
	private:
		class impl; std::unique_ptr<impl> m_i;
	public:
		SoundBaseClass();
		~SoundBaseClass();

		sf::SoundSource* getPtr();
		void setPtr(sf::SoundSource*);
		virtual void play();
		virtual void pause();
		virtual void stop();
};
class SoundEffect: public SoundBaseClass{
	private:
		class impl; std::unique_ptr<impl> m_i;
	public:
		SoundEffect();
		~SoundEffect();

		void loadFromFile(std::string);
		void play();
		void pause();
		void stop();
};
class SoundMusic: public SoundBaseClass{
	private:
		class impl; std::unique_ptr<impl> m_i;
	public:
		SoundMusic();
		~SoundMusic();

		void loadFromFile(std::string);
		void play();
		void pause();
		void stop();
};

namespace Engine{
    namespace Sound{
        namespace Detail{
            class SoundManagement final{
                public:
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