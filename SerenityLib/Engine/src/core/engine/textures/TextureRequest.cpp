#include <core/engine/textures/TextureRequest.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/textures/TextureLoader.h>

#include <core/engine/Engine.h>

#include <boost/filesystem.hpp>

#include <iostream>

using namespace std;
using namespace Engine;
using namespace Engine::epriv;

TextureRequest::TextureRequest() {
    file          = "";
    fileExtension = "";
    fileExists    = false;
    async         = false;
    selfClean     = true;
}
TextureRequest::TextureRequest(const string& _filename, const bool& genMipMaps, const ImageInternalFormat::Format& _internal, const GLuint& openglTextureType) :TextureRequest() {
    file = _filename;
    if (!file.empty()) {
        fileExtension = boost::filesystem::extension(file);
        if (boost::filesystem::exists(file)) {
            fileExists = true;
        }
    }
    internalFormat = _internal;
    isToBeMipmapped = genMipMaps;
    type = openglTextureType;

    if (type == GL_TEXTURE_2D)
        textureType = TextureType::Texture2D;
    else if (type == GL_TEXTURE_1D)
        textureType = TextureType::Texture1D;
    else if (type == GL_TEXTURE_3D)
        textureType = TextureType::Texture3D;
    else if (type == GL_TEXTURE_CUBE_MAP)
        textureType = TextureType::CubeMap;
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

void InternalTextureRequestPublicInterface::Request(TextureRequest& request) {
    if (!request.file.empty()) {
        if (request.fileExists) {

            request.part.name = request.file;
            request.part.texture = new Texture();
            request.part.texture->m_TextureType = request.textureType;
            request.part.texture->setName(request.part.name);
            request.part.handle = Core::m_Engine->m_ResourceManager.m_Resources->add(request.part.texture, ResourceType::Texture);

            auto lambda_cpu = [&]() {
                if (request.textureType == TextureType::Texture2D)
                    TextureLoader::InitFromFile(*request.part.texture, request.file, request.isToBeMipmapped, request.internalFormat, request.type);
                InternalTextureRequestPublicInterface::LoadCPU(request);
            };

            if (request.async) {
                const auto& reference = std::ref(request);
                const auto& job = std::bind(lambda_cpu);
                const auto& cbk = std::bind(&InternalTextureRequestPublicInterface::LoadGPU, reference);
                threading::addJobWithPostCallback(job, cbk);
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