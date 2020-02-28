#pragma once
#ifndef ENGINE_SOUND_QUEUE_H
#define ENGINE_SOUND_QUEUE_H

#include <queue>
#include <core/engine/resources/Handle.h>

class  SoundBaseClass;
namespace Engine::priv {
    class SoundManager;
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
        SoundQueue() = delete;
        SoundQueue(Engine::priv::SoundManager& manager, const float delay = 0.5f);
        ~SoundQueue();

        void enqueueEffect(Handle soundEffectHandle, const unsigned int loops = 1);
        void enqueueMusic(Handle soundMusicHandle, const unsigned int loops = 1);
        void dequeue();
        void update(const float dt);
        void clear();
        const bool empty() const;
        const bool& active() const;
        const size_t size() const;

        void activate();
        void deactivate();
};
#endif