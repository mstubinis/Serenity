#pragma once
#ifndef ENGINE_SOUND_EFFECT_H
#define ENGINE_SOUND_EFFECT_H

#include <core/engine/sounds/SoundBaseClass.h>
#include <SFML/Audio.hpp>

struct Handle;
class  SoundData;
class  SoundQueue;
class  SoundEffect : public SoundBaseClass {
    friend class Engine::epriv::SoundManager;
    friend class SoundQueue;
    private:
        sf::Sound   m_Sound;
        float       m_Duration;
        bool        m_Active;

        SoundEffect();
    public:
        SoundEffect& operator=(const SoundEffect&) = delete;
        SoundEffect(const SoundEffect&) = default;
        SoundEffect(SoundEffect&&) noexcept = default;
        ~SoundEffect();

        void update(const double& dt);
        const bool play(const uint& numLoops = 1);
        const bool pause();
        const bool stop(const bool& stopAllLoops = false);
        const bool restart();
        const float getDuration() const;
        const uint getChannelCount();
        const float getMinDistance();
        void setMinDistance(const float& minDistance);
        const bool isRelativeToListener();
        void setRelativeToListener(const bool relative = true);

        const float getAttenuation() const;
        void setAttenuation(const float& attenuation);
        const glm::vec3 getPosition();
        void setPosition(const float&, const float&, const float&);
        void setPosition(const glm::vec3&);
        const float getVolume() const;
        void setVolume(const float&);
        const float getPitch() const;
        void setPitch(const float&);
};

#endif