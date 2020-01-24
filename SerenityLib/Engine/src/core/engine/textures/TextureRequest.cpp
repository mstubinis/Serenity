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

TextureRequestPart::TextureRequestPart() {
    texture = nullptr;
    name    = "";
    handle  = Handle();
}
TextureRequestPart::~TextureRequestPart() {

}
TextureRequestPart::TextureRequestPart(const TextureRequestPart& other) {
    texture = other.texture;
    name    = other.name;
    handle  = other.handle;
}
TextureRequestPart& TextureRequestPart::operator=(const TextureRequestPart& other) {
    texture = other.texture;
    name    = other.name;
    handle  = other.handle;
    return *this;
}

#pragma endregion

#pragma region TextureRequest


TextureRequest::TextureRequest(const string& _filename, const bool& genMipMaps, const ImageInternalFormat::Format& _internal, const GLuint& openglTextureType){
    fileExtension = "";
    fileExists = false;
    async = false;
    
    file = _filename;
    if (!file.empty()) {
        fileExtension = boost::filesystem::extension(file);
        if (boost::filesystem::exists(file)) {
            fileExists = true;
        }
    }
    internalFormat  = _internal;
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
TextureRequest::~TextureRequest() {

}
TextureRequest::TextureRequest(const TextureRequest& other) {
    file            = other.file;
    fileExists      = other.fileExists;
    fileExtension   = other.fileExtension;
    internalFormat  = other.internalFormat;
    async           = other.async;
    isToBeMipmapped = other.isToBeMipmapped;
    textureType     = other.textureType;
    type            = other.type;
    part            = other.part;
}
TextureRequest& TextureRequest::operator=(const TextureRequest& other) {
    file            = other.file;
    fileExists      = other.fileExists;
    fileExtension   = other.fileExtension;
    internalFormat  = other.internalFormat;
    async           = other.async;
    isToBeMipmapped = other.isToBeMipmapped;
    textureType     = other.textureType;
    type            = other.type;
    part            = other.part;
    return *this;
}

void TextureRequest::request() {
    async = false;
    InternalTextureRequestPublicInterface::Request(*this);
}
void TextureRequest::requestAsync() {
    async = true;
    InternalTextureRequestPublicInterface::Request(*this);
}

#pragma endregion

#pragma region TextureRequestFromMemory


TextureRequestFromMemory::TextureRequestFromMemory(sf::Image& sfImage, const string& _filename, const bool& genMipMaps, const ImageInternalFormat::Format& _internal, const GLuint& openglTextureType){
    fileExtension = "";
    fileExists = false;
    async = false;
    
    file  = _filename;
    image = &sfImage;
    if (!file.empty()) {
        fileExtension = boost::filesystem::extension(file);
        if (boost::filesystem::exists(file)) {
            fileExists = true;
        }
    }
    internalFormat = _internal;
    isToBeMipmapped = genMipMaps;
    type = openglTextureType;

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
    file            = other.file;
    fileExists      = other.fileExists;
    fileExtension   = other.fileExtension;
    internalFormat  = other.internalFormat;
    async           = other.async;
    isToBeMipmapped = other.isToBeMipmapped;
    textureType     = other.textureType;
    type            = other.type;
    image           = other.image;
    part            = other.part;
}
TextureRequestFromMemory& TextureRequestFromMemory::operator=(const TextureRequestFromMemory& other) {
    file            = other.file;
    fileExists      = other.fileExists;
    fileExtension   = other.fileExtension;
    internalFormat  = other.internalFormat;
    async           = other.async;
    isToBeMipmapped = other.isToBeMipmapped;
    textureType     = other.textureType;
    type            = other.type;
    image           = other.image;
    part            = other.part;
    return *this;
}
void TextureRequestFromMemory::request() {
    async = false;
    InternalTextureRequestPublicInterface::RequestMem(*this);
}
void TextureRequestFromMemory::requestAsync() {
    async = true;
    InternalTextureRequestPublicInterface::RequestMem(*this);
}

#pragma endregion

void InternalTextureRequestPublicInterface::Request(TextureRequest& request) {
    if (!request.file.empty()) {
        if (request.fileExists) {

            request.part.name    = request.file;
            request.part.texture = NEW Texture();
            request.part.texture->m_TextureType = request.textureType;
            request.part.texture->setName(request.part.name);
            request.part.handle  = Core::m_Engine->m_ResourceManager.m_Resources->add(request.part.texture, ResourceType::Texture);

            const auto lambda_cpu = [=]() {
                if (request.textureType == TextureType::Texture2D) {
                    TextureLoader::InitFromFile(*request.part.texture, request.file, request.isToBeMipmapped, request.internalFormat, request.type);
                }
                InternalTextureRequestPublicInterface::LoadCPU(const_cast<TextureRequest&>(request));
            };
            if (request.async) {
                const auto cbk = [=]() { 
                    InternalTextureRequestPublicInterface::LoadGPU(const_cast<TextureRequest&>(request)); 
                };
                threading::addJobWithPostCallback(lambda_cpu, cbk);
            }else{
                lambda_cpu();
                InternalTextureRequestPublicInterface::LoadGPU(request);
            }
        }else{
            //we got either an invalid file or memory data
        }
    }
}
void InternalTextureRequestPublicInterface::LoadCPU(TextureRequest& request) {
    InternalTexturePublicInterface::LoadCPU(*request.part.texture);
}
void InternalTextureRequestPublicInterface::LoadGPU(TextureRequest& request) {
    InternalTexturePublicInterface::LoadGPU(*request.part.texture);
}


void InternalTextureRequestPublicInterface::RequestMem(TextureRequestFromMemory& request) {
    if (!request.file.empty()) {
        if (request.fileExists) {
            request.part.name    = request.file;
            request.part.texture = NEW Texture();
            request.part.texture->m_TextureType = request.textureType;
            request.part.texture->setName(request.part.name);
            request.part.handle  = Core::m_Engine->m_ResourceManager.m_Resources->add(request.part.texture, ResourceType::Texture);

            const auto lambda_cpu_mem = [=]() {
                if (request.textureType == TextureType::Texture2D) {
                    TextureLoader::InitFromMemory(*request.part.texture, *request.image, request.file, request.isToBeMipmapped, request.internalFormat, request.type);
                }
                InternalTextureRequestPublicInterface::LoadCPUMem(const_cast<TextureRequestFromMemory&>(request));
            };
            if (request.async) {
                const auto cbk = [=]() {
                    InternalTextureRequestPublicInterface::LoadGPUMem(const_cast<TextureRequestFromMemory&>(request));
                };
                threading::addJobWithPostCallback(lambda_cpu_mem, cbk);
            }else{
                lambda_cpu_mem();
                InternalTextureRequestPublicInterface::LoadGPUMem(request);
            }
        }else{
            //we got either an invalid file or memory data
        }
    }
}
void InternalTextureRequestPublicInterface::LoadCPUMem(TextureRequestFromMemory& request) {
    InternalTexturePublicInterface::LoadCPU(*request.part.texture);
}
void InternalTextureRequestPublicInterface::LoadGPUMem(TextureRequestFromMemory& request) {
    InternalTexturePublicInterface::LoadGPU(*request.part.texture);
}