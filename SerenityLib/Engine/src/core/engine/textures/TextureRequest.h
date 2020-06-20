#pragma once
#ifndef ENGINE_TEXTURE_TEXTURE_REQUEST_H
#define ENGINE_TEXTURE_TEXTURE_REQUEST_H

class Texture;

#include <SFML/Graphics/Image.hpp>
#include <vector>
#include <string>
#include <core/engine/resources/Handle.h>
#include <core/engine/textures/TextureIncludes.h>

struct TextureRequestPart final {
    Texture*                     texture         = nullptr;
    Handle                       handle          = Handle();
    std::string                  name            = "";
    bool                         async           = false;
    GLuint                       type            = 0U;
    TextureType::Type            textureType;
    bool                         isToBeMipmapped = false;
    ImageInternalFormat::Format  internalFormat;

    TextureRequestPart() = default;
    ~TextureRequestPart();

    TextureRequestPart(const TextureRequestPart&);
    TextureRequestPart& operator=(const TextureRequestPart&);
    TextureRequestPart(TextureRequestPart&& other) noexcept = delete;
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
    std::string         file              = "";
    std::string         fileExtension     = "";
    bool                fileExists        = false;

    TextureRequestPart  part;

    TextureRequest(
        const std::string& filenameOrData,
        bool genMipMaps = true,
        ImageInternalFormat::Format internal_ = ImageInternalFormat::Format::SRGB8_ALPHA8,
        GLuint openglTextureType = GL_TEXTURE_2D
    );
    ~TextureRequest();

    void request(bool async = false);
};

struct TextureRequestFromMemory final {
    sf::Image              image;
    std::string            textureName       = "";

    TextureRequestPart     part;

    TextureRequestFromMemory(
        sf::Image& sfImage,
        const std::string& textureName,
        bool genMipMaps = true,
        ImageInternalFormat::Format internal_ = ImageInternalFormat::Format::SRGB8_ALPHA8,
        GLuint openglTextureType = GL_TEXTURE_2D
    );
    ~TextureRequestFromMemory();

    TextureRequestFromMemory(const TextureRequestFromMemory&);
    TextureRequestFromMemory& operator=(const TextureRequestFromMemory&);
    TextureRequestFromMemory(TextureRequestFromMemory&& other) noexcept = delete;
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