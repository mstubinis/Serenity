#include <core/engine/sounds/SoundData.h>

using namespace std;

SoundData::SoundData(const string& file) {
    m_Buffer = nullptr;
    m_File   = file;
    m_Volume = 100;
    buildBuffer();
}
SoundData::~SoundData() {
    SAFE_DELETE(m_Buffer);
}
void SoundData::buildBuffer() {
    if (!m_Buffer) {
        m_Buffer = new sf::SoundBuffer();
    }
    m_Buffer->loadFromFile(m_File);
}
const float& SoundData::getDuration() const {
    return m_Buffer->getDuration().asSeconds();
}
sf::SoundBuffer* SoundData::getBuffer() {
    return m_Buffer;
}
const std::string& SoundData::getFilename() {
    return m_File;
}
const float& SoundData::getVolume() const {
    return m_Volume;
}
void SoundData::setVolume(const float& volume) {
    m_Volume = glm::clamp(volume, 0.0f, 100.0f);
}
