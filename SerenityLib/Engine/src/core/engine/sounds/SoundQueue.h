#pragma once
#ifndef ENGINE_SOUND_QUEUE_H
#define ENGINE_SOUND_QUEUE_H

#include <queue>

#include <core/engine/resources/Handle.h>

class  SoundBaseClass;
namespace Engine {
    namespace priv {
        class SoundManager;
    };
};
class SoundQueue final {
    friend class Engine::priv::SoundManager;
    private:
        Engine::priv::SoundManager&   m_SoundManager;
        SoundBaseClass*                m_Current;
        std::queue<Handle>             m_Queue;
        float                          m_DelayInSeconds;
        float                          m_DelayTimer;
        bool                           m_IsDelayProcess;
        bool                           m_Active;
    public:
        SoundQueue(Engine::priv::SoundManager& manager, const float& delay = 0.5f);
        ~SoundQueue();

        void enqueueEffect(Handle&, const unsigned int& loops = 1); //need a clone handle due to type conversion for determining effect or music
        void enqueueMusic(Handle&, const unsigned int& loops = 1); //need a clone handle due to type conversion for determining effect or music
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