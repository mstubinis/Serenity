
#include <serenity/resources/texture/TextureRequest.h>
#include <serenity/resources/texture/Texture.h>
#include <serenity/resources/texture/TextureLoader.h>
#include <serenity/system/window/Window.h>
#include <serenity/system/Engine.h>
#include <filesystem>

TextureRequest::TextureRequest(std::string_view filename, bool genMipMaps, ImageInternalFormat intFmt, TextureType textureType)
    : m_Part{ filename, intFmt, genMipMaps, textureType }
{
    if (!m_Part.m_CPUData.m_Name.empty()) {
        m_FileData.m_FileExtension = std::filesystem::path(m_Part.m_CPUData.m_Name).extension().string();
        if (std::filesystem::exists(m_Part.m_CPUData.m_Name)) {
            m_FileData.m_FileExists = true;
        }
    }
}
TextureRequest::TextureRequest(std::string_view filename, bool genMipMaps, ImageInternalFormat intFmt, TextureType textureType, Engine::ResourceCallback&& callback)
    : TextureRequest{ filename, genMipMaps, intFmt, textureType }
{
    m_Part.m_Callback = std::move(callback);
}
TextureRequest::TextureRequest(const sf::Image& sfImage, std::string_view filename, bool genMipMaps, ImageInternalFormat intFmt, TextureType textureType, Engine::ResourceCallback&& callback)
    : m_Part{ filename, intFmt, genMipMaps, textureType }
    , m_SFMLImage{ sfImage }
{
    m_FromMemory      = true;
    m_Part.m_Callback = std::move(callback);
}

void TextureRequest::request(bool inAsync) {
    if (m_Part.m_CPUData.m_Name.empty() || (!isFromFile() && !isFromMemory())) {
        ENGINE_PRODUCTION_LOG("TextureRequest::request(): request was invalid!")
        return;
    }
    m_Part.m_Async = inAsync && Engine::hardware_concurrency() > 1;
    auto info      = Engine::Resources::getResource<Texture>(m_Part.m_CPUData.m_Name);
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
    auto l_gpu = [textureRequestPart{ m_Part }]() {
        Engine::priv::TextureLoader::LoadGPU(textureRequestPart.m_Handle);
        textureRequestPart.m_Callback(textureRequestPart.m_Handle);
    };

    const bool isMainThread = Engine::priv::threading::isMainThread();
    if (m_Part.m_Async || !isMainThread) {
        Engine::priv::threading::addJobWithPostCallback(l_cpu, l_gpu);
    } else {
        l_cpu();
        l_gpu();
    }
}