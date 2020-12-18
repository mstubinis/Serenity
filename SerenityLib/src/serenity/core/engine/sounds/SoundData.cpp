
#include <serenity/core/engine/sounds/SoundData.h>

SoundData::SoundData(const std::string& file) 
    : Resource{ ResourceType::SoundData, file }
    , m_File{ file }
{
    buildBuffer();
}
SoundData::SoundData(SoundData&& other) noexcept 
    : Resource(std::move(other))
    , m_Buffer {std::move(other.m_Buffer)}
    , m_File   { std::move(other.m_File) }
    , m_Volume { std::move(other.m_Volume) }
{}
SoundData& SoundData::operator=(SoundData&& other) noexcept {
    Resource::operator=(std::move(other));
    m_Buffer = std::move(other.m_Buffer);
    m_File   = std::move(other.m_File);
    m_Volume = std::move(other.m_Volume);
    return *this;
}
void SoundData::buildBuffer() {
    if (!m_Buffer) {
        m_Buffer = std::make_unique<sf::SoundBuffer>();
    }
    m_Buffer->loadFromFile(m_File);
}