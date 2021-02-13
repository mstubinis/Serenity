
#include <serenity/resources/texture/TextureRequest.h>
#include <serenity/resources/texture/Texture.h>
#include <serenity/resources/texture/TextureLoader.h>
#include <serenity/system/window/Window.h>
#include <serenity/system/Engine.h>
#include <filesystem>

using namespace Engine;
using namespace Engine::priv;

#pragma region TextureRequest

TextureRequest::TextureRequest(const std::string& filename, bool genMipMaps, ImageInternalFormat intFmt, TextureType textureType) 
    : m_Part{ filename, intFmt, genMipMaps, textureType }
{
    if (!m_Part.m_CPUData.m_Name.empty()) {
        m_FileData.m_FileExtension = std::filesystem::path(m_Part.m_CPUData.m_Name).extension().string();
        if (std::filesystem::exists(m_Part.m_CPUData.m_Name)) {
            m_FileData.m_FileExists = true;
        }
    }
}
TextureRequest::TextureRequest(const std::string& filename, bool genMipMaps, ImageInternalFormat intFmt, TextureType textureType, Engine::ResourceCallback&& callback)
    : TextureRequest{ filename, genMipMaps, intFmt, textureType }
{
    m_Part.m_Callback = std::move(callback);
}

TextureRequest::TextureRequest(const sf::Image& sfImage, const std::string& filename, bool genMipMaps, ImageInternalFormat intFmt, TextureType textureType, Engine::ResourceCallback&& callback)
    : m_SFMLImage{ sfImage }
    , m_Part{ filename, intFmt, genMipMaps, textureType }
{
    m_FromMemory      = true;
    m_Part.m_Callback = std::move(callback);
}

void TextureRequest::request(bool inAsync) {
    if (m_Part.m_CPUData.m_Name.empty() || (!isFromFile() && !isFromMemory())) {
        ENGINE_PRODUCTION_LOG("TextureRequest::request(): request was invalid!")
        return;
    }
    m_Part.m_Async   = (inAsync && Engine::hardware_concurrency() > 1);

    auto info = Engine::Resources::getResource<Texture>(m_Part.m_CPUData.m_Name);
    if (info.m_Resource) {
        //Texture was already loaded
        m_Part.m_Handle = info.m_Handle;
        return;
    }
    m_Part.m_Handle  = Engine::Resources::addResource<Texture>(m_Part.m_CPUData.m_Name, m_Part.m_CPUData.m_TextureType, m_Part.m_CPUData.m_IsToBeMipmapped);

    auto l_cpu = [textureRequest{ *this }]() mutable {
        //6 file cubemaps and framebuffers are not loaded this way
        if (textureRequest.m_Part.m_CPUData.m_TextureType == TextureType::Texture2D) {
            if (textureRequest.m_FromMemory)
                textureRequest.m_Part.m_CPUData.initFromMemory(textureRequest.m_SFMLImage);
            else
                textureRequest.m_Part.m_CPUData.initFromFile();
        }
        Engine::priv::TextureLoader::LoadCPU(textureRequest.m_Part.m_CPUData, textureRequest.m_Part.m_Handle);
    };
    auto l_gpu = [textureRequestPart{ m_Part }]() mutable {
        Engine::priv::TextureLoader::LoadGPU(textureRequestPart.m_Handle);
        textureRequestPart.m_Callback(textureRequestPart.m_Handle);
    };
    if (m_Part.m_Async || !threading::isMainThread()) {
        if (threading::isMainThread()) {
            threading::addJobWithPostCallback(std::move(l_cpu), std::move(l_gpu), 1U);
        }else{
            threading::submitTaskForMainThread([c{ std::move(l_cpu) }, g{ std::move(l_gpu) }]() mutable { 
                threading::addJobWithPostCallback(std::move(c), std::move(g), 1U); 
            });
        }
    }else{
        l_cpu();
        l_gpu();
    }
}


#pragma endregion
