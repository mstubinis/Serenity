#pragma once
#ifndef ENGINE_SOUND_EFFECT_H
#define ENGINE_SOUND_EFFECT_H

class  Handle;
class  SoundData;
class  SoundQueue;

#include <core/engine/sounds/SoundBaseClass.h>
#include <SFML/Audio.hpp>

class  SoundEffect : public SoundBaseClass {
    friend class Engine::priv::SoundManager;
    friend class SoundQueue;
    private:
        sf::Sound   m_Sound;
        float       m_Duration = 0.0f;
        bool        m_Active   = false;

    public:
        SoundEffect();

        SoundEffect(const SoundEffect& other)                = delete;
        SoundEffect& operator=(const SoundEffect& other)     = delete;
        SoundEffect(SoundEffect&& other) noexcept            = default;
        SoundEffect& operator=(SoundEffect&& other) noexcept = default;

        ~SoundEffect();

        void update(const float dt);
        bool play(const unsigned int numLoops = 1);
        bool pause();
        bool stop(const bool stopAllLoops = false);
        bool restart();
        float getDuration() const;
        unsigned int getChannelCount();
        float getMinDistance();
        void setMinDistance(const float minDistance);
        bool isRelativeToListener();
        void setRelativeToListener(const bool relative = true);

        float getAttenuation() const;
        void setAttenuation(const float attenuation);
        glm::vec3 getPosition();
        void setPosition(const float, const float, const float);
        void setPosition(const glm::vec3&);
        float getVolume() const;
        void setVolume(const float);
        float getPitch() const;
        void setPitch(const float);
};
#endif