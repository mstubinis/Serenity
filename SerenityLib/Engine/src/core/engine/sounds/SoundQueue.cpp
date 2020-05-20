#include <core/engine/sounds/SoundQueue.h>
#include <core/engine/sounds/SoundEffect.h>
#include <core/engine/sounds/SoundMusic.h>
#include <core/engine/sounds/Engine_Sounds.h>

using namespace std;

SoundQueue::SoundQueue(Engine::priv::SoundManager& manager, const float delay) : m_SoundManager(manager){
    m_DelayInSeconds = delay;
    m_SoundManager.m_SoundQueues.push_back(this);
}
SoundQueue::~SoundQueue() {
    clear();
}
void SoundQueue::enqueueEffect(Handle handle, const unsigned int loops) {
    if (!m_Current) {
        m_Current = m_SoundManager._getNextFreeEffect();
        if (m_Current) {
            handle.type = 1;
            m_SoundManager._setSoundInformation(handle, *static_cast<SoundEffect*>(m_Current));
        }
    }
    m_Queue.push(handle);
}
void SoundQueue::enqueueMusic(Handle handle, const unsigned int loops) {
    if (!m_Current) {
        m_Current = m_SoundManager._getNextFreeMusic();
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
void SoundQueue::update(const float dt) {
    if (m_Active) {
        if (m_IsDelayProcess) {
            m_DelayTimer += dt;
            if (m_DelayTimer > m_DelayInSeconds) {
                m_IsDelayProcess = false;
                m_DelayTimer = 0;
            }
        }else{
            if (m_Queue.size() > 0) {
                if (!m_Current) {
                    auto& handle = m_Queue.front();
                    if (handle.type == 1) {
                        m_Current = m_SoundManager._getNextFreeEffect();
                        if (m_Current) {
                            m_SoundManager._setSoundInformation(handle, *static_cast<SoundEffect*>(m_Current));
                        }
                    }else if (handle.type == 2) {
                        m_Current = m_SoundManager._getNextFreeMusic();
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
                    const auto loopsLeft = m_Current->getLoopsLeft();
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
    if (m_Current) {
        m_Current->stop();
    }
    while (m_Queue.size() > 0) {
        m_Queue.pop();
    }
    m_Current        = nullptr;
    m_DelayTimer     = 0;
    m_IsDelayProcess = false;
    m_Active         = false;
}
bool SoundQueue::empty() const {
    return m_Queue.empty();
}
bool SoundQueue::active() const {
    return m_Active;
}
void SoundQueue::activate() {
    m_Active = true;
}
void SoundQueue::deactivate() {
    m_Active = false;
}
size_t SoundQueue::size() const {
    return m_Queue.size();
}