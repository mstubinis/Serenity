#ifndef ENGINE_ENGINE_SOUNDS_H
#define ENGINE_ENGINE_SOUNDS_H

#include <SFML/Audio.hpp>


class SoundEffect{
	private:
		sf::Sound* m_Sound;
	public:
		SoundEffect(std::string file);
		~SoundEffect();

		void play();
		void pause();
		void playAt(float seconds);
		void stop();
};

namespace Engine{
	namespace Sound{
		namespace Detail{
			class SoundManagement final{
				public:
					static sf::SoundBuffer* m_Buffer;

					static void init();
					static void destruct();
			};
		};
		const static sf::SoundBuffer* getBuffer(){ return Detail::SoundManagement::m_Buffer; }
	};
};

#endif