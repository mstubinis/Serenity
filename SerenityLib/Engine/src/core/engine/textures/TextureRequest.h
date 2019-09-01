#pragma once
#ifndef ENGINE_TEXTURE_TEXTURE_REQUEST_H
#define ENGINE_TEXTURE_TEXTURE_REQUEST_H

#include <vector>
#include <string>
#include <core/engine/resources/Handle.h>
#include <core/engine/textures/TextureIncludes.h>

class Texture;
struct TextureRequestPart final {
    Texture*     texture;
    Handle       handle;
    std::string  name;

    TextureRequestPart() {
        texture = nullptr;
        name = "";
        handle = Handle();
    }
    ~TextureRequestPart() {
    }
};

struct TextureRequest final {
    std::string          file;
    std::string          fileExtension;
    bool                 fileExists;
    TextureRequestPart   part;
    bool                 async;
    bool                 selfClean;


    GLuint                       type;
    TextureType::Type            textureType;
    bool                         isToBeMipmapped;
    ImageInternalFormat::Format  internalFormat;

    TextureRequest();
    TextureRequest(const std::string& filenameOrData, const bool& genMipMaps = true, const ImageInternalFormat::Format& _internal = ImageInternalFormat::Format::SRGB8_ALPHA8, const GLuint& openglTextureType = GL_TEXTURE_2D);
    ~TextureRequest();

    void request();
    void requestAsync();
};

namespace Engine {
    namespace epriv {
        struct InternalTextureRequestPublicInterface final {
            friend class  Texture;
            static void Request(TextureRequest&);
            static void LoadGPU(TextureRequest&);
            static void LoadCPU(TextureRequest&);
        };
    };
};


#endif