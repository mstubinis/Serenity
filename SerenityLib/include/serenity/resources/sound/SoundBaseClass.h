#pragma once
#ifndef ENGINE_SOUND_BASE_CLASS_H
#define ENGINE_SOUND_BASE_CLASS_H

#include <serenity/system/TypeDefs.h>
#include <serenity/dependencies/glm.h>

enum class SoundStatus : uint8_t {
    Fresh,
    Playing,
    PlayingLooped,
    Paused,
    Stopped,
};

namespace Engine::priv {
    class SoundModule;
};

class SoundBaseClass {
    friend class Engine::priv::SoundModule;
    protected:
        SoundStatus m_Status      = SoundStatus::Fresh;
        uint32_t    m_Loops       = 0U;
        uint32_t    m_CurrentLoop = 0U;

        SoundBaseClass(uint32_t numLoops)
            : m_Loops{ numLoops }
        {}
    public:
        SoundBaseClass(const SoundBaseClass&)                = delete;
        SoundBaseClass& operator=(const SoundBaseClass&)     = delete;
        SoundBaseClass(SoundBaseClass&&) noexcept            = default;
        SoundBaseClass& operator=(SoundBaseClass&&) noexcept = default;
        virtual ~SoundBaseClass() = default;

        [[nodiscard]] inline constexpr SoundStatus status() const noexcept { return m_Status; }
        virtual void update(const float dt) {}
        virtual bool play(uint32_t numLoops = 1) { return false; }
        virtual bool pause() { return false; }
        virtual bool stop() { return false; }
        virtual bool restart() { return false; }
        [[nodiscard]] inline constexpr uint32_t getLoopsLeft() const noexcept { return m_Loops - m_CurrentLoop; }
        [[nodiscard]] virtual float getAttenuation() const { return 0.0f; }
        [[nodiscard]] virtual uint32_t getChannelCount() const { return 0U; }
        [[nodiscard]] virtual float getMinDistance() const { return 0.0f; }
        virtual void setMinDistance(float minDistance) {}
        [[nodiscard]] virtual bool isRelativeToListener() const { return false; }
        virtual void setRelativeToListener(bool relative = true) {}


        virtual void setAttenuation(float attenuation) {}
        [[nodiscard]] virtual glm::vec3 getPosition() const { return glm::vec3(0.0f); }
        virtual void setPosition(float x, float y, float z) {}
        virtual void setPosition(const glm::vec3& position) {}
        [[nodiscard]] virtual float getVolume() const { return 0.0f; }
        virtual void setVolume(float volume) {}
        [[nodiscard]] virtual float getPitch() const { return 0.0f; }
        virtual void setPitch(float pitch) {}
};

#endif