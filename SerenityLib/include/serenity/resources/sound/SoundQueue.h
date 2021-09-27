#pragma once
#ifndef ENGINE_SOUND_QUEUE_H
#define ENGINE_SOUND_QUEUE_H

class  SoundBaseClass;
namespace Engine::priv {
    class SoundModule;
};

#include <serenity/resources/Handle.h>
#include <queue>

class SoundQueue {
    friend class Engine::priv::SoundModule;
    private:
        std::queue<std::pair<Handle, int8_t>>   m_Queue;
        Engine::priv::SoundModule&              m_SoundModule;
        SoundBaseClass*                         m_Current          = nullptr;
        float                                   m_DelayInSeconds   = 0.0f;
        float                                   m_DelayTimer       = 0.0f;
        bool                                    m_IsDelayProcess   = false;
        bool                                    m_Active           = false;

        SoundQueue() = delete;
    public:
        SoundQueue(Engine::priv::SoundModule& manager, float delay = 0.5f);
        virtual ~SoundQueue();

        [[nodiscard]] inline bool empty() const noexcept { return m_Queue.empty(); }
        [[nodiscard]] inline size_t size() const noexcept { return m_Queue.size(); }
        inline constexpr bool active() const noexcept { return m_Active; }
        inline constexpr void activate(bool active = true) noexcept { m_Active = active; }
        inline constexpr void deactivate() noexcept { m_Active = false; }
        void enqueueEffect(Handle soundEffectHandle, uint32_t loops = 1);
        void enqueueMusic(Handle soundMusicHandle, uint32_t loops = 1);
        void dequeue();
        void clear();

        void update(const float dt);
};
#endif