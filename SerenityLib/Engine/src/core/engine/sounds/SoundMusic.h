#pragma once
#ifndef ENGINE_SOUNDS_MUSIC_H
#define ENGINE_SOUNDS_MUSIC_H

class  Handle;
class  SoundQueue;

#include <core/engine/sounds/SoundBaseClass.h>
#include <SFML/Audio.hpp>

class SoundMusic : public SoundBaseClass {
    friend class Engine::priv::SoundModule;
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

        virtual ~SoundMusic() = default;

        inline constexpr bool isActive() const noexcept { return m_Active; }
        bool play(unsigned int numLoops = 1) override;
        bool pause() override;
        bool stop(bool stopAllLoops = false);
        bool restart() override;
        inline constexpr float getDuration() const noexcept { return m_Duration; }
        unsigned int getChannelCount() const override;
        float getMinDistance() const override;
        void setMinDistance(float minDistance) override;
        bool isRelativeToListener() const override;
        void setRelativeToListener(bool relative = true) override;
        float getAttenuation() const override;
        void setAttenuation(float attenuation) override;
        glm::vec3 getPosition() const override;
        void setPosition(float x, float y, float z) override;
        void setPosition(const glm::vec3& position) override;
        float getVolume() const override;
        void setVolume(float volume) override;
        float getPitch() const override;
        void setPitch(float pitch) override;

        void update(const float dt) override;
};
#endif