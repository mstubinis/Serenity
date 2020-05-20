#pragma once
#ifndef ENGINE_SOUND_DATA_H
#define ENGINE_SOUND_DATA_H

#include <core/engine/resources/Engine_ResourceBasic.h>
#include <SFML/Audio.hpp>

class SoundData final : public EngineResource {
    private:
        sf::SoundBuffer*  m_Buffer   = nullptr;
        std::string       m_File     = "";
        float             m_Volume   = 100.0f;
    public:
        SoundData(const std::string& filename);
        ~SoundData();

        float getDuration() const;
        void buildBuffer();
        sf::SoundBuffer* getBuffer();
        const std::string& getFilename();
        float getVolume() const;
        void setVolume(const float volume);
};

#endif