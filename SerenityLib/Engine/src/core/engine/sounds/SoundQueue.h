#pragma once
#ifndef ENGINE_SOUND_QUEUE_H
#define ENGINE_SOUND_QUEUE_H

#include <core/engine/utils/Utils.h>
#include <queue>

struct Handle;
class  SoundBaseClass;
namespace Engine {
    namespace epriv {
        class SoundManager;
    };
};

class SoundQueue final {
    friend class Engine::epriv::SoundManager;
    private:
        Engine::epriv::SoundManager&   m_SoundManager;
        SoundBaseClass*                m_Current;
        std::queue<Handle>             m_Queue;
        float                          m_DelayInSeconds;
        float                          m_DelayTimer;
        bool                           m_IsDelayProcess;
        bool                           m_Active;
    public:
        SoundQueue(Engine::epriv::SoundManager& manager, const float& delay = 0.5f);
        ~SoundQueue();

        void enqueueEffect(Handle, const uint& loops = 1); //need a clone handle due to type conversion for determining effect or music
        void enqueueMusic(Handle, const uint& loops = 1); //need a clone handle due to type conversion for determining effect or music
        void dequeue();
        void update(const double& dt);
        void clear();
        const bool empty() const;
        const bool& active() const;
        const size_t size() const;

        void activate();
        void deactivate();
};

#endif