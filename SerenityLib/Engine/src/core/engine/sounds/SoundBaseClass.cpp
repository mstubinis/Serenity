#include <core/engine/sounds/SoundBaseClass.h>



SoundBaseClass::SoundBaseClass(const uint& numLoops) {
    m_Status      = SoundStatus::Fresh;
    m_Loops       = numLoops;
    m_CurrentLoop = 0;
}
SoundBaseClass::~SoundBaseClass() {
    m_Status = SoundStatus::Stopped;
}
const SoundStatus::Status& SoundBaseClass::status() {
    return m_Status;
}
void SoundBaseClass::update(const double& dt) {

}
const bool SoundBaseClass::play(const uint& numLoops) {
    numLoops != 1 ? m_Status = SoundStatus::PlayingLooped : m_Status = SoundStatus::Playing;
    m_Loops = numLoops;
    return false;
}
const bool SoundBaseClass::play() {
    m_Loops != 1 ? m_Status = SoundStatus::PlayingLooped : m_Status = SoundStatus::Playing;
    return false;
}
const bool SoundBaseClass::pause() {
    m_Status = SoundStatus::Paused;
    return false;
}
const bool SoundBaseClass::stop() {
    m_Status = SoundStatus::Stopped;
    return false;
}
const bool SoundBaseClass::restart() {
    return false;
}
const uint& SoundBaseClass::getLoopsLeft() {
    return m_Loops - m_CurrentLoop;
}

const float SoundBaseClass::getAttenuation() {
    return 0;
}
const glm::vec3& SoundBaseClass::getPosition() {
    return glm::vec3(0.0f);
}
void SoundBaseClass::setPosition(const float& x, const float& y, const float& z) {

}
void SoundBaseClass::setPosition(const glm::vec3& position) {

}
const float SoundBaseClass::getVolume() {
    return 0;
}
void SoundBaseClass::setVolume(const float& volume) {

}
const float SoundBaseClass::getPitch() {
    return 0;
}
void SoundBaseClass::setPitch(const float& pitch) {

}