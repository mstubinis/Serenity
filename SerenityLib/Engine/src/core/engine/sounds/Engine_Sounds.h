#pragma once
#ifndef ENGINE_ENGINE_SOUNDS_H
#define ENGINE_ENGINE_SOUNDS_H

#include <core/engine/resources/Engine_Resources.h>
#include <SFML/Audio.hpp>
#include <glm/vec3.hpp>
#include <stack>

#include <core/engine/sounds/SoundBaseClass.h>
#include <core/engine/sounds/SoundMusic.h>
#include <core/engine/sounds/SoundEffect.h>

class SoundQueue;
class SoundData;
namespace Engine{
    namespace epriv{
        class SoundManager;
    };
};
namespace Engine{
    namespace epriv{
        class SoundManager final{
            friend class ::SoundBaseClass;
            public:
                static const uint MAX_SOUND_EFFECTS = 128;
                static const uint MAX_SOUND_MUSIC   = 6;

                SoundEffect                    m_SoundEffects[MAX_SOUND_EFFECTS];
                SoundMusic                     m_SoundMusics[MAX_SOUND_MUSIC];
                std::stack<uint>               m_FreelistEffects;
                std::stack<uint>               m_FreelistMusics;

                std::vector<SoundQueue*>       m_SoundQueues;

                SoundManager(const char* name,uint w,uint h);
                ~SoundManager();

                void _update(const double& dt);

                void _setSoundInformation(Handle&, SoundEffect& sound);
                void _setSoundInformation(Handle&, SoundMusic& sound);

                SoundEffect* _getFreeEffect();
                SoundMusic* _getFreeMusic();

        };
    };
    namespace Sound{
        SoundQueue* createQueue(const float& delay);
        SoundEffect* playEffect(Handle&, const uint& numLoops = 1);
        SoundMusic* playMusic(Handle&, const uint& numLoops = 1);

        void stop_all_music();
        void stop_all_effect();
    };
};
#endif