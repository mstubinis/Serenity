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
    Texture*     texture;
    Handle       handle;
    std::string  name;

    TextureRequestPart();
    ~TextureRequestPart();

    TextureRequestPart(const TextureRequestPart&);
    TextureRequestPart& operator=(const TextureRequestPart&);
    TextureRequestPart(TextureRequestPart&& other) noexcept = delete;
    TextureRequestPart& operator=(TextureRequestPart&& other) noexcept = delete;
};
struct TextureRequest final {
    std::string          file;
    std::string          fileExtension;
    bool                 fileExists;
    TextureRequestPart   part;
    bool                 async;


    GLuint                       type;
    TextureType::Type            textureType;
    bool                         isToBeMipmapped;
    ImageInternalFormat::Format  internalFormat;

    TextureRequest(
        const std::string& filenameOrData,
        const bool& genMipMaps = true,
        const ImageInternalFormat::Format& _internal = ImageInternalFormat::Format::SRGB8_ALPHA8,
        const GLuint& openglTextureType = GL_TEXTURE_2D
    );
    ~TextureRequest();

    void request();
    void requestAsync();
};

struct TextureRequestFromMemory final {
    sf::Image            image;
    std::string          textureName;
    TextureRequestPart   part;
    bool                 async;

    GLuint                       type;
    TextureType::Type            textureType;
    bool                         isToBeMipmapped;
    ImageInternalFormat::Format  internalFormat;

    TextureRequestFromMemory(
        sf::Image& sfImage,
        const std::string& textureName,
        const bool& genMipMaps = true,
        const ImageInternalFormat::Format& _internal = ImageInternalFormat::Format::SRGB8_ALPHA8,
        const GLuint& openglTextureType = GL_TEXTURE_2D
    );
    ~TextureRequestFromMemory();

    TextureRequestFromMemory(const TextureRequestFromMemory&);
    TextureRequestFromMemory& operator=(const TextureRequestFromMemory&);
    TextureRequestFromMemory(TextureRequestFromMemory&& other) noexcept = delete;
    TextureRequestFromMemory& operator=(TextureRequestFromMemory&& other) noexcept = delete;

    void request();
    void requestAsync();
};

namespace Engine::priv {
    struct InternalTextureRequestPublicInterface final {
        friend class  Texture;
        static void Request(TextureRequest&);
        static void LoadGPU(TextureRequest&);
        static void LoadCPU(TextureRequest&);

        static void RequestMem(TextureRequestFromMemory&);
        static void LoadGPUMem(TextureRequestFromMemory&);
        static void LoadCPUMem(TextureRequestFromMemory&);
    };
};


#endif