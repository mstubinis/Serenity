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
    class SoundManager;
};

#include <glm/vec3.hpp>

class SoundBaseClass {
    friend class Engine::priv::SoundManager;
    protected:
        SoundStatus::Status      m_Status;
        unsigned int             m_Loops;
        unsigned int             m_CurrentLoop;

        SoundBaseClass(const unsigned int numLoops);
    public:
        SoundBaseClass& operator=(const SoundBaseClass&) = delete;
        SoundBaseClass(const SoundBaseClass&) = default;
        SoundBaseClass(SoundBaseClass&&) noexcept = default;
        virtual ~SoundBaseClass();

        SoundStatus::Status status();
        virtual void update(const float dt);
        virtual bool play(const unsigned int numLoops = 1);
        virtual bool pause();
        virtual bool stop();
        virtual bool restart();
        unsigned int getLoopsLeft();
        virtual unsigned int getChannelCount();
        virtual float getMinDistance();
        virtual void setMinDistance(const float minDistance);
        virtual bool isRelativeToListener();
        virtual void setRelativeToListener(const bool relative = true);

        virtual float getAttenuation();
        virtual void setAttenuation(const float attenuation);
        virtual glm::vec3 getPosition();
        virtual void setPosition(const float x, const float y, const float z);
        virtual void setPosition(const glm::vec3& position);
        virtual float getVolume();
        virtual void setVolume(const float volume);
        virtual float getPitch();
        virtual void setPitch(const float pitch);
};

#endif