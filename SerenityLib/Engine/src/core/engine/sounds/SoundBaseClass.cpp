#include "core/engine/utils/PrecompiledHeader.h"
#include <core/engine/sounds/SoundBaseClass.h>

SoundBaseClass::SoundBaseClass(unsigned int numLoops) {
    m_Loops       = numLoops;
}
SoundBaseClass::~SoundBaseClass() {
    m_Status = SoundStatus::Stopped;
}
SoundStatus::Status SoundBaseClass::status() const {
    return m_Status;
}
void SoundBaseClass::update(const float dt) {
}
bool SoundBaseClass::play(unsigned int numLoops) {
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
unsigned int SoundBaseClass::getLoopsLeft() const {
    return m_Loops - m_CurrentLoop;
}
float SoundBaseClass::getAttenuation() const {
    return 0.0f;
}
unsigned int SoundBaseClass::getChannelCount() const {
    return 0U;
}
void SoundBaseClass::setAttenuation(float attenuation) {

}
float SoundBaseClass::getMinDistance() const {
    return 0.0f;
}
void SoundBaseClass::setMinDistance(float minDistance) {

}
bool SoundBaseClass::isRelativeToListener() const {
    return false;
}
void SoundBaseClass::setRelativeToListener(bool relative) {

}
glm::vec3 SoundBaseClass::getPosition() const {
    return glm::vec3(0.0f);
}
void SoundBaseClass::setPosition(float x, float y, float z) {
}
void SoundBaseClass::setPosition(const glm::vec3& position) {
}
float SoundBaseClass::getVolume() const {
    return 0.0f;
}
void SoundBaseClass::setVolume(float volume) {
}
float SoundBaseClass::getPitch() const {
    return 0.0f;
}
void SoundBaseClass::setPitch(float pitch) {
}