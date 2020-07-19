#pragma once
#ifndef ENGINE_SOUND_DATA_H
#define ENGINE_SOUND_DATA_H

#include <core/engine/resources/Resource.h>
#include <SFML/Audio.hpp>

class SoundData final : public Resource {
    private:
        sf::SoundBuffer*  m_Buffer   = nullptr;
        std::string       m_File     = "";
        float             m_Volume   = 100.0f;
    public:
        SoundData(const std::string& filename);
        ~SoundData();

        float getDuration() const;
        void buildBuffer();
        inline constexpr sf::SoundBuffer* getBuffer() noexcept { return m_Buffer; }
        inline constexpr const std::string& getFilename() const noexcept { return m_File; }
        inline constexpr float getVolume() const noexcept { return m_Volume; }
        void setVolume(float volume);
};

#endif