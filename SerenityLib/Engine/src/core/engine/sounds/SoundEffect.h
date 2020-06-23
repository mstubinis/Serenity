#pragma once
#ifndef ENGINE_SOUND_EFFECT_H
#define ENGINE_SOUND_EFFECT_H

class  Handle;
class  SoundData;
class  SoundQueue;

#include <core/engine/sounds/SoundBaseClass.h>
#include <SFML/Audio.hpp>

class  SoundEffect : public SoundBaseClass {
    friend class Engine::priv::SoundModule;
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

        constexpr const bool isActive() const noexcept { return m_Active; }
        bool play(unsigned int numLoops = 1) override;
        bool pause() override;
        bool stop(bool stopAllLoops = false);
        bool restart() override;
        constexpr const float getDuration() const noexcept { return m_Duration; }
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