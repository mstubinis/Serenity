#pragma once
#ifndef ENGINE_SOUND_QUEUE_H
#define ENGINE_SOUND_QUEUE_H

class  SoundBaseClass;
namespace Engine::priv {
    class SoundModule;
};

#include <core/engine/resources/Handle.h>

class SoundQueue {
    friend class Engine::priv::SoundModule;
    private:
        Engine::priv::SoundModule&     m_SoundModule;
        std::queue<Handle>             m_Queue;
        SoundBaseClass*                m_Current = nullptr;
        float                          m_DelayInSeconds  = 0.0f;
        float                          m_DelayTimer      = 0.0f;
        bool                           m_IsDelayProcess  = false;
        bool                           m_Active          = false;

        SoundQueue() = delete;
    public:
        SoundQueue(Engine::priv::SoundModule& manager, float delay = 0.5f);
        virtual ~SoundQueue();

        inline bool empty() const noexcept { return m_Queue.empty(); }
        inline size_t size() const noexcept { return m_Queue.size(); }
        inline CONSTEXPR bool active() const noexcept { return m_Active; }
        inline CONSTEXPR void activate(bool active = true) noexcept { m_Active = active; }
        inline CONSTEXPR void deactivate() noexcept { m_Active = false; }
        void enqueueEffect(Handle soundEffectHandle, unsigned int loops = 1);
        void enqueueMusic(Handle soundMusicHandle, unsigned int loops = 1);
        void dequeue();
        void clear();

        void update(const float dt);
};
#endif