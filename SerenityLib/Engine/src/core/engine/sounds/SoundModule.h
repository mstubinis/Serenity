#pragma once
#ifndef ENGINE_SOUND_MODULE_H
#define ENGINE_SOUND_MODULE_H

class  SoundQueue;
class  SoundData;
class  SoundBaseClass;
class  Handle;

#include <stack>
#include <vector>
#include <core/engine/sounds/SoundMusic.h>
#include <core/engine/sounds/SoundEffect.h>
#include <core/engine/containers/Freelist.h>

constexpr unsigned int MAX_SOUND_EFFECTS = 128;
constexpr unsigned int MAX_SOUND_MUSIC   = 6;

namespace Engine::priv {
    class SoundModule final {
        friend class SoundBaseClass;
        friend class SoundQueue;
        private:
            std::stack<unsigned int>               m_FreelistEffects;
            std::stack<unsigned int>               m_FreelistMusics;
            std::vector<SoundQueue*>               m_SoundQueues;
        public:
            SoundEffect                            m_SoundEffects[MAX_SOUND_EFFECTS];
            SoundMusic                             m_SoundMusics[MAX_SOUND_MUSIC];

            SoundModule();
            ~SoundModule();

            void cleanup();

            void updateSoundQueues(const float dt);
            void updateSoundEffects(const float dt);
            void updateSoundMusic(const float dt);
            void updateCameraPosition();
            void update(const float dt);

            SoundEffect* getNextFreeEffect();
            SoundMusic*  getNextFreeMusic();

            void setSoundInformation(Handle soundHandle, SoundEffect& soundEffect);
            void setSoundInformation(Handle soundHandle, SoundMusic& soundMusic);
    };
};
namespace Engine::Sound{
    SoundQueue*  createQueue(float delay);
    SoundEffect* playEffect(Handle soundHandle, unsigned int numLoops = 1);
    SoundMusic*  playMusic(Handle soundHandle, unsigned int numLoops = 1);

    void stop_all_music();
    void stop_all_effects();
};

#endif