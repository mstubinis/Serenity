#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/sounds/SoundMusic.h>
#include <core/engine/sounds/SoundData.h>
#include <core/engine/resources/Engine_Resources.h>

#include <core/engine/system/Engine.h>
#include <core/engine/events/EventDispatcher.h>
#include <core/engine/events/Event.h>

using namespace Engine;
using namespace Engine::priv;
using namespace std;

SoundMusic::SoundMusic() : SoundBaseClass(1) {
}
void SoundMusic::update(const float dt) {
    auto sfStatus = m_Sound.getStatus();
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
bool SoundMusic::play(unsigned int numLoops) {
    auto sfStatus = m_Sound.getStatus();
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
    Event ev(EventType::SoundStatusChanged);
    ev.eventSoundStatusChanged = e;
    Core::m_Engine->m_EventModule.m_EventDispatcher.dispatchEvent(ev);
    return true;
}
bool SoundMusic::pause() {
    auto sfStatus = m_Sound.getStatus();
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
    Core::m_Engine->m_EventModule.m_EventDispatcher.dispatchEvent(ev);
    return true;
}
bool SoundMusic::stop(bool stopAllLoops) {
    auto sfStatus = m_Sound.getStatus();
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
    Event ev(EventType::SoundStatusChanged);
    ev.eventSoundStatusChanged = e;
    Core::m_Engine->m_EventModule.m_EventDispatcher.dispatchEvent(ev);
    return true;
}
bool SoundMusic::restart() {
    auto sfStatus = m_Sound.getStatus();
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
    if (status() == SoundStatus::Fresh || m_Sound.getPlayingOffset() == sf::Time::Zero) {
        return false;
    }
    m_Sound.setPlayingOffset(sf::Time::Zero); //only if paused or playing. if stopped, has no effect

    EventSoundStatusChanged e(m_Status);
    Event ev(EventType::SoundStatusChanged);
    ev.eventSoundStatusChanged = e;
    Core::m_Engine->m_EventModule.m_EventDispatcher.dispatchEvent(ev);
    return true;
}
float SoundMusic::getAttenuation() const {
    return m_Sound.getAttenuation();
}
glm::vec3 SoundMusic::getPosition() const {
    const sf::Vector3f pos = m_Sound.getPosition();
    return glm::vec3(pos.x, pos.y, pos.z);
}
uint SoundMusic::getChannelCount() const {
    return m_Sound.getChannelCount();
}
float SoundMusic::getMinDistance() const {
    return m_Sound.getMinDistance();
}
void SoundMusic::setMinDistance(float minDistance) {
    m_Sound.setMinDistance(minDistance);
}
bool SoundMusic::isRelativeToListener() const {
    return m_Sound.isRelativeToListener();
}
void SoundMusic::setRelativeToListener(bool relative) {
    m_Sound.setRelativeToListener(relative);
}
void SoundMusic::setAttenuation(float attenuation) {
    m_Sound.setAttenuation(attenuation);
}
void SoundMusic::setPosition(float x, float y, float z) {
    m_Sound.setPosition(x, y, z);
}
void SoundMusic::setPosition(const glm::vec3& position) {
    m_Sound.setPosition(position.x, position.y, position.z);
}
float SoundMusic::getVolume() const {
    return m_Sound.getVolume();
}
void SoundMusic::setVolume(float volume) {
    m_Sound.setVolume(volume);
}
float SoundMusic::getPitch() const {
    return m_Sound.getPitch();
}
void SoundMusic::setPitch(float pitch) {
    m_Sound.setPitch(pitch);
}