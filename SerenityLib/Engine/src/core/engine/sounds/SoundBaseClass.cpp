#include <core/engine/sounds/SoundBaseClass.h>

SoundBaseClass::SoundBaseClass(const unsigned int numLoops) {
    m_Status      = SoundStatus::Fresh;
    m_Loops       = numLoops;
    m_CurrentLoop = 0;
}
SoundBaseClass::~SoundBaseClass() {
    m_Status = SoundStatus::Stopped;
}
SoundStatus::Status SoundBaseClass::status() {
    return m_Status;
}
void SoundBaseClass::update(const float dt) {
}
bool SoundBaseClass::play(const unsigned int numLoops) {
    return false;
}
bool SoundBaseClass::pause() {
    return false;
}
bool SoundBaseClass::stop() {
    return false;
}
bool SoundBaseClass::restart() {
    return false;
}
unsigned int SoundBaseClass::getLoopsLeft() {
    return m_Loops - m_CurrentLoop;
}
float SoundBaseClass::getAttenuation() {
    return 0;
}
unsigned int SoundBaseClass::getChannelCount() {
    return 0;
}
void SoundBaseClass::setAttenuation(const float attenuation) {

}
float SoundBaseClass::getMinDistance() {
    return 0.0f;
}
void SoundBaseClass::setMinDistance(const float minDistance) {

}
bool SoundBaseClass::isRelativeToListener() {
    return false;
}
void SoundBaseClass::setRelativeToListener(const bool relative) {

}
glm::vec3 SoundBaseClass::getPosition() {
    return glm::vec3(0.0f);
}
void SoundBaseClass::setPosition(const float x, const float y, const float z) {
}
void SoundBaseClass::setPosition(const glm::vec3& position) {
}
float SoundBaseClass::getVolume() {
    return 0;
}
void SoundBaseClass::setVolume(const float volume) {
}
float SoundBaseClass::getPitch() {
    return 0;
}
void SoundBaseClass::setPitch(const float pitch) {
}