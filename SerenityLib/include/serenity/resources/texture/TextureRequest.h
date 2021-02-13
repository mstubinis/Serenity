#pragma once
#ifndef ENGINE_TEXTURE_TEXTURE_REQUEST_H
#define ENGINE_TEXTURE_TEXTURE_REQUEST_H

class Texture;

#include <SFML/Graphics/Image.hpp>
#include <serenity/resources/texture/TextureIncludes.h>
#include <serenity/resources/texture/Texture.h>
#include <serenity/types/Types.h>

struct TextureRequestPart final {
    Engine::priv::TextureCPUData  m_CPUData;
    Engine::ResourceCallback      m_Callback  = [](Handle) {};
    Handle                        m_Handle      = Handle{};
    bool                          m_Async       = false;

    TextureRequestPart() = default;
    TextureRequestPart(const std::string& filename, ImageInternalFormat intFmt, bool mipmapped, TextureType textureType_) {
        m_CPUData.m_Name                            = filename;
        m_CPUData.m_IsToBeMipmapped                 = mipmapped;
        m_CPUData.m_TextureType                     = textureType_;
        m_CPUData.m_ImagesDatas[0].m_InternalFormat = intFmt;
        m_CPUData.m_ImagesDatas[0].m_Filename       = filename;
    }
};

struct TextureRequest final {
    struct FileData final {
        std::string           m_FileExtension;
        bool                  m_FileExists    = false;
    };

    TextureRequestPart        m_Part;
    bool                      m_FromMemory    = false;

    //either one or the other, using both at once won't happen
    FileData                  m_FileData;
    sf::Image                 m_SFMLImage;

    TextureRequest() = delete;
    TextureRequest(const std::string& filenameOrData, bool genMipMaps, ImageInternalFormat, TextureType, Engine::ResourceCallback&& callback);
    TextureRequest(const std::string& filenameOrData, bool genMipMaps, ImageInternalFormat, TextureType);
    TextureRequest(const sf::Image&, const std::string& textureName, bool genMipMaps, ImageInternalFormat, TextureType, Engine::ResourceCallback&& callback);

    [[nodiscard]] inline constexpr bool isFromFile() const noexcept { return (!m_FromMemory && m_FileData.m_FileExists); }
    [[nodiscard]] inline constexpr bool isFromMemory() const noexcept { return (m_FromMemory && m_SFMLImage.getSize().x > 0 && m_SFMLImage.getSize().y > 0); }

    void request(bool async = false);
};

#endif