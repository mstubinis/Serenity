#include <core/engine/textures/TextureRequest.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/textures/TextureLoader.h>

#include <core/engine/system/Engine.h>

#include <boost/filesystem.hpp>

#include <iostream>

using namespace std;
using namespace Engine;
using namespace Engine::priv;

#pragma region TextureRequestPart

TextureRequestPart::~TextureRequestPart() {
}
TextureRequestPart::TextureRequestPart(const TextureRequestPart& other) {
    texture = other.texture;
    name    = other.name;
    handle  = other.handle;
}
TextureRequestPart& TextureRequestPart::operator=(const TextureRequestPart& other) {
    if (&other != this) {
        texture = other.texture;
        name = other.name;
        handle = other.handle;
    }
    return *this;
}
#pragma endregion

#pragma region TextureRequest

TextureRequest::TextureRequest(const string& filename, const bool genMipMaps, const ImageInternalFormat::Format internal_, const GLuint openglTextureType) {
    file            = filename;
    internalFormat  = internal_;
    isToBeMipmapped = genMipMaps;
    type            = openglTextureType;

    if (!file.empty()) {
        fileExtension = boost::filesystem::extension(file);
        if (boost::filesystem::exists(file)) {
            fileExists = true;
        }
    }
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
TextureRequest::~TextureRequest() {

}
void TextureRequest::request() {
    async = false;
    TextureRequestStaticImpl::Request(*this);
}
void TextureRequest::requestAsync() {
    if (Engine::hardware_concurrency() > 1) {
        async = true;
        TextureRequestStaticImpl::Request(*this);
    }else{
        TextureRequest::request();
    }
}
void TextureRequestStaticImpl::Request(TextureRequest& request) {
    if (!request.file.empty()) {
        if (request.fileExists) {

            request.part.name = request.file;
            request.part.texture = NEW Texture();
            request.part.texture->m_TextureType = request.textureType;
            request.part.texture->setName(request.part.name);
            request.part.handle = Core::m_Engine->m_ResourceManager.m_Resources.add(request.part.texture, ResourceType::Texture);

            const auto lambda_cpu = [request]() {
                if (request.textureType == TextureType::Texture2D) {
                    TextureLoader::InitFromFile(*request.part.texture, request.file, request.isToBeMipmapped, request.internalFormat, request.type);
                }
                InternalTexturePublicInterface::LoadCPU(*request.part.texture);
            };
            if (request.async) {
                const auto cbk = [request]() {
                    InternalTexturePublicInterface::LoadGPU(*request.part.texture);
                };
                threading::addJobWithPostCallback(lambda_cpu, cbk);
            }else{
                lambda_cpu();
                InternalTexturePublicInterface::LoadGPU(*request.part.texture);
            }
        }else{
            //we got either an invalid file or memory data
        }
    }
}

#pragma endregion

#pragma region TextureRequestFromMemory

TextureRequestFromMemory::TextureRequestFromMemory(sf::Image& sfImage, const string& _filename, const bool genMipMaps, const ImageInternalFormat::Format internal_, const GLuint openglTextureType){
    async           = false;
    textureName     = _filename;
    image           = sfImage;
    internalFormat  = internal_;
    isToBeMipmapped = genMipMaps;
    type            = openglTextureType;
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
TextureRequestFromMemory::~TextureRequestFromMemory() {

}
TextureRequestFromMemory::TextureRequestFromMemory(const TextureRequestFromMemory& other) {
    image            = other.image;
    textureName      = other.textureName;
    part             = other.part;
    async            = other.async;
    type             = other.type;
    textureType      = other.textureType;
    isToBeMipmapped  = other.isToBeMipmapped;
    internalFormat   = other.internalFormat;
}
TextureRequestFromMemory& TextureRequestFromMemory::operator=(const TextureRequestFromMemory& other) {
    if (&other != this) {
        image           = other.image;
        textureName     = other.textureName;
        part            = other.part;
        async           = other.async;
        type            = other.type;
        textureType     = other.textureType;
        isToBeMipmapped = other.isToBeMipmapped;
        internalFormat  = other.internalFormat;
    }
    return *this;
}

void TextureRequestFromMemory::request() {
    async = false;
    TextureRequestStaticImpl::RequestMem(*this);
}
void TextureRequestFromMemory::requestAsync() {
    if (Engine::hardware_concurrency() > 1) {
        async = true;
        TextureRequestStaticImpl::RequestMem(*this);
    }else{
        TextureRequestFromMemory::request();
    }
}
void TextureRequestStaticImpl::RequestMem(TextureRequestFromMemory& request) {
    auto imgSize = request.image.getSize();
    if (imgSize.x > 0 && imgSize.y > 0) {
        request.part.name = request.textureName;
        request.part.texture = NEW Texture();
        request.part.texture->m_TextureType = request.textureType;
        request.part.texture->setName(request.part.name);
        request.part.handle = Core::m_Engine->m_ResourceManager.m_Resources.add(request.part.texture, ResourceType::Texture);

        const auto lambda_cpu = [request]() {
            if (request.textureType == TextureType::Texture2D) {
                TextureLoader::InitFromMemory(*request.part.texture, request.image, request.textureName, request.isToBeMipmapped, request.internalFormat, request.type);
            }
            InternalTexturePublicInterface::LoadCPU(*request.part.texture);
        };
        if (request.async) {
            const auto cbk = [request]() {
                InternalTexturePublicInterface::LoadGPU(*request.part.texture);
            };
            threading::addJobWithPostCallback(lambda_cpu, cbk);
        }else{
            lambda_cpu();
            InternalTexturePublicInterface::LoadGPU(*request.part.texture);
        }
    }
}

#pragma endregion