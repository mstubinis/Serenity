
#include <serenity/resources/sound/SoundData.h>
#include <serenity/resources/sound/SoundEffect.h>

#include <serenity/resources/Handle.h>

#include <serenity/system/Engine.h>
#include <serenity/events/EventDispatcher.h>
#include <serenity/events/Event.h>

using namespace Engine;
using namespace Engine::priv;

#pragma region SoundEffect

SoundEffect::SoundEffect() 
    : m_Loops { 1 }
{}
void SoundEffect::update(const float dt) {
    auto sfStatus = m_Sound.getStatus();
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
bool SoundEffect::play(unsigned int numLoops) {
    auto sfStatus = m_Sound.getStatus();
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

    Event ev(EventType::SoundStatusChanged);
    ev.eventSoundStatusChanged = EventSoundStatusChanged((unsigned int)m_Status);
    Core::m_Engine->m_EventModule.m_EventDispatcher.dispatchEvent(ev);
    return true;
}
bool SoundEffect::pause() {
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

    Event ev(EventType::SoundStatusChanged);
    ev.eventSoundStatusChanged = EventSoundStatusChanged((unsigned int)m_Status);
    Core::m_Engine->m_EventModule.m_EventDispatcher.dispatchEvent(ev);
    return true;
}
bool SoundEffect::stop(bool stopAllLoops) {
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

    Event ev(EventType::SoundStatusChanged);
    ev.eventSoundStatusChanged = EventSoundStatusChanged((unsigned int)m_Status);
    Core::m_Engine->m_EventModule.m_EventDispatcher.dispatchEvent(ev);
    return true;
}
bool SoundEffect::restart() {
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
    if (getStatus() == SoundStatus::Fresh || m_Sound.getPlayingOffset() == sf::Time::Zero) {
        return false;
    }
    m_Sound.setPlayingOffset(sf::Time::Zero); //only if paused or playing. if stopped, has no effect

    Event ev(EventType::SoundStatusChanged);
    ev.eventSoundStatusChanged = EventSoundStatusChanged((unsigned int)m_Status);
    Core::m_Engine->m_EventModule.m_EventDispatcher.dispatchEvent(ev);
    return true;
}
float SoundEffect::getAttenuation() const {
    return m_Sound.getAttenuation();
}
glm::vec3 SoundEffect::getPosition() const {
    const sf::Vector3f v = m_Sound.getPosition();
    return glm::vec3(v.x, v.y, v.z);
}
uint SoundEffect::getChannelCount() const {
    auto buffer = m_Sound.getBuffer();
    if (buffer) {
        return buffer->getChannelCount();
    }
    return 0;
}
void SoundEffect::setAttenuation(float attenuation) {
    m_Sound.setAttenuation(attenuation);
}
float SoundEffect::getMinDistance() const {
    return m_Sound.getMinDistance();
}
void SoundEffect::setMinDistance(float minDistance) {
    m_Sound.setMinDistance(minDistance);
}
bool SoundEffect::isRelativeToListener() const {
    return m_Sound.isRelativeToListener();
}
void SoundEffect::setRelativeToListener(bool relative) {
    m_Sound.setRelativeToListener(relative);
}
void SoundEffect::setPosition(float x, float y, float z) {
    m_Sound.setPosition(x, y, z);
}
void SoundEffect::setPosition(const glm::vec3& position) {
    SoundEffect::setPosition(position.x, position.y, position.z);
}
void SoundEffect::translate(float x, float y, float z) {
    const auto pos = m_Sound.getPosition();
    SoundEffect::setPosition(pos.x + x, pos.y + y, pos.z + z);
}
void SoundEffect::translate(const glm::vec3& position) {
    SoundEffect::translate(position.x, position.y, position.z);
}
float SoundEffect::getVolume() const {
    return m_Sound.getVolume();
}
void SoundEffect::setVolume(float volume) {
    m_Sound.setVolume(volume);
}
float SoundEffect::getPitch() const {
    return m_Sound.getPitch();
}
void SoundEffect::setPitch(float pitch) {
    m_Sound.setPitch(pitch);
}

#pragma endregion

#pragma region SoundEffectLUABinder

SoundEffectLUABinder::SoundEffectLUABinder(SoundEffect& soundEffect)
    : m_SoundEffect{ &soundEffect }
{}

SoundStatus SoundEffectLUABinder::getStatus() const {
    return m_SoundEffect->getStatus();
}
uint32_t SoundEffectLUABinder::getLoopsLeft() const {
    return m_SoundEffect->getLoopsLeft();
}
bool SoundEffectLUABinder::isActive() const {
    return m_SoundEffect->isActive();
}
bool SoundEffectLUABinder::play(luabridge::LuaRef numLoops) {
    return m_SoundEffect->play(numLoops.isNil() ? 1U : numLoops.cast<uint32_t>());
}
bool SoundEffectLUABinder::pause() {
    return m_SoundEffect->pause();
}
bool SoundEffectLUABinder::stop(luabridge::LuaRef stopAllLoops) {
    return m_SoundEffect->stop(stopAllLoops.isNil() ? false : stopAllLoops.cast<bool>());
}
bool SoundEffectLUABinder::restart() {
    return m_SoundEffect->restart();
}
float SoundEffectLUABinder::getDuration() const {
    return m_SoundEffect->getDuration();
}
unsigned int SoundEffectLUABinder::getChannelCount() const {
    return m_SoundEffect->getChannelCount();
}
float SoundEffectLUABinder::getMinDistance() const {
    return m_SoundEffect->getMinDistance();
}
void SoundEffectLUABinder::setMinDistance(float minDistance) {
    m_SoundEffect->setMinDistance(minDistance);
}
bool SoundEffectLUABinder::isRelativeToListener() const {
    return m_SoundEffect->isRelativeToListener();
}
void SoundEffectLUABinder::setRelativeToListener(luabridge::LuaRef relative) {
    m_SoundEffect->setRelativeToListener(relative.isNil() ? true : relative.cast<bool>());
}
float SoundEffectLUABinder::getAttenuation() const {
    return m_SoundEffect->getAttenuation();
}

glm::vec3 SoundEffectLUABinder::getPosition() const {
    return m_SoundEffect->getPosition();
}
void SoundEffectLUABinder::setPosition(luabridge::LuaRef x, luabridge::LuaRef y, luabridge::LuaRef z) const {
    if (!x.isNil()) {
        if (x.isNumber() && y.isNumber() && z.isNumber()) {
            m_SoundEffect->setPosition(x.cast<float>(), y.cast<float>(), z.cast<float>());
        } else if (!x.isNumber()) {
            m_SoundEffect->setPosition(x.cast<glm::vec3>());
        }
    }
}
void SoundEffectLUABinder::translate(luabridge::LuaRef x, luabridge::LuaRef y, luabridge::LuaRef z) const {
    if (!x.isNil()) {
        if (x.isNumber() && y.isNumber() && z.isNumber()) {
            m_SoundEffect->translate(x.cast<float>(), y.cast<float>(), z.cast<float>());
        } else if (!x.isNumber()) {
            m_SoundEffect->translate(x.cast<glm::vec3>());
        }
    }
}
void SoundEffectLUABinder::setAttenuation(float attenuation) {
    m_SoundEffect->setAttenuation(attenuation);
}
float SoundEffectLUABinder::getVolume() const {
    return m_SoundEffect->getVolume();
}
void SoundEffectLUABinder::setVolume(float volume) const {
    m_SoundEffect->setVolume(volume);
}
float SoundEffectLUABinder::getPitch() const {
    return m_SoundEffect->getPitch();
}
void SoundEffectLUABinder::setPitch(float pitch) const {
    m_SoundEffect->setPitch(pitch);
}

#pragma endregion