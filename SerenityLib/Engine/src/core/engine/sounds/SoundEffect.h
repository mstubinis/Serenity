#pragma once
#ifndef ENGINE_SOUND_EFFECT_H
#define ENGINE_SOUND_EFFECT_H

#include <core/engine/sounds/SoundBaseClass.h>
#include <SFML/Audio.hpp>

class Handle;
class SoundData;
class SoundEffect : public SoundBaseClass {
    friend class Engine::epriv::SoundManager;
    private:
        sf::Sound   m_Sound;
        float       m_Duration;
    public:
        SoundEffect(Handle&, const uint& loops = 1);
        SoundEffect(SoundData&, const uint& loops = 1);
        ~SoundEffect();

        void update(const double& dt);
        const bool play(const uint& loop);
        const bool play();
        const bool pause();
        const bool stop();
        const bool restart();
        const float& getDuration() const;

        const float& getAttenuation() const;
        const glm::vec3& getPosition();
        void setPosition(const float&, const float&, const float&);
        void setPosition(const glm::vec3&);
        const float& getVolume() const;
        void setVolume(const float&);
        const float& getPitch() const;
        void setPitch(const float&);
};

#endif