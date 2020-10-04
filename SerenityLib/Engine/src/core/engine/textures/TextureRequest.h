#pragma once
#ifndef ENGINE_TEXTURE_TEXTURE_REQUEST_H
#define ENGINE_TEXTURE_TEXTURE_REQUEST_H

class Texture;

#include <SFML/Graphics/Image.hpp>
#include <core/engine/resources/Handle.h>
#include <core/engine/textures/TextureIncludes.h>

struct TextureRequestPart final {
    std::string                  fileOrTextureName = "";
    Handle                       handle            = Handle();
    TextureType                  textureType       = TextureType::Unknown;
    ImageInternalFormat          internalFormat    = ImageInternalFormat::Unknown;
    bool                         async             = false;
    bool                         isToBeMipmapped   = false;
    std::function<void()>        m_Callback        = []() {};
};

struct TextureRequest final {
    struct FileData final {
        std::string           m_FileExtension = "";
        bool                  m_FileExists = false;
    };

    TextureRequestPart        m_Part;
    bool                      m_FromMemory = false;
    //union {
        FileData              m_FileData;
        sf::Image             m_SFMLImage;
    //};

    TextureRequest() = delete;
    TextureRequest(const std::string& filenameOrData, bool genMipMaps, ImageInternalFormat, TextureType, std::function<void()>&& callback);
    TextureRequest(const std::string& filenameOrData, bool genMipMaps, ImageInternalFormat, TextureType);
    TextureRequest(const sf::Image&, const std::string& textureName, bool genMipMaps, ImageInternalFormat, TextureType, std::function<void()>&& callback);

    TextureRequest(const TextureRequest& other);
    TextureRequest& operator=(const TextureRequest& other);
    TextureRequest(TextureRequest&& other) noexcept;
    TextureRequest& operator=(TextureRequest&& other) noexcept;
    ~TextureRequest() {}

    inline constexpr bool isFromFile() const noexcept { return (!m_FromMemory && m_FileData.m_FileExists); }
    inline constexpr bool isFromMemory() const noexcept { return (m_FromMemory && m_SFMLImage.getSize().x > 0 && m_SFMLImage.getSize().y > 0); }

    void request(bool async = false);
};

namespace Engine::priv {
    struct TextureRequestStaticImpl final {
        friend class  Texture;
        static void Request(TextureRequest& request);
    };
};


#endif