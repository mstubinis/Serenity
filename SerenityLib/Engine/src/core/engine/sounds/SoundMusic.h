#pragma once
#ifndef ENGINE_SOUNDS_MUSIC_H
#define ENGINE_SOUNDS_MUSIC_H

class  Handle;
class  SoundQueue;

#include <core/engine/sounds/SoundBaseClass.h>
#include <SFML/Audio.hpp>

class  SoundMusic : public SoundBaseClass {
    friend class Engine::priv::SoundManager;
    friend class SoundQueue;
    private:
        sf::Music   m_Sound;
        float       m_Duration = 0.0f;
        bool        m_Active   = false;
    public:
        SoundMusic();

        SoundMusic(const SoundMusic& other)                = delete;
        SoundMusic& operator=(const SoundMusic& other)     = delete;
        SoundMusic(SoundMusic&& other) noexcept            = default;
        SoundMusic& operator=(SoundMusic&& other) noexcept = default;

        ~SoundMusic();

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
        void setPosition(const float x, const float y, const float z);
        void setPosition(const glm::vec3& position);
        float getVolume() const;
        void setVolume(const float volume);
        float getPitch() const;
        void setPitch(const float pitch);
};
#endif