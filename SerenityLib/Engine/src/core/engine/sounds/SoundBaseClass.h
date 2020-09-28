#pragma once
#ifndef ENGINE_SOUND_BASE_CLASS_H
#define ENGINE_SOUND_BASE_CLASS_H

enum class SoundStatus : unsigned char {
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
        SoundStatus   m_Status      = SoundStatus::Fresh;
        unsigned int  m_Loops       = 0U;
        unsigned int  m_CurrentLoop = 0U;

        SoundBaseClass(unsigned int numLoops) 
            : m_Loops{ numLoops }
        {}
    public:
        SoundBaseClass(const SoundBaseClass& other)                = delete;
        SoundBaseClass& operator=(const SoundBaseClass& other)     = delete;
        SoundBaseClass(SoundBaseClass&& other) noexcept            = default;
        SoundBaseClass& operator=(SoundBaseClass&& other) noexcept = default;
        virtual ~SoundBaseClass()                                  = default;

        inline CONSTEXPR SoundStatus status() const noexcept { return m_Status; }
        virtual void update(const float dt) {}
        virtual bool play(unsigned int numLoops = 1) { return false; }
        virtual bool pause() { return false; }
        virtual bool stop() { return false; }
        virtual bool restart() { return false; }
        inline CONSTEXPR unsigned int getLoopsLeft() const noexcept { return m_Loops - m_CurrentLoop; }
        virtual float getAttenuation() const { return 0.0f; }
        virtual unsigned int getChannelCount() const { return 0U; }
        virtual float getMinDistance() const { return 0.0f; }
        virtual void setMinDistance(float minDistance) {}
        virtual bool isRelativeToListener() const { return false; }
        virtual void setRelativeToListener(bool relative = true) {}


        virtual void setAttenuation(float attenuation) {}
        virtual glm::vec3 getPosition() const { return glm::vec3(0.0f); }
        virtual void setPosition(float x, float y, float z) {}
        virtual void setPosition(const glm::vec3& position) {}
        virtual float getVolume() const { return 0.0f; }
        virtual void setVolume(float volume) {}
        virtual float getPitch() const { return 0.0f; }
        virtual void setPitch(float pitch) {}
};

#endif