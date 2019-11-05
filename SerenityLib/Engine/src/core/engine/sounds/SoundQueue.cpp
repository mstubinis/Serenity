#include <core/engine/sounds/Engine_Sounds.h>
#include <core/engine/sounds/SoundQueue.h>
#include <core/engine/sounds/SoundEffect.h>
#include <core/engine/sounds/SoundMusic.h>

using namespace std;

SoundQueue::SoundQueue(Engine::epriv::SoundManager& manager, const float& delay):m_SoundManager(manager){
    m_DelayInSeconds = delay;
    m_DelayTimer     = 0;
    m_IsDelayProcess = false;
    m_Current        = nullptr;
    m_Active         = false;
    m_SoundManager.m_SoundQueues.push_back(this);
}
SoundQueue::~SoundQueue() {
    clear();
}
void SoundQueue::enqueueEffect(Handle handle, const uint& loops) {
    if (!m_Current) {
        m_Current = m_SoundManager._getFreeEffect();
        if (m_Current) {
            handle.type = 1;
            m_SoundManager._setSoundInformation(handle, *static_cast<SoundEffect*>(m_Current));
        }
    }
    m_Queue.push(handle);
}
void SoundQueue::enqueueMusic(Handle handle, const uint& loops) {
    if (!m_Current) {
        m_Current = m_SoundManager._getFreeMusic();
        if (m_Current) {
            handle.type = 2;
            m_SoundManager._setSoundInformation(handle, *static_cast<SoundMusic*>(m_Current));
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
void SoundQueue::update(const double& dt) {
    if (m_Active) {
        if (m_IsDelayProcess) {
            m_DelayTimer += static_cast<float>(dt);
            if (m_DelayTimer > m_DelayInSeconds) {
                m_IsDelayProcess = false;
                m_DelayTimer = 0;
            }
        }else{
            if (m_Queue.size() > 0) {
                if (!m_Current) {
                    auto& handle = m_Queue.front();
                    if (handle.type == 1) {
                        m_Current = m_SoundManager._getFreeEffect();
                        if (m_Current) {
                            m_SoundManager._setSoundInformation(handle, *static_cast<SoundEffect*>(m_Current));
                        }
                    }else if (handle.type == 2) {
                        m_Current = m_SoundManager._getFreeMusic();
                        if (m_Current) {
                            m_SoundManager._setSoundInformation(handle, *static_cast<SoundMusic*>(m_Current));
                        }
                    }
                }
                const SoundStatus::Status& status = m_Current->status();
                if (status == SoundStatus::Fresh) {
                    m_Current->play();
                }else if (status == SoundStatus::Playing || status == SoundStatus::PlayingLooped) {
                    m_Current->update(dt);
                }else if (status == SoundStatus::Stopped) {
                    const auto& loopsLeft = m_Current->getLoopsLeft();
                    if (loopsLeft <= 1) {
                        m_Queue.pop();
                        m_IsDelayProcess = true;
                        m_Current = nullptr;
                    }
                }
            }
        }
    }
}
void SoundQueue::clear() {
    if (m_Current)
        m_Current->stop();
    while (m_Queue.size() > 0)
        m_Queue.pop();
    m_Current = nullptr;
    m_DelayTimer = 0;
    m_IsDelayProcess = false;
    m_Active = false;
}
const bool SoundQueue::empty() const {
    return m_Queue.empty();
}
const bool& SoundQueue::active() const {
    return m_Active;
}
void SoundQueue::activate() {
    m_Active = true;
}
void SoundQueue::deactivate() {
    m_Active = false;
}
const size_t SoundQueue::size() const {
    return m_Queue.size();
}