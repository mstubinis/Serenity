#include <core/engine/sounds/Engine_Sounds.h>
#include <core/engine/sounds/SoundQueue.h>
#include <core/engine/sounds/SoundEffect.h>
#include <core/engine/sounds/SoundMusic.h>

using namespace std;

SoundQueue::SoundQueue(Engine::epriv::SoundManager& manager, const float& delay):m_SoundManager(manager){
    m_DelayInSeconds = delay;
    m_DelayTimer     = 0;
    m_IsDelayProcess = false;
    m_Active         = false;
    m_SoundManager.m_SoundQueues.push_back(this);
}
SoundQueue::~SoundQueue() {
    clear();
}
void SoundQueue::enqueueEffect(Handle& handle, const uint& loops) {
    auto effect = new SoundEffect(handle, loops);
    effect->play();
    m_Queue.push(effect);
}
void SoundQueue::enqueueMusic(Handle& handle, const uint& loops) {
    auto music = new SoundMusic(handle, loops);
    music->play();
    m_Queue.push(music);
}
void SoundQueue::dequeue() {
    if (m_Queue.size() > 0) {
        auto item = m_Queue.front();
        SAFE_DELETE(item);
        m_Queue.pop();
        m_IsDelayProcess = true;
    }
}
void SoundQueue::update(const double& dt) {
    if (!m_Active)
        return;
    if (m_IsDelayProcess) {
        m_DelayTimer += dt;
        if (m_DelayTimer > m_DelayInSeconds) {
            m_IsDelayProcess = false;
            m_DelayTimer = 0;
        }
    }else{
        if (m_Queue.size() > 0) {
            auto item = m_Queue.front();
            const SoundStatus::Status& status = item->status();
            if (status == SoundStatus::Fresh) {
                item->play();
            }else if (status == SoundStatus::Playing || status == SoundStatus::PlayingLooped){
                item->update(dt);
            }else if (status == SoundStatus::Stopped){
                if (item->getLoopsLeft() <= 1) {
                    SAFE_DELETE(item); //this sound has finished, remove it from the queue and start the delay process
                    m_Queue.pop();
                    m_IsDelayProcess = true;
                }
            }
        }
    }
}
void SoundQueue::clear() {
    SAFE_DELETE_QUEUE(m_Queue);
}
const bool& SoundQueue::empty() const {
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