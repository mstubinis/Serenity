#pragma once
#ifndef ENGINE_SOUND_MODULE_H
#define ENGINE_SOUND_MODULE_H

class  SoundQueue;
class  SoundData;
class  SoundBaseClass;
class  Handle;
class  Scene;

#include <serenity/core/engine/sounds/SoundMusic.h>
#include <serenity/core/engine/sounds/SoundEffect.h>
#include <serenity/core/engine/containers/Freelist.h>
#include <serenity/core/engine/system/TypeDefs.h>
#include <vector>
#include <stack>
#include <array>

constexpr uint32_t MAX_SOUND_EFFECTS = 128;
constexpr uint32_t MAX_SOUND_MUSIC   = 6;

namespace Engine::priv {
    class SoundModule final {
        friend class SoundBaseClass;
        friend class SoundQueue;
        private:
            std::stack<uint32_t>                       m_FreelistEffects;
            std::stack<uint32_t>                       m_FreelistMusics;
            std::vector<std::unique_ptr<SoundQueue>>   m_SoundQueues;

            void updateSoundQueues(Scene& scene, const float dt);
            void updateSoundEffects(Scene& scene, const float dt);
            void updateSoundMusic(Scene& scene, const float dt);
        public:
            std::array<SoundEffect, MAX_SOUND_EFFECTS>  m_SoundEffects;
            std::array<SoundMusic, MAX_SOUND_MUSIC>     m_SoundMusics;

            SoundModule();
            ~SoundModule();

            void updateCameraPosition(Scene& scene);
            void update(Scene& scene, const float dt);

            SoundEffect* getNextFreeEffect();
            SoundMusic*  getNextFreeMusic();

            void setSoundInformation(Handle soundHandle, SoundEffect& soundEffect);
            void setSoundInformation(Handle soundHandle, SoundMusic& soundMusic);
    };
};
namespace Engine::Sound{
    SoundQueue*  createQueue(float delay);
    SoundEffect* playEffect(Handle soundHandle, uint32_t numLoops = 1);
    SoundMusic*  playMusic(Handle soundHandle, uint32_t numLoops = 1);

    std::array<SoundEffect, MAX_SOUND_EFFECTS>& getAllSoundEffects();
    std::array<SoundMusic, MAX_SOUND_MUSIC>& getAllSoundMusics();

    void stop_all_music();
    void stop_all_effects();
};

#endif