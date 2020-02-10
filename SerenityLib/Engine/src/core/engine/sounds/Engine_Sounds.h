#pragma once
#ifndef ENGINE_ENGINE_SOUNDS_H
#define ENGINE_ENGINE_SOUNDS_H

class  SoundQueue;
class  SoundData;
class  SoundBaseClass;
struct Handle;

#include <stack>
#include <vector>
#include <core/engine/sounds/SoundMusic.h>
#include <core/engine/sounds/SoundEffect.h>
#include <core/engine/containers/Freelist.h>

namespace Engine::priv {
    class SoundManager final {
        friend class SoundBaseClass;
        public:
            static const unsigned int              MAX_SOUND_EFFECTS = 128;
            static const unsigned int              MAX_SOUND_MUSIC = 6;

            SoundEffect                            m_SoundEffects[MAX_SOUND_EFFECTS];
            SoundMusic                             m_SoundMusics[MAX_SOUND_MUSIC];
            std::stack<unsigned int>               m_FreelistEffects;
            std::stack<unsigned int>               m_FreelistMusics;

            std::vector<SoundQueue*>               m_SoundQueues;

            SoundManager();
            ~SoundManager();

            void cleanup();

            void _update(const double& dt);

            SoundEffect* _getNextFreeEffect();
            SoundMusic*  _getNextFreeMusic();

            void _setSoundInformation(Handle& soundHandle, SoundEffect& soundEffect);
            void _setSoundInformation(Handle& soundHandle, SoundMusic& soundMusic);
    };
};
namespace Engine::Sound{
    SoundQueue* createQueue(const float& delay);
    SoundEffect* playEffect(Handle&, const unsigned int& numLoops = 1);
    SoundMusic* playMusic(Handle&, const unsigned int& numLoops = 1);

    void stop_all_music();
    void stop_all_effects();
};

#endif