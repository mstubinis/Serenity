#include <core/engine/textures/TextureRequest.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/textures/TextureLoader.h>
#include <core/engine/system/window/Window.h>

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
    texture         = other.texture;
    name            = other.name;
    handle          = other.handle;
    async           = other.async;
    type            = other.type;
    textureType     = other.textureType;
    isToBeMipmapped = other.isToBeMipmapped;
    internalFormat  = other.internalFormat;
}
TextureRequestPart& TextureRequestPart::operator=(const TextureRequestPart& other) {
    if (&other != this) {
        texture         = other.texture;
        name            = other.name;
        handle          = other.handle;
        async           = other.async;
        type            = other.type;
        textureType     = other.textureType;
        isToBeMipmapped = other.isToBeMipmapped;
        internalFormat  = other.internalFormat;
    }
    return *this;
}
#pragma endregion

#pragma region TextureRequest

TextureRequest::TextureRequest(const string& filename, bool genMipMaps, ImageInternalFormat::Format internal_, GLuint openglTextureType) {
    file                 = filename;
    part.internalFormat  = internal_;
    part.isToBeMipmapped = genMipMaps;
    part.type            = openglTextureType;

    if (!file.empty()) {
        fileExtension = boost::filesystem::extension(file);
        if (boost::filesystem::exists(file)) {
            fileExists = true;
        }
    }
    part.assignType();
}
TextureRequest::~TextureRequest() {

}
void TextureRequest::request(bool inAsync) {
    if (inAsync && Engine::hardware_concurrency() > 1) {
        part.async = true;
    }else{
        part.async = false;
    }
    TextureRequestStaticImpl::Request(*this);
}
void TextureRequestStaticImpl::Request(TextureRequest& request) {
    if (!request.file.empty()) {
        if (request.fileExists) {
            request.part.name = request.file;
            request.part.texture = NEW Texture();
            request.part.texture->m_TextureType = request.part.textureType;
            request.part.texture->setName(request.part.name);
            request.part.handle = Core::m_Engine->m_ResourceManager.m_Resources.add(request.part.texture, ResourceType::Texture);

            auto lambda_cpu = [request]() {
                if (request.part.textureType == TextureType::Texture2D) {
                    TextureLoader::InitFromFile(*request.part.texture, request.file, request.part.isToBeMipmapped, request.part.internalFormat, request.part.type);
                }
                InternalTexturePublicInterface::LoadCPU(*request.part.texture);
            };
            if (request.part.async || std::this_thread::get_id() != Resources::getWindow().getOpenglThreadID()) {
                threading::addJobWithPostCallback(lambda_cpu, [request]() {
                    InternalTexturePublicInterface::LoadGPU(*request.part.texture);
                });
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

TextureRequestFromMemory::TextureRequestFromMemory(sf::Image& sfImage, const string& _filename, bool genMipMaps, ImageInternalFormat::Format internal_, GLuint openglTextureType){
    part.async           = false;
    textureName          = _filename;
    image                = sfImage;
    part.internalFormat  = internal_;
    part.isToBeMipmapped = genMipMaps;
    part.type            = openglTextureType;

    part.assignType();
}
TextureRequestFromMemory::~TextureRequestFromMemory() {

}
TextureRequestFromMemory::TextureRequestFromMemory(const TextureRequestFromMemory& other) {
    image            = other.image;
    textureName      = other.textureName;
    part             = other.part;
}
TextureRequestFromMemory& TextureRequestFromMemory::operator=(const TextureRequestFromMemory& other) {
    if (&other != this) {
        image           = other.image;
        textureName     = other.textureName;
        part            = other.part;
    }
    return *this;
}

void TextureRequestFromMemory::request(bool inAsync) {
    if (inAsync && Engine::hardware_concurrency() > 1) {
        part.async = true;
    }else{
        part.async = false;
    }
    TextureRequestStaticImpl::Request(*this);
}
void TextureRequestStaticImpl::Request(TextureRequestFromMemory& request) {
    auto imgSize = request.image.getSize();
    if (imgSize.x > 0 && imgSize.y > 0) {
        request.part.name = request.textureName;
        request.part.texture = NEW Texture();
        request.part.texture->m_TextureType = request.part.textureType;
        request.part.texture->setName(request.part.name);
        request.part.handle = Core::m_Engine->m_ResourceManager.m_Resources.add(request.part.texture, ResourceType::Texture);

        auto lambda_cpu = [request]() {
            if (request.part.textureType == TextureType::Texture2D) {
                TextureLoader::InitFromMemory(*request.part.texture, request.image, request.textureName, request.part.isToBeMipmapped, request.part.internalFormat, request.part.type);
            }
            InternalTexturePublicInterface::LoadCPU(*request.part.texture);
        };
        if (request.part.async || std::this_thread::get_id() != Resources::getWindow().getOpenglThreadID()) {
            threading::addJobWithPostCallback(lambda_cpu, [request]() {
                InternalTexturePublicInterface::LoadGPU(*request.part.texture);
            });
        }else{
            lambda_cpu();
            InternalTexturePublicInterface::LoadGPU(*request.part.texture);
        }
    }
}

#pragma endregion