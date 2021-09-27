
#include <serenity/resources/sound/SoundQueue.h>
#include <serenity/resources/sound/SoundEffect.h>
#include <serenity/resources/sound/SoundMusic.h>
#include <serenity/resources/sound/SoundModule.h>

SoundQueue::SoundQueue(Engine::priv::SoundModule& module, float delay) 
    : m_SoundModule{ module }
    , m_DelayInSeconds{ delay }
{
    m_SoundModule.m_SoundQueues.push_back(std::unique_ptr<SoundQueue>(this));
}
SoundQueue::~SoundQueue() {
    clear();
}
void SoundQueue::enqueueEffect(Handle handle, uint32_t loops) {
    if (!m_Current) {
        m_Current = m_SoundModule.getNextFreeEffect();
        if (m_Current) {
            m_SoundModule.setSoundInformation(handle, *(SoundEffect*)m_Current);
        }
    }
    m_Queue.emplace(handle, 1);
}
void SoundQueue::enqueueMusic(Handle handle, uint32_t loops) {
    if (!m_Current) {
        m_Current = m_SoundModule.getNextFreeMusic();
        if (m_Current) {
            m_SoundModule.setSoundInformation(handle, *(SoundMusic*)m_Current);
        }
    }
    m_Queue.emplace(handle, 2);
}
void SoundQueue::dequeue() {
    if (m_Queue.size() > 0) {
        m_Queue.pop();
        m_IsDelayProcess = true;
    }
}
void SoundQueue::update(const float dt) {
    if (m_Active) {
        if (m_IsDelayProcess) {
            m_DelayTimer += dt;
            if (m_DelayTimer > m_DelayInSeconds) {
                m_IsDelayProcess = false;
                m_DelayTimer = 0.0f;
            }
        } else {
            if (m_Queue.size() > 0) {
                if (!m_Current) {
                    auto& [handle, type] = m_Queue.front();
                    if (type == 1) {
                        m_Current = m_SoundModule.getNextFreeEffect();
                        if (m_Current) {
                            m_SoundModule.setSoundInformation(handle, *(SoundEffect*)m_Current);
                        }
                    } else if (type == 2) {
                        m_Current = m_SoundModule.getNextFreeMusic();
                        if (m_Current) {
                            m_SoundModule.setSoundInformation(handle, *(SoundMusic*)m_Current);
                        }
                    }
                }
                SoundStatus status = m_Current->status();
                switch (status) {
                    case SoundStatus::Fresh: {
                        m_Current->play();
                        break;
                    } case SoundStatus::Stopped: {
                        uint32_t loopsLeft = m_Current->getLoopsLeft();
                        if (loopsLeft <= 1) {
                            m_Queue.pop();
                            m_IsDelayProcess = true;
                            m_Current        = nullptr;
                        }
                        break;
                    } case SoundStatus::Playing: {
                    } case SoundStatus::PlayingLooped: {
                        m_Current->update(dt);
                        break;
                    } default: {
                        break;
                    }
                }
            }
        }
    }
}
void SoundQueue::clear() {
    if (m_Current) {
        m_Current->stop();
    }
    while (m_Queue.size() > 0) {
        m_Queue.pop();
    }
    m_Current        = nullptr;
    m_DelayTimer     = 0.0f;
    m_IsDelayProcess = false;
    m_Active         = false;
}