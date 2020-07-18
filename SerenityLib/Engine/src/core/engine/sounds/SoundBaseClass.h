#pragma once
#ifndef ENGINE_SOUND_BASE_CLASS_H
#define ENGINE_SOUND_BASE_CLASS_H

struct SoundStatus final {enum Status {
    Playing,
    PlayingLooped,
    Paused,
    Stopped,
    Fresh,
};};

namespace Engine::priv {
    class SoundModule;
};

class SoundBaseClass {
    friend class Engine::priv::SoundModule;
    protected:
        SoundStatus::Status      m_Status      = SoundStatus::Fresh;
        unsigned int             m_Loops       = 0U;
        unsigned int             m_CurrentLoop = 0U;

        SoundBaseClass(unsigned int numLoops);
    public:
        SoundBaseClass& operator=(const SoundBaseClass&) = delete;
        SoundBaseClass(const SoundBaseClass&) = default;
        SoundBaseClass(SoundBaseClass&&) noexcept = default;
        virtual ~SoundBaseClass();

        SoundStatus::Status status() const;
        virtual void update(const float dt);
        virtual bool play(unsigned int numLoops = 1);
        virtual bool pause();
        virtual bool stop();
        virtual bool restart();
        unsigned int getLoopsLeft() const;
        virtual unsigned int getChannelCount() const;
        virtual float getMinDistance() const;
        virtual void setMinDistance(float minDistance);
        virtual bool isRelativeToListener() const;
        virtual void setRelativeToListener(bool relative = true);

        virtual float getAttenuation() const;
        virtual void setAttenuation(float attenuation);
        virtual glm::vec3 getPosition() const;
        virtual void setPosition(float x, float y, float z);
        virtual void setPosition(const glm::vec3& position);
        virtual float getVolume() const;
        virtual void setVolume(float volume);
        virtual float getPitch() const;
        virtual void setPitch(float pitch);
};

#endif