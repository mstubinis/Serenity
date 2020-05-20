#pragma once
#ifndef ENGINE_SOUND_QUEUE_H
#define ENGINE_SOUND_QUEUE_H

class  SoundBaseClass;
namespace Engine::priv {
    class SoundManager;
};

#include <queue>
#include <core/engine/resources/Handle.h>

class SoundQueue final {
    friend class Engine::priv::SoundManager;
    private:
        Engine::priv::SoundManager&   m_SoundManager;
        SoundBaseClass*                m_Current         = nullptr;
        std::queue<Handle>             m_Queue;
        float                          m_DelayInSeconds  = 0.0f;
        float                          m_DelayTimer      = 0.0f;
        bool                           m_IsDelayProcess  = false;
        bool                           m_Active          = false;
    public:
        SoundQueue() = delete;
        SoundQueue(Engine::priv::SoundManager& manager, const float delay = 0.5f);
        ~SoundQueue();

        void enqueueEffect(Handle soundEffectHandle, const unsigned int loops = 1);
        void enqueueMusic(Handle soundMusicHandle, const unsigned int loops = 1);
        void dequeue();
        void update(const float dt);
        void clear();
        bool empty() const;
        bool active() const;
        size_t size() const;

        void activate();
        void deactivate();
};
#endif