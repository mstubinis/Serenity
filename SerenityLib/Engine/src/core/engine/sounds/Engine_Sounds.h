#pragma once
#ifndef ENGINE_ENGINE_SOUNDS_H
#define ENGINE_ENGINE_SOUNDS_H

#include <core/engine/resources/Engine_Resources.h>
#include <SFML/Audio.hpp>
#include <glm/vec3.hpp>
#include <queue>

#include <core/engine/sounds/SoundBaseClass.h>

class SoundEffect;
class SoundMusic;
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
                std::vector<SoundBaseClass*>   m_CurrentlyPlayingSounds;
                std::vector<SoundQueue*>       m_SoundQueues;

                SoundManager(const char* name,uint w,uint h);
                ~SoundManager();

                void _update(const double& dt);


                void _play(SoundBaseClass& sound, const uint& numLoops = 1);
                void _pause(SoundBaseClass& sound);
                void _stop(SoundBaseClass& sound);
                void _restart(SoundBaseClass& sound);

        };
    };
    namespace Sound{
        SoundQueue* createQueue(const float& delay);
        SoundEffect* playEffect(Handle&, const uint& numLoops = 1);
        SoundMusic* playMusic(Handle&, const uint& numLoops = 1);
    };
};
#endif