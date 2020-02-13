#pragma once
#ifndef ENGINE_SOUNDS_MUSIC_H
#define ENGINE_SOUNDS_MUSIC_H

#include <core/engine/sounds/SoundBaseClass.h>
#include <SFML/Audio.hpp>

struct Handle;
class  SoundQueue;
class  SoundMusic : public SoundBaseClass {
    friend class Engine::priv::SoundManager;
    friend class SoundQueue;
    private:
        sf::Music   m_Sound;
        float       m_Duration;
        bool        m_Active;

    public:
        SoundMusic();

        SoundMusic(const SoundMusic& other)                = delete;
        SoundMusic& operator=(const SoundMusic& other)     = delete;
        SoundMusic(SoundMusic&& other) noexcept            = default;
        SoundMusic& operator=(SoundMusic&& other) noexcept = default;


        ~SoundMusic();

        void update(const float& dt);
        const bool play(const unsigned int& numLoops = 1);
        const bool pause();
        const bool stop(const bool& stopAllLoops = false);
        const bool restart();
        const float getDuration() const;
        const unsigned int getChannelCount();
        const float getMinDistance();
        void setMinDistance(const float& minDistance);
        const bool isRelativeToListener();
        void setRelativeToListener(const bool relative = true);

        const float getAttenuation() const;
        void setAttenuation(const float& attenuation);
        const glm::vec3 getPosition();
        void setPosition(const float& x, const float& y, const float& z);
        void setPosition(const glm::vec3& position);
        const float getVolume() const;
        void setVolume(const float& volume);
        const float getPitch() const;
        void setPitch(const float& pitch);
};
#endif