#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/textures/TextureRequest.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/textures/TextureLoader.h>
#include <core/engine/system/window/Window.h>
#include <core/engine/system/Engine.h>

#include <boost/filesystem.hpp>

#pragma region TextureRequest
TextureRequest::TextureRequest(const std::string& filename, bool genMipMaps, ImageInternalFormat internal_, GLuint openglTextureType) 
    : file{ filename }
{
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
TextureRequest::TextureRequest(const std::string& filename, bool genMipMaps, ImageInternalFormat internal_, GLuint openglTextureType, std::function<void()>&& callback_)
    : TextureRequest{ filename, genMipMaps, internal_, openglTextureType }
{
    m_Callback           = std::move(callback_);
}
void TextureRequest::request(bool inAsync) {
    if (inAsync && Engine::hardware_concurrency() > 1) {
        part.async = true;
    }else{
        part.async = false;
    }
    Engine::priv::TextureRequestStaticImpl::Request(*this);
}
void Engine::priv::TextureRequestStaticImpl::Request(TextureRequest& request) {
    if (!request.file.empty()) {
        if (request.fileExists) {
            request.part.name = request.file;
            request.part.texture = NEW Texture();
            request.part.texture->m_TextureType = request.part.textureType;
            request.part.texture->setName(request.part.name);
            request.part.handle = Engine::priv::Core::m_Engine->m_ResourceManager.m_ResourcePool.add(request.part.texture, (unsigned int)ResourceType::Texture);

            auto lambda_cpu = [request]() {
                if (request.part.textureType == TextureType::Texture2D) {
                    Engine::priv::TextureLoader::InitFromFile(
                        *request.part.texture, 
                        request.file,
                        request.part.isToBeMipmapped, 
                        request.part.internalFormat, 
                        request.part.type
                    );
                }
                Engine::priv::InternalTexturePublicInterface::LoadCPU(*request.part.texture);
            };
            if (request.part.async || std::this_thread::get_id() != Engine::Resources::getWindow().getOpenglThreadID()) {
                Engine::priv::threading::addJobWithPostCallback(lambda_cpu, [request]() {
                    Engine::priv::InternalTexturePublicInterface::LoadGPU(*request.part.texture);
                    request.m_Callback();
                });
            }else{
                lambda_cpu();
                Engine::priv::InternalTexturePublicInterface::LoadGPU(*request.part.texture);
                request.m_Callback();
            }
        }else{
            //we got either an invalid file or memory data
        }
    }
}

#pragma endregion

#pragma region TextureRequestFromMemory

TextureRequestFromMemory::TextureRequestFromMemory(sf::Image& sfImage, const std::string& filename_, bool genMipMaps, ImageInternalFormat internal_, GLuint openglTextureType, std::function<void()>&& callback_)
    : image{ sfImage }
    , textureName{ filename_ }
    , m_Callback{ std::move(callback_) }
{
    part.async           = false;
    part.internalFormat  = internal_;
    part.isToBeMipmapped = genMipMaps;
    part.type            = openglTextureType;
    part.assignType();
}
void TextureRequestFromMemory::request(bool inAsync) {
    if (inAsync && Engine::hardware_concurrency() > 1) {
        part.async = true;
    }else{
        part.async = false;
    }
    Engine::priv::TextureRequestStaticImpl::Request(*this);
}
void Engine::priv::TextureRequestStaticImpl::Request(TextureRequestFromMemory& request) {
    auto imgSize = request.image.getSize();
    if (imgSize.x > 0 && imgSize.y > 0) {
        request.part.name = request.textureName;
        request.part.texture = NEW Texture();
        request.part.texture->m_TextureType = request.part.textureType;
        request.part.texture->setName(request.part.name);
        request.part.handle = Core::m_Engine->m_ResourceManager.m_ResourcePool.add(request.part.texture, (unsigned int)ResourceType::Texture);

        auto lambda_cpu = [request]() {
            if (request.part.textureType == TextureType::Texture2D) {
                Engine::priv::TextureLoader::InitFromMemory(
                    *request.part.texture,
                    request.image, 
                    request.textureName,
                    request.part.isToBeMipmapped, 
                    request.part.internalFormat,
                    request.part.type
                );
            }
            Engine::priv::InternalTexturePublicInterface::LoadCPU(*request.part.texture);
        };
        if (request.part.async || std::this_thread::get_id() != Engine::Resources::getWindow().getOpenglThreadID()) {
            Engine::priv::threading::addJobWithPostCallback(lambda_cpu, [request]() {
                Engine::priv::InternalTexturePublicInterface::LoadGPU(*request.part.texture);
                request.m_Callback();
            });
        }else{
            lambda_cpu();
            Engine::priv::InternalTexturePublicInterface::LoadGPU(*request.part.texture);
            request.m_Callback();
        }
    }
}

#pragma endregion