#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/sounds/SoundData.h>

SoundData::SoundData(const std::string& file) 
    : Resource{ ResourceType::SoundData, file }
    , m_File{ file }
{
    buildBuffer();
}
SoundData::~SoundData() {
}
void SoundData::buildBuffer() {
    if (!m_Buffer) {
        m_Buffer = std::make_unique<sf::SoundBuffer>();
    }
    m_Buffer->loadFromFile(m_File);
}
float SoundData::getDuration() const {
    return m_Buffer->getDuration().asSeconds();
}
void SoundData::setVolume(float volume) {
    m_Volume = glm::clamp(volume, 0.0f, 100.0f);
}
