#pragma once
#ifndef ENGINE_SOUNDS_MUSIC_H
#define ENGINE_SOUNDS_MUSIC_H

class  Handle;
class  SoundQueue;
namespace Engine::priv {
    class SoundModule;
}

#include <serenity/resources/sound/SoundIncludes.h>
#include <SFML/Audio.hpp>

class SoundMusic {
    friend class Engine::priv::SoundModule;
    friend class SoundQueue;
    private:
        sf::Music   m_Sound;
        SoundStatus m_Status      = SoundStatus::Fresh;
        uint32_t    m_Loops       = 0U;
        uint32_t    m_CurrentLoop = 0U;
        float       m_Duration    = 0.0f;
        bool        m_Active      = false;
    public:
        SoundMusic();
        SoundMusic(const SoundMusic& other)                = delete;
        SoundMusic& operator=(const SoundMusic& other)     = delete;
        SoundMusic(SoundMusic&& other) noexcept            = default;
        SoundMusic& operator=(SoundMusic&& other) noexcept = default;
        virtual ~SoundMusic()                              = default;

        [[nodiscard]] inline constexpr SoundStatus status() const noexcept { return m_Status; }
        [[nodiscard]] inline constexpr uint32_t getLoopsLeft() const noexcept { return m_Loops - m_CurrentLoop; }
        [[nodiscard]] inline constexpr bool isActive() const noexcept { return m_Active; }
        bool play(unsigned int numLoops = 1);
        bool pause();
        bool stop(bool stopAllLoops = false);
        bool restart();
        [[nodiscard]] inline constexpr float getDuration() const noexcept { return m_Duration; }
        [[nodiscard]] unsigned int getChannelCount() const;
        [[nodiscard]] float getMinDistance() const;
        void setMinDistance(float minDistance);
        [[nodiscard]] bool isRelativeToListener() const;
        void setRelativeToListener(bool relative = true);
        [[nodiscard]] float getAttenuation() const;
        void setAttenuation(float attenuation);
        [[nodiscard]] glm::vec3 getPosition() const;
        void setPosition(float x, float y, float z);
        void setPosition(const glm::vec3& position);
        void translate(float x, float y, float z);
        void translate(const glm::vec3& position);
        [[nodiscard]] float getVolume() const;
        void setVolume(float volume);
        [[nodiscard]] float getPitch() const;
        void setPitch(float pitch);

        void update(const float dt);
};
#endif