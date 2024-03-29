#pragma once
#ifndef ENGINE_SOUND_MODULE_H
#define ENGINE_SOUND_MODULE_H

class  SoundQueue;
class  SoundData;
class  Handle;
class  Scene;

#include <serenity/resources/sound/SoundMusic.h>
#include <serenity/resources/sound/SoundEffect.h>
#include <serenity/containers/Freelist.h>
#include <serenity/system/TypeDefs.h>
#include <vector>
#include <stack>
#include <array>

constexpr uint32_t MAX_SOUND_EFFECTS = 128;
constexpr uint32_t MAX_SOUND_MUSIC   = 6;

namespace Engine::priv {
    class SoundModule final {
        friend class ::SoundQueue;
        private:
            std::vector<uint32_t>                      m_FreelistEffects;
            std::vector<uint32_t>                      m_FreelistMusics;
            std::vector<std::unique_ptr<SoundQueue>>   m_SoundQueues;

            void updateSoundQueues(Scene&, const float dt);
            void updateSoundEffects(Scene&, const float dt);
            void updateSoundMusic(Scene&, const float dt);
        public:
            std::array<SoundEffect, MAX_SOUND_EFFECTS>  m_SoundEffects;
            std::array<SoundMusic, MAX_SOUND_MUSIC>     m_SoundMusics;

            SoundModule();
            ~SoundModule();

            void updateCameraPosition(Scene&);
            void update(Scene&, const float dt);

            SoundEffect* getNextFreeEffect();
            SoundMusic*  getNextFreeMusic();

            void setSoundInformation(Handle soundHandle, SoundEffect&);
            void setSoundInformation(Handle soundHandle, SoundMusic&);
    };
};
namespace Engine::Sound {
    SoundQueue*  createQueue(float delay);
    SoundEffect* playEffect(Handle soundHandle, uint32_t numLoops = 1);
    SoundMusic*  playMusic(Handle soundHandle, uint32_t numLoops = 1);

    void stop_all_music();
    void stop_all_effects();

    std::array<SoundEffect, MAX_SOUND_EFFECTS>& getAllSoundEffects();
    std::array<SoundMusic, MAX_SOUND_MUSIC>& getAllSoundMusics();
};
namespace Engine::lua::sound {

    Engine::priv::SoundEffectLUABinder playEffect(Handle soundHandle, luabridge::LuaRef numLoops); //uint32_t numLoops = 1
    Engine::priv::SoundMusicLUABinder playMusic(Handle soundHandle, luabridge::LuaRef numLoops); //uint32_t numLoops = 1

}

#endif