#include <core/engine/sounds/SoundData.h>
#include <core/engine/sounds/SoundEffect.h>

#include <core/engine/resources/Handle.h>

#include <core/engine/system/Engine.h>
#include <core/engine/events/Engine_EventDispatcher.h>
#include <core/engine/events/Engine_EventObject.h>

using namespace Engine;
using namespace Engine::priv;
using namespace std;

SoundEffect::SoundEffect() : SoundBaseClass(1) {
    m_Active   = false;
    m_Duration = 0;
}
SoundEffect::~SoundEffect() {

}


void SoundEffect::update(const float dt) {
    const auto sfStatus = m_Sound.getStatus();
    if (sfStatus == sf::SoundSource::Status::Stopped) {
        if (m_Loops >= 2) {//handle the looping logic
            if (getLoopsLeft() >= 2) {
                m_CurrentLoop++;
                play(m_Loops); //apparently playing the sound when it is stopped restarts it (sfml internally)
            }//else {
             //   stop();
            //}
        }else if (m_Loops == 1) {//only once
            stop();
        }else {//endless loop (sound will have to be stoped manually by the user to end an endless loop)
            play(m_Loops); //apparently playing the sound when it is stopped restarts it (sfml internally)
        }
    }
}
const float SoundEffect::getDuration() const {
    return m_Duration;
}
const bool SoundEffect::play(const uint numLoops) {
    const auto sfStatus = m_Sound.getStatus();
    m_Loops = numLoops;
    switch (sfStatus) {
        case sf::SoundSource::Status::Stopped: {//it starts
            (m_Loops != 1) ? m_Status = SoundStatus::PlayingLooped : m_Status = SoundStatus::Playing;
            break;
        }case sf::SoundSource::Status::Paused: {//it resumes
            (m_Loops != 1) ? m_Status = SoundStatus::PlayingLooped : m_Status = SoundStatus::Playing;
            break;
        }case sf::SoundSource::Status::Playing: {//it restarts
            (m_Loops != 1) ? m_Status = SoundStatus::PlayingLooped : m_Status = SoundStatus::Playing;
            break;
        }default: {
            return false;
        }
    }
    m_Sound.play();
    m_Active = true;

    EventSoundStatusChanged e(m_Status);
    Event ev(EventType::SoundStatusChanged);
    ev.eventSoundStatusChanged = e;
    Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
    return true;
}
const bool SoundEffect::pause() {
    const auto sfStatus = m_Sound.getStatus();
    switch (sfStatus) {
        case sf::SoundSource::Status::Stopped: {
            m_Status = SoundStatus::Stopped;
            return false;
        }case sf::SoundSource::Status::Paused: {
            m_Status = SoundStatus::Paused;
            return false;
        }case sf::SoundSource::Status::Playing: {
            m_Status = SoundStatus::Paused;
            break;
        }default: {
            return false;
        }
    }
    m_Sound.pause();

    EventSoundStatusChanged e(m_Status);
    Event ev(EventType::SoundStatusChanged);
    ev.eventSoundStatusChanged = e;
    Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
    return true;
}
const bool SoundEffect::stop(const bool stopAllLoops) {
    const auto sfStatus = m_Sound.getStatus();
    switch (sfStatus) {
        case sf::SoundSource::Status::Stopped: {
            if (m_Status != SoundStatus::Stopped) {
                m_Status = SoundStatus::Stopped;
                break;
            }
            return false;
        }case sf::SoundSource::Status::Paused: {
            m_Status = SoundStatus::Stopped;
            break;
        }case sf::SoundSource::Status::Playing: {
            //this is to prevent event dispatch spamming if you wanted to stop a sound as it was created
            if (m_Sound.getPlayingOffset().asSeconds() <= 0.01f) {
                m_Sound.stop();
                m_Status = SoundStatus::Stopped;
                return false;
            }
            m_Status = SoundStatus::Stopped;
            break;
        }default: {
            return false;
        }
    }
    m_Sound.stop(); //stops if playing or paused. does nothing if already stopped. also resets playing position to zero.
    if (stopAllLoops) {
        m_CurrentLoop = 0;
        m_Loops = 0;
    }

    EventSoundStatusChanged e(m_Status);
    Event ev(EventType::SoundStatusChanged);
    ev.eventSoundStatusChanged = e;
    Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
    return true;
}
const bool SoundEffect::restart() {
    const auto sfStatus = m_Sound.getStatus();
    switch (sfStatus) {
        case sf::SoundSource::Status::Stopped: {
            return false;
        }case sf::SoundSource::Status::Paused: {
            m_Status = SoundStatus::Paused;
            break;
        }case sf::SoundSource::Status::Playing: {
            m_Status = SoundStatus::Playing;
            break;
        }default: {
            return false;
        }
    }
    if (status() == SoundStatus::Fresh || m_Sound.getPlayingOffset() == sf::Time::Zero)
        return false;
    m_Sound.setPlayingOffset(sf::Time::Zero); //only if paused or playing. if stopped, has no effect

    EventSoundStatusChanged e(m_Status);
    Event ev(EventType::SoundStatusChanged);
    ev.eventSoundStatusChanged = e;
    Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
    return true;
}

const float SoundEffect::getAttenuation() const {
    return m_Sound.getAttenuation();
}
const glm::vec3 SoundEffect::getPosition() {
    const sf::Vector3f v = m_Sound.getPosition();
    return glm::vec3(v.x, v.y, v.z);
}
const uint SoundEffect::getChannelCount() {
    auto buffer = m_Sound.getBuffer();
    if (buffer)
        return buffer->getChannelCount();
    return 0;
}
void SoundEffect::setAttenuation(const float attenuation) {
    m_Sound.setAttenuation(attenuation);
}
const float SoundEffect::getMinDistance() {
    return m_Sound.getMinDistance();
}
void SoundEffect::setMinDistance(const float minDistance) {
    m_Sound.setMinDistance(minDistance);
}
const bool SoundEffect::isRelativeToListener() {
    return m_Sound.isRelativeToListener();
}
void SoundEffect::setRelativeToListener(const bool relative) {
    m_Sound.setRelativeToListener(relative);
}
void SoundEffect::setPosition(const float x, const float y, const float z) {
    m_Sound.setPosition(x, y, z);
}
void SoundEffect::setPosition(const glm::vec3& position) {
    m_Sound.setPosition(position.x, position.y, position.z);
}
const float SoundEffect::getVolume() const {
    return m_Sound.getVolume();
}
void SoundEffect::setVolume(const float volume) {
    m_Sound.setVolume(volume);
}
const float SoundEffect::getPitch() const {
    return m_Sound.getPitch();
}
void SoundEffect::setPitch(const float pitch) {
    m_Sound.setPitch(pitch);
}