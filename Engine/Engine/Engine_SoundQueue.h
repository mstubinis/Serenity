#pragma once
#ifndef ENGINE_SOUNDQUEUE_H
#define ENGINE_SOUNDQUEUE_H

#include <string>

typedef unsigned int uint;

class SoundQueue final{
    private:
		class impl; impl* m_i;
    public:
		SoundQueue(float delay = 0.5f);
		~SoundQueue();

		void enqueueEffect(std::string);
		void enqueueMusic(std::string);
		void dequeue();
		void update(float dt);
		void clear();
		bool empty();
};

#endif