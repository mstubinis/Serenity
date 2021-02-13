#pragma once
#ifndef ENGINE_SOUND_EFFECT_H
#define ENGINE_SOUND_EFFECT_H

class  Handle;
class  SoundData;
class  SoundQueue;

#include <serenity/resources/sound/SoundBaseClass.h>
#include <SFML/Audio.hpp>

class SoundEffect : public SoundBaseClass {
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
        virtual ~SoundEffect()                               = default;

        [[nodiscard]] inline constexpr bool isActive() const noexcept { return m_Active; }
        bool play(unsigned int numLoops = 1) override;
        bool pause() override;
        bool stop(bool stopAllLoops = false);
        bool restart() override;
        [[nodiscard]] inline constexpr float getDuration() const noexcept { return m_Duration; }
        [[nodiscard]] unsigned int getChannelCount() const override;
        [[nodiscard]] float getMinDistance() const override;
        void setMinDistance(float minDistance) override;
        [[nodiscard]] bool isRelativeToListener() const override;
        void setRelativeToListener(bool relative = true) override;
        [[nodiscard]] float getAttenuation() const override;


        /*
        Set the attenuation factor of the sound. The attenuation is a multiplicative factor which makes
        the sound more or less loud according to its distance from the listener.An attenuation of 0 will
        produce a non - attenuated sound, i.e. its volume will always be the same whether it is heard from
        near or from far. On the other hand, an attenuation value such as 100 will make the sound fade out
        very quickly as it gets further from the listener. The default value of the attenuation is 1.
        */
        void setAttenuation(float attenuation) override;

        [[nodiscard]] glm::vec3 getPosition() const override;
        void setPosition(float x, float y, float z) override;
        void setPosition(const glm::vec3& position) override;
        [[nodiscard]] float getVolume() const override;
        void setVolume(float volume) override;
        [[nodiscard]] float getPitch() const override;
        void setPitch(float pitch) override;

        void update(const float dt) override;
};
#endif