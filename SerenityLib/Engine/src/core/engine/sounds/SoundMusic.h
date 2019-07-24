#pragma once
#ifndef ENGINE_SOUNDS_MUSIC_H
#define ENGINE_SOUNDS_MUSIC_H

#include <core/engine/sounds/SoundBaseClass.h>
#include <SFML/Audio.hpp>

class Handle;
class SoundQueue;
class SoundMusic : public SoundBaseClass {
    friend class Engine::epriv::SoundManager;
    friend class SoundQueue;
    private:
        sf::Music   m_Sound;
        float       m_Duration;
        bool        m_Active;

        SoundMusic();
    public:
        ~SoundMusic();

        void update(const double& dt);
        const bool play(const uint& numLoops = 1);
        const bool pause();
        const bool stop(const bool& stopAllLoops = false);
        const bool restart();
        const float& getDuration() const;

        const float& getAttenuation() const;
        const glm::vec3& getPosition();
        void setPosition(const float& x, const float& y, const float& z);
        void setPosition(const glm::vec3& position);
        const float& getVolume() const;
        void setVolume(const float& volume);
        const float& getPitch() const;
        void setPitch(const float& pitch);
};
#endif