#pragma once
#ifndef ENGINE_SOUND_QUEUE_H
#define ENGINE_SOUND_QUEUE_H

class  SoundBaseClass;
namespace Engine::priv {
    class SoundModule;
};

#include <queue>
#include <core/engine/resources/Handle.h>

class SoundQueue {
    friend class Engine::priv::SoundModule;
    private:
        Engine::priv::SoundModule&     m_SoundModule;
        SoundBaseClass*                m_Current         = nullptr;
        std::queue<Handle>             m_Queue;
        float                          m_DelayInSeconds  = 0.0f;
        float                          m_DelayTimer      = 0.0f;
        bool                           m_IsDelayProcess  = false;
        bool                           m_Active          = false;

        SoundQueue() = delete;
    public:
        SoundQueue(Engine::priv::SoundModule& manager, float delay = 0.5f);
        virtual ~SoundQueue();

        void activate(bool active = true);
        void deactivate();
        void enqueueEffect(Handle soundEffectHandle, unsigned int loops = 1);
        void enqueueMusic(Handle soundMusicHandle, unsigned int loops = 1);
        void dequeue();
        void clear();
        bool empty() const;
        bool active() const;
        size_t size() const;

        void update(const float dt);
};
#endif