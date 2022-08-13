
#include <serenity/resources/sound/SoundMusic.h>
#include <serenity/resources/sound/SoundData.h>
#include <serenity/resources/Engine_Resources.h>

#include <serenity/system/Engine.h>
#include <serenity/events/EventDispatcher.h>
#include <serenity/events/Event.h>

using namespace Engine;
using namespace Engine::priv;

#pragma region SoundMusic

SoundMusic::SoundMusic() 
    : m_Loops{ 1 }
{}
void SoundMusic::update(const float dt) {
    auto sfStatus = m_Sound.getStatus();
    if (sfStatus == sf::SoundSource::Status::Stopped) {
        const auto& loopsLeft = getLoopsLeft();
        if (m_Loops >= 2) {//handle the looping logic
            if (loopsLeft >= 2) {
                m_CurrentLoop++;
                play(m_Loops); //apparently playing the sound when it is stopped restarts it (sfml internally)
            } else {
                stop();
            }
        } else if (m_Loops == 1) {//only once
            stop();
        } else {//endless loop (sound will have to be stoped manually by the user to end an endless loop)
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
        } case sf::SoundSource::Status::Paused: {//it resumes
            m_Loops != 1 ? m_Status = SoundStatus::PlayingLooped : m_Status = SoundStatus::Playing;
            break;
        } case sf::SoundSource::Status::Playing: {//it restarts
            m_Loops != 1 ? m_Status = SoundStatus::PlayingLooped : m_Status = SoundStatus::Playing; 
            break;
        } default: {
            return false;
        }
    }
    m_Sound.play();
    m_Active = true;

    Event ev(EventType::SoundStatusChanged);
    ev.eventSoundStatusChanged = EventSoundStatusChanged((unsigned int)m_Status);
    Core::m_Engine->m_EventDispatcher.dispatchEvent(ev);
    return true;
}
bool SoundMusic::pause() {
    auto sfStatus = m_Sound.getStatus();
    switch (sfStatus) {
        case sf::SoundSource::Status::Stopped: {
            m_Status = SoundStatus::Stopped;
            return false;
        } case sf::SoundSource::Status::Paused: {
            m_Status = SoundStatus::Paused;
            return false;
        } case sf::SoundSource::Status::Playing: {
            m_Status = SoundStatus::Paused;
            break;
        } default: {
            return false;
        }
    }
    m_Sound.pause();

    Event ev(EventType::SoundStatusChanged);
    ev.eventSoundStatusChanged = EventSoundStatusChanged((unsigned int)m_Status);
    Core::m_Engine->m_EventDispatcher.dispatchEvent(ev);
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
        } case sf::SoundSource::Status::Paused: {
            m_Status = SoundStatus::Stopped;
            break;
        } case sf::SoundSource::Status::Playing: {
            if (m_Sound.getPlayingOffset().asSeconds() <= 0.01f) {
                m_Sound.stop();
                m_Status = SoundStatus::Stopped;
                return false;
            }
            m_Status = SoundStatus::Stopped;
            break;
        } default: {
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
    Core::m_Engine->m_EventDispatcher.dispatchEvent(ev);
    return true;
}
bool SoundMusic::restart() {
    auto sfStatus = m_Sound.getStatus();
    switch (sfStatus) {
        case sf::SoundSource::Status::Stopped: {
            return false;
        } case sf::SoundSource::Status::Paused: {
            m_Status = SoundStatus::Paused;
            break;
        } case sf::SoundSource::Status::Playing: {
            m_Status = SoundStatus::Playing;
            break;
        } default: {
            return false;
        }
    }
    if (getStatus() == SoundStatus::Fresh || m_Sound.getPlayingOffset() == sf::Time::Zero) {
        return false;
    }
    m_Sound.setPlayingOffset(sf::Time::Zero); //only if paused or playing. if stopped, has no effect

    Event ev(EventType::SoundStatusChanged);
    ev.eventSoundStatusChanged = EventSoundStatusChanged((unsigned int)m_Status);;
    Core::m_Engine->m_EventDispatcher.dispatchEvent(ev);
    return true;
}
float SoundMusic::getAttenuation() const {
    return m_Sound.getAttenuation();
}
glm::vec3 SoundMusic::getPosition() const {
    const auto pos = m_Sound.getPosition();
    return glm::vec3{ pos.x, pos.y, pos.z };
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
    SoundMusic::setPosition(position.x, position.y, position.z);
}
void SoundMusic::translate(float x, float y, float z) {
    const auto pos = m_Sound.getPosition();
    m_Sound.setPosition(pos.x + x, pos.y + y, pos.z + z);
}
void SoundMusic::translate(const glm::vec3& position) {
    SoundMusic::translate(position.x, position.y, position.z);
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

#pragma endregion

#pragma region SoundMusicLUABinder

SoundMusicLUABinder::SoundMusicLUABinder(SoundMusic& soundMusic)
    : m_SoundMusic{ &soundMusic }
{}
SoundStatus SoundMusicLUABinder::getStatus() const {
    return m_SoundMusic->getStatus();
}
uint32_t SoundMusicLUABinder::getLoopsLeft() const {
    return m_SoundMusic->getLoopsLeft();
}
bool SoundMusicLUABinder::isActive() const {
    return m_SoundMusic->isActive();
}
bool SoundMusicLUABinder::play(luabridge::LuaRef numLoops) {
    return m_SoundMusic->play(numLoops.isNil() ? 1U : numLoops.cast<uint32_t>());
}
bool SoundMusicLUABinder::pause() {
    return m_SoundMusic->pause();
}
bool SoundMusicLUABinder::stop(luabridge::LuaRef stopAllLoops) {
    return m_SoundMusic->stop(stopAllLoops.isNil() ? false : stopAllLoops.cast<bool>());
}
bool SoundMusicLUABinder::restart() {
    return m_SoundMusic->restart();
}
float SoundMusicLUABinder::getDuration() const {
    return m_SoundMusic->getDuration();
}
unsigned int SoundMusicLUABinder::getChannelCount() const {
    return m_SoundMusic->getChannelCount();
}
float SoundMusicLUABinder::getMinDistance() const {
    return m_SoundMusic->getMinDistance();
}
void SoundMusicLUABinder::setMinDistance(float minDistance) {
    m_SoundMusic->setMinDistance(minDistance);
}
bool SoundMusicLUABinder::isRelativeToListener() const {
    return m_SoundMusic->isRelativeToListener();
}
void SoundMusicLUABinder::setRelativeToListener(luabridge::LuaRef relative) {
    m_SoundMusic->setRelativeToListener(relative.isNil() ? true : relative.cast<bool>());
}
float SoundMusicLUABinder::getAttenuation() const {
    return m_SoundMusic->getAttenuation();
}
void SoundMusicLUABinder::setAttenuation(float attenuation) {
    m_SoundMusic->setAttenuation(attenuation);
}
glm::vec3 SoundMusicLUABinder::getPosition() const {
    return m_SoundMusic->getPosition();
}
void SoundMusicLUABinder::setPosition(luabridge::LuaRef x, luabridge::LuaRef y, luabridge::LuaRef z) {
    if (!x.isNil()) {
        if (x.isNumber() && y.isNumber() && z.isNumber()) {
            m_SoundMusic->setPosition(x.cast<float>(), y.cast<float>(), z.cast<float>());
        } else if (!x.isNumber()) {
            m_SoundMusic->setPosition(x.cast<glm::vec3>());
        }
    }
}
void SoundMusicLUABinder::translate(luabridge::LuaRef x, luabridge::LuaRef y, luabridge::LuaRef z) {
    if (!x.isNil()) {
        if (x.isNumber() && y.isNumber() && z.isNumber()) {
            m_SoundMusic->translate(x.cast<float>(), y.cast<float>(), z.cast<float>());
        } else if (!x.isNumber()) {
            m_SoundMusic->translate(x.cast<glm::vec3>());
        }
    }
}
float SoundMusicLUABinder::getVolume() const {
    return m_SoundMusic->getVolume();
}
void SoundMusicLUABinder::setVolume(float volume) {
    m_SoundMusic->setVolume(volume);
}
float SoundMusicLUABinder::getPitch() const {
    return m_SoundMusic->getPitch();
}
void SoundMusicLUABinder::setPitch(float pitch) {
    m_SoundMusic->setPitch(pitch);
}

#pragma endregion