#include <core/engine/sounds/SoundMusic.h>
#include <core/engine/sounds/SoundData.h>
#include <core/engine/resources/Engine_Resources.h>

#include <core/engine/Engine.h>
#include <core/engine/events/Engine_EventDispatcher.h>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;


SoundMusic::SoundMusic() : SoundBaseClass(1) {
    m_Active   = false;
    m_Duration = 0;
}
SoundMusic::~SoundMusic() {

}
const float SoundMusic::getDuration() const {
    return m_Duration;
}
void SoundMusic::update(const double& dt) {
    const auto sfStatus = m_Sound.getStatus();
    if (sfStatus == sf::SoundSource::Status::Stopped) {
        const auto& loopsLeft = getLoopsLeft();
        if (m_Loops >= 2) {//handle the looping logic
            if (loopsLeft >= 2) {
                m_CurrentLoop++;
                play(m_Loops); //apparently playing the sound when it is stopped restarts it (sfml internally)
            }else{
                stop();
            }
        }else if (m_Loops == 1) {//only once
            stop();
        }else {//endless loop (sound will have to be stoped manually by the user to end an endless loop)
            play(m_Loops); //apparently playing the sound when it is stopped restarts it (sfml internally)
        }
    }
}
const bool SoundMusic::play(const uint& numLoops) {
    const auto sfStatus = m_Sound.getStatus();
    m_Loops = numLoops;
    switch (sfStatus) {
        case sf::SoundSource::Status::Stopped: {//it starts
            m_Loops != 1 ? m_Status = SoundStatus::PlayingLooped : m_Status = SoundStatus::Playing;
            break;
        }case sf::SoundSource::Status::Paused: {//it resumes
            m_Loops != 1 ? m_Status = SoundStatus::PlayingLooped : m_Status = SoundStatus::Playing;
            break;
        }case sf::SoundSource::Status::Playing: {//it restarts
            m_Loops != 1 ? m_Status = SoundStatus::PlayingLooped : m_Status = SoundStatus::Playing; 
            break;
        }default: {
            return false;
        }
    }
    m_Sound.play();
    m_Active = true;

    EventSoundStatusChanged e(m_Status);
    Event ev;
    ev.eventSoundStatusChanged = e;
    ev.type = EventType::SoundStatusChanged;
    Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
    return true;
}
const bool SoundMusic::pause() {
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
    Event ev;
    ev.eventSoundStatusChanged = e;
    ev.type = EventType::SoundStatusChanged;
    Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
    return true;
}
const bool SoundMusic::stop(const bool& stopAllLoops) {
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
    Event ev;
    ev.eventSoundStatusChanged = e;
    ev.type = EventType::SoundStatusChanged;
    Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
    return true;
}
const bool SoundMusic::restart() {
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
    Event ev;
    ev.eventSoundStatusChanged = e;
    ev.type = EventType::SoundStatusChanged;
    Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(ev);
    return true;
}

const float SoundMusic::getAttenuation() const {
    return m_Sound.getAttenuation();
}
const glm::vec3 SoundMusic::getPosition() {
    const sf::Vector3f pos = m_Sound.getPosition();
    return glm::vec3(pos.x, pos.y, pos.z);
}
const uint SoundMusic::getChannelCount() {
    return m_Sound.getChannelCount();
}
const float SoundMusic::getMinDistance() {
    return m_Sound.getMinDistance();
}
void SoundMusic::setMinDistance(const float& minDistance) {
    m_Sound.setMinDistance(minDistance);
}
const bool SoundMusic::isRelativeToListener() {
    return m_Sound.isRelativeToListener();
}
void SoundMusic::setRelativeToListener(const bool relative) {
    m_Sound.setRelativeToListener(relative);
}
void SoundMusic::setAttenuation(const float& attenuation) {
    m_Sound.setAttenuation(attenuation);
}
void SoundMusic::setPosition(const float& x, const float& y, const float& z) {
    m_Sound.setPosition(x, y, z);
}
void SoundMusic::setPosition(const glm::vec3& position) {
    m_Sound.setPosition(position.x, position.y, position.z);
}
const float SoundMusic::getVolume() const {
    return m_Sound.getVolume();
}
void SoundMusic::setVolume(const float& volume) {
    m_Sound.setVolume(volume);
}
const float SoundMusic::getPitch() const {
    return m_Sound.getPitch();
}
void SoundMusic::setPitch(const float& pitch) {
    m_Sound.setPitch(pitch);
}