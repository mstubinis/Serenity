#pragma once
#ifndef ENGINE_SOUND_BASE_CLASS_H
#define ENGINE_SOUND_BASE_CLASS_H

#include <core/engine/Engine_Utils.h>
#include <glm/vec3.hpp>

struct SoundStatus final {enum Status {
    Playing,
    PlayingLooped,
    Paused,
    Stopped,
    Fresh,
};};

namespace Engine {
    namespace epriv {
        class SoundManager;
    };
};

class SoundBaseClass {
    friend class Engine::epriv::SoundManager;
    protected:
        SoundStatus::Status      m_Status;
        uint                     m_Loops;
        uint                     m_CurrentLoop;

        SoundBaseClass(const uint& numLoops);
    public:
        virtual ~SoundBaseClass();

        const SoundStatus::Status& status();
        virtual void update(const double& dt);
        virtual const bool play(const uint& numLoops = 1);
        virtual const bool pause();
        virtual const bool stop();
        virtual const bool restart();
        const uint& getLoopsLeft();

        virtual const float getAttenuation();
        virtual const glm::vec3& getPosition();
        virtual void setPosition(const float& x, const float& y, const float& z);
        virtual void setPosition(const glm::vec3& position);
        virtual const float getVolume();
        virtual void setVolume(const float& volume);
        virtual const float getPitch();
        virtual void setPitch(const float& pitch);
};

#endif