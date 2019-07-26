#pragma once
#ifndef ENGINE_SOUND_QUEUE_H
#define ENGINE_SOUND_QUEUE_H

#include <core/engine/utils/Utils.h>
#include <queue>

class Handle;
class SoundBaseClass;
namespace Engine {
    namespace epriv {
        class SoundManager;
    };
};

class SoundQueue final {
    friend class Engine::epriv::SoundManager;
    private:
        Engine::epriv::SoundManager&   m_SoundManager;
        std::queue<SoundBaseClass*>    m_Queue;
        float                          m_DelayInSeconds;
        float                          m_DelayTimer;
        bool                           m_IsDelayProcess;
        bool                           m_Active;
    public:
        SoundQueue(Engine::epriv::SoundManager& manager, const float& delay = 0.5f);
        ~SoundQueue();

        void enqueueEffect(Handle&, const uint& loops = 1);
        void enqueueMusic(Handle&, const uint& loops = 1);
        void dequeue();
        void update(const double& dt);
        void clear();
        const bool& empty() const;
        const bool& active() const;

        void activate();
        void deactivate();
};

#endif