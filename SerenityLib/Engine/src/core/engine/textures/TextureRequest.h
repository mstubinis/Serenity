#pragma once
#ifndef ENGINE_TEXTURE_TEXTURE_REQUEST_H
#define ENGINE_TEXTURE_TEXTURE_REQUEST_H

class Texture;

#include <SFML/Graphics/Image.hpp>
#include <core/engine/resources/Handle.h>
#include <core/engine/textures/TextureIncludes.h>

struct TextureRequestPart final {
    Texture*                     texture         = nullptr;
    Handle                       handle          = Handle();
    std::string                  name            = "";
    bool                         async           = false;
    GLuint                       type            = 0U;
    TextureType                  textureType     = TextureType::Unknown;
    bool                         isToBeMipmapped = false;
    ImageInternalFormat          internalFormat  = ImageInternalFormat::Unknown;

    TextureRequestPart() = default;
    ~TextureRequestPart() = default;

    TextureRequestPart(const TextureRequestPart& other)                = default;
    TextureRequestPart& operator=(const TextureRequestPart& other)     = default;
    TextureRequestPart(TextureRequestPart&& other) noexcept            = delete;
    TextureRequestPart& operator=(TextureRequestPart&& other) noexcept = delete;

    void assignType() {
        switch (type) {
            case GL_TEXTURE_2D: {
                textureType = TextureType::Texture2D; break;
            }case GL_TEXTURE_1D: {
                textureType = TextureType::Texture1D; break;
            }case GL_TEXTURE_3D: {
                textureType = TextureType::Texture3D; break;
            }case GL_TEXTURE_CUBE_MAP: {
                textureType = TextureType::CubeMap; break;
            }default: {
                textureType = TextureType::Texture2D; break;
            }
        }
    }
};

struct TextureRequest final {
    std::string           file              = "";
    std::string           fileExtension     = "";
    bool                  fileExists        = false;
    std::function<void()> m_Callback        = []() {};

    TextureRequestPart    part;

    TextureRequest(
        const std::string& filenameOrData,
        bool genMipMaps,
        ImageInternalFormat internal_ ,
        GLuint openglTextureType,
        std::function<void()>&& callback
    );
    TextureRequest(
        const std::string& filenameOrData,
        bool genMipMaps,
        ImageInternalFormat internal_,
        GLuint openglTextureType
    );
    ~TextureRequest() = default;

    void request(bool async = false);
};

struct TextureRequestFromMemory final {
    sf::Image              image;
    std::string            textureName  = "";
    std::function<void()>  m_Callback   = []() {};

    TextureRequestPart     part;

    TextureRequestFromMemory(
        sf::Image& sfImage,
        const std::string& textureName,
        bool genMipMaps,
        ImageInternalFormat internal_,
        GLuint openglTextureType,
        std::function<void()>&& callback_
    );
    ~TextureRequestFromMemory() = default;

    TextureRequestFromMemory(const TextureRequestFromMemory& other)                = default;
    TextureRequestFromMemory& operator=(const TextureRequestFromMemory& other)     = default;
    TextureRequestFromMemory(TextureRequestFromMemory&& other) noexcept            = delete;
    TextureRequestFromMemory& operator=(TextureRequestFromMemory&& other) noexcept = delete;

    void request(bool async = false);
};

namespace Engine::priv {
    struct TextureRequestStaticImpl final {
        friend class  Texture;
        static void Request(TextureRequest& request);
        static void Request(TextureRequestFromMemory& requestFromMemory);
    };
};


#endif