#include <core/engine/textures/TextureRequest.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/textures/TextureLoader.h>

#include <core/engine/system/Engine.h>

#include <boost/filesystem.hpp>

#include <iostream>

using namespace std;
using namespace Engine;
using namespace Engine::priv;

#pragma region TextureRequest

TextureRequest::TextureRequest() {
    file          = "";
    fileExtension = "";
    fileExists    = false;
    async         = false;
    selfClean     = true;
}
TextureRequest::TextureRequest(const string& _filename, const bool& genMipMaps, const ImageInternalFormat::Format& _internal, const GLuint& openglTextureType) : TextureRequest() {
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
TextureRequestFromMemory::TextureRequestFromMemory() {
    file = "";
    fileExtension = "";
    fileExists = false;
    async = false;
    selfClean = true;
    image = nullptr;
}
TextureRequestFromMemory::TextureRequestFromMemory(sf::Image& sfImage, const string& _filename, const bool& genMipMaps, const ImageInternalFormat::Format& _internal, const GLuint& openglTextureType) : TextureRequestFromMemory() {
    file = _filename;
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

            const auto lambda_cpu = [&]() {
                if (request.textureType == TextureType::Texture2D)
                    TextureLoader::InitFromFile(*request.part.texture, request.file, request.isToBeMipmapped, request.internalFormat, request.type);
                InternalTextureRequestPublicInterface::LoadCPU(request);
            };
            if (request.async) {
                const auto cbk = [&]() { InternalTextureRequestPublicInterface::LoadGPU(request); };
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
    if (request.selfClean && request.async)
        delete(&request); //yes its ugly, but its needed. see Resources::loadTextureAsync()
}


void InternalTextureRequestPublicInterface::RequestMem(TextureRequestFromMemory& request) {
    if (!request.file.empty()) {
        if (request.fileExists) {
            request.part.name    = request.file;
            request.part.texture = NEW Texture();
            request.part.texture->m_TextureType = request.textureType;
            request.part.texture->setName(request.part.name);
            request.part.handle  = Core::m_Engine->m_ResourceManager.m_Resources->add(request.part.texture, ResourceType::Texture);

            const auto lambda_cpu = [&]() {
                if (request.textureType == TextureType::Texture2D)
                    TextureLoader::InitFromMemory(*request.part.texture, std::ref(*request.image), request.file, request.isToBeMipmapped, request.internalFormat, request.type);
                InternalTextureRequestPublicInterface::LoadCPUMem(request);
            };
            if (request.async) {
                const auto cbk = [&]() { InternalTextureRequestPublicInterface::LoadGPUMem(request); };
                threading::addJobWithPostCallback(lambda_cpu, cbk);
            }else{
                lambda_cpu();
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
    if (request.selfClean && request.async)
        delete(&request); //yes its ugly, but its needed. see Resources::loadTextureAsync()
}