#include <core/engine/sounds/SoundMusic.h>
#include <core/engine/sounds/SoundData.h>
#include <core/engine/resources/Engine_Resources.h>

using namespace Engine;
using namespace std;


SoundMusic::SoundMusic(Handle& handle, const uint& numLoops) : SoundBaseClass(numLoops) {
    SoundData& data = *Resources::getSoundData(handle);
    bool res = m_Sound.openFromFile(data.getFilename());
    if (res) {
        setVolume(data.getVolume());
    }
    m_Duration = data.getDuration();
}
SoundMusic::~SoundMusic() {

}
const float& SoundMusic::getDuration() const {
    return m_Duration;
}
void SoundMusic::update(const double& dt) {
    const auto& sfStatus = m_Sound.getStatus();
    if (sfStatus != sf::SoundSource::Status::Stopped)
        return;
    if (m_Loops != 1 && m_Loops != 0) {//handle the looping logic
        if (getLoopsLeft() >= 2) {
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
const bool SoundMusic::play(const uint& numLoops) {
    if (status() == SoundStatus::Playing || status() == SoundStatus::PlayingLooped)
        return false;
    SoundBaseClass::play(numLoops);
    m_Sound.play();
    return true;
}
const bool SoundMusic::play() {
    if (status() == SoundStatus::Playing || status() == SoundStatus::PlayingLooped)
        return false;
    SoundBaseClass::play();
    m_Sound.play();
    return true;
}
const bool SoundMusic::pause() {
    if (status() == SoundStatus::Paused) 
        return false;
    SoundBaseClass::pause();
    m_Sound.pause();
    return true;
}
const bool SoundMusic::stop() {
    if (status() == SoundStatus::Stopped) 
        return false;
    SoundBaseClass::stop();
    m_Sound.stop();
    return true;
}
const bool SoundMusic::restart() {
    if (status() == SoundStatus::Stopped || status() == SoundStatus::Fresh || m_Sound.getPlayingOffset() == sf::Time::Zero)
        return false;
    m_Sound.setPlayingOffset(sf::Time::Zero);
    return true;
}

const float& SoundMusic::getAttenuation() const {
    return m_Sound.getAttenuation();
}
const glm::vec3& SoundMusic::getPosition() {
    sf::Vector3f pos = m_Sound.getPosition();
    return glm::vec3(pos.x, pos.y, pos.z);
}
void SoundMusic::setPosition(const float& x, const float& y, const float& z) {
    m_Sound.setPosition(x, y, z);
}
void SoundMusic::setPosition(const glm::vec3& position) {
    m_Sound.setPosition(position.x, position.y, position.z);
}
const float& SoundMusic::getVolume() const {
    return m_Sound.getVolume();
}
void SoundMusic::setVolume(const float& volume) {
    m_Sound.setVolume(volume);
}
const float& SoundMusic::getPitch() const {
    return m_Sound.getPitch();
}
void SoundMusic::setPitch(const float& pitch) {
    m_Sound.setPitch(pitch);
}