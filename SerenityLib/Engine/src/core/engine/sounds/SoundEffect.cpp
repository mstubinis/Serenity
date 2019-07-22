#include <core/engine/sounds/SoundData.h>
#include <core/engine/sounds/SoundEffect.h>

#include <core/engine/resources/Handle.h>


SoundEffect::SoundEffect(Handle& handle, const uint& numLoops) : SoundBaseClass(numLoops) {
    SoundData* data = Engine::Resources::getSoundData(handle);
    auto buffer = data->getBuffer();
    if (!buffer) {
        data->buildBuffer();
    }
    m_Duration = data->getDuration();
    m_Sound.setBuffer(*buffer);
    setVolume(data->getVolume());
}
SoundEffect::SoundEffect(SoundData& soundData, const uint& numLoops) : SoundBaseClass(numLoops) {
    if (!soundData.getBuffer()) {
        soundData.buildBuffer();
    }
    m_Duration = soundData.getDuration();
    m_Sound.setBuffer(*(soundData.getBuffer()));
    setVolume(soundData.getVolume());
}
SoundEffect::~SoundEffect() {

}
void SoundEffect::update(const double& dt) {
    const auto& sfStatus = m_Sound.getStatus();
    if (sfStatus != sf::SoundSource::Status::Stopped)
        return;
    if (m_Loops != 1 && m_Loops != 0) {//handle the looping logic
        if (getLoopsLeft() >= 2) {
            m_CurrentLoop++;
            play(m_Loops); //apparently playing the sound when it is stopped restarts it (sfml internally)
        }else {
            stop();
        }
    }else if (m_Loops == 1) {//only once
        stop();
    }else {//endless loop (sound will have to be stoped manually by the user to end an endless loop)
        play(m_Loops); //apparently playing the sound when it is stopped restarts it (sfml internally)
    }
}
const float& SoundEffect::getDuration() const {
    return m_Duration;
}
const bool SoundEffect::play(const uint& numLoops) {
    auto& _status = status();
    if (_status == SoundStatus::Playing || _status == SoundStatus::PlayingLooped)
        return false;
    SoundBaseClass::play(numLoops);
    m_Sound.play();
}
const bool SoundEffect::play() {
    auto& _status = status();
    if (_status == SoundStatus::Playing || _status == SoundStatus::PlayingLooped)
        return false;
    SoundBaseClass::play();
    m_Sound.play();
}
const bool SoundEffect::pause() {
    if (status() == SoundStatus::Paused)
        return false;
    SoundBaseClass::pause();
    m_Sound.pause();
    return true;
}
const bool SoundEffect::stop() {
    if (status() == SoundStatus::Stopped)
        return false;
    SoundBaseClass::stop();
    m_Sound.stop();
    return true;
}
const bool SoundEffect::restart() {
    auto& _status = status();
    if (_status == SoundStatus::Stopped || _status == SoundStatus::Fresh || m_Sound.getPlayingOffset() == sf::Time::Zero)
        return false;
    m_Sound.setPlayingOffset(sf::Time::Zero);
    return true;
}

const float& SoundEffect::getAttenuation() const {
    return m_Sound.getAttenuation();
}
const glm::vec3& SoundEffect::getPosition() {
    const sf::Vector3f& v = m_Sound.getPosition();
    return glm::vec3(v.x, v.y, v.z);
}
void SoundEffect::setPosition(const float& x, const float& y, const float& z) {
    m_Sound.setPosition(x, y, z);
}
void SoundEffect::setPosition(const glm::vec3& position) {
    m_Sound.setPosition(position.x, position.y, position.z);
}
const float& SoundEffect::getVolume() const {
    return m_Sound.getVolume();
}
void SoundEffect::setVolume(const float& volume) {
    m_Sound.setVolume(volume);
}
const float& SoundEffect::getPitch() const {
    return m_Sound.getPitch();
}
void SoundEffect::setPitch(const float& pitch) {
    m_Sound.setPitch(pitch);
}