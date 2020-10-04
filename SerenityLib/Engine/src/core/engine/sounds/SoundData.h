#pragma once
#ifndef ENGINE_SOUND_DATA_H
#define ENGINE_SOUND_DATA_H

#include <core/engine/resources/Resource.h>
#include <SFML/Audio.hpp>

class SoundData final : public Resource {
    private:
        std::string                       m_File = "";
        std::unique_ptr<sf::SoundBuffer>  m_Buffer;
        float                             m_Volume   = 100.0f;
    public:
        SoundData() = default;
        SoundData(const std::string& filename);

        SoundData(const SoundData& other)            = delete;
        SoundData& operator=(const SoundData& other) = delete;
        SoundData(SoundData&& other) noexcept;
        SoundData& operator=(SoundData&& other) noexcept;
        ~SoundData() = default;

        void buildBuffer();

        inline float getDuration() const noexcept { return m_Buffer->getDuration().asSeconds(); }
        inline CONSTEXPR sf::SoundBuffer* getBuffer() noexcept { return m_Buffer.get(); }
        inline CONSTEXPR const std::string& getFilename() const noexcept { return m_File; }
        inline CONSTEXPR float getVolume() const noexcept { return m_Volume; }
        inline void setVolume(float volume) noexcept { m_Volume = glm::clamp(volume, 0.0f, 100.0f); }
};

#endif