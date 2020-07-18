#include "core/engine/utils/PrecompiledHeader.h"
#include <core/engine/sounds/SoundQueue.h>
#include <core/engine/sounds/SoundEffect.h>
#include <core/engine/sounds/SoundMusic.h>
#include <core/engine/sounds/SoundModule.h>

using namespace std;

SoundQueue::SoundQueue(Engine::priv::SoundModule& module, float delay) : m_SoundModule(module){
    m_DelayInSeconds = delay;
    m_SoundModule.m_SoundQueues.push_back(this);
}
SoundQueue::~SoundQueue() {
    clear();
}
void SoundQueue::enqueueEffect(Handle handle, unsigned int loops) {
    if (!m_Current) {
        m_Current = m_SoundModule.getNextFreeEffect();
        if (m_Current) {
            handle.m_Type = 1;
            m_SoundModule.setSoundInformation(handle, *static_cast<SoundEffect*>(m_Current));
        }
    }
    m_Queue.push(handle);
}
void SoundQueue::enqueueMusic(Handle handle, unsigned int loops) {
    if (!m_Current) {
        m_Current = m_SoundModule.getNextFreeMusic();
        if (m_Current) {
            handle.m_Type = 2;
            m_SoundModule.setSoundInformation(handle, *static_cast<SoundMusic*>(m_Current));
        }
    }
    m_Queue.push(handle);
}
void SoundQueue::dequeue() {
    if (m_Queue.size() > 0) {
        auto item = m_Queue.front();
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
        }else{
            if (m_Queue.size() > 0) {
                if (!m_Current) {
                    Handle handle = m_Queue.front();
                    if (handle.type() == 1) {
                        m_Current = m_SoundModule.getNextFreeEffect();
                        if (m_Current) {
                            m_SoundModule.setSoundInformation(handle, *static_cast<SoundEffect*>(m_Current));
                        }
                    }else if (handle.type() == 2) {
                        m_Current = m_SoundModule.getNextFreeMusic();
                        if (m_Current) {
                            m_SoundModule.setSoundInformation(handle, *static_cast<SoundMusic*>(m_Current));
                        }
                    }
                }
                SoundStatus::Status status = m_Current->status();
                switch (status) {
                    case SoundStatus::Fresh: {
                        m_Current->play();
                        break;
                    }case SoundStatus::Stopped: {
                        unsigned int loopsLeft = m_Current->getLoopsLeft();
                        if (loopsLeft <= 1) {
                            m_Queue.pop();
                            m_IsDelayProcess = true;
                            m_Current        = nullptr;
                        }
                        break;
                    }case SoundStatus::Playing: {
                    }case SoundStatus::PlayingLooped: {
                        m_Current->update(dt);
                        break;
                    }default: {
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