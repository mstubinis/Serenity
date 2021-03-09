#pragma once
#ifndef ENGINE_SOUND_DATA_H
#define ENGINE_SOUND_DATA_H

#include <serenity/resources/Resource.h>
#include <SFML/Audio.hpp>
#include <serenity/dependencies/glm.h>

class SoundData final : public Resource<SoundData> {
    private:
        std::string                       m_File;
        std::unique_ptr<sf::SoundBuffer>  m_Buffer;
        float                             m_Volume   = 100.0f;
    public:
        SoundData() = default;
        SoundData(const std::string& filename);

        SoundData(const SoundData&)            = delete;
        SoundData& operator=(const SoundData&) = delete;
        SoundData(SoundData&&) noexcept;
        SoundData& operator=(SoundData&&) noexcept;
        ~SoundData() = default;

        void buildBuffer();

        [[nodiscard]] inline float getDuration() const noexcept { return m_Buffer->getDuration().asSeconds(); }
        [[nodiscard]] inline sf::SoundBuffer* getBuffer() noexcept { return m_Buffer.get(); }
        [[nodiscard]] inline constexpr const std::string& getFilename() const noexcept { return m_File; }
        [[nodiscard]] inline constexpr float getVolume() const noexcept { return m_Volume; }
        inline void setVolume(float volume) noexcept { m_Volume = glm::clamp(volume, 0.0f, 100.0f); }
};

#endif