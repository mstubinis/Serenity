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

		void enqueueEffect(std::string,uint loops = 1);
		void enqueueMusic(std::string,uint loops = 1);
		void dequeue();
		void update(float dt);
		void clear();
		bool empty();
};

#endif