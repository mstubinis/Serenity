#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/textures/TextureRequest.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/textures/TextureLoader.h>
#include <core/engine/system/window/Window.h>
#include <core/engine/system/Engine.h>

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
TextureRequest::TextureRequest(const std::string& filename, bool genMipMaps, ImageInternalFormat intFmt, TextureType textureType, std::function<void()>&& callback)
    : TextureRequest{ filename, genMipMaps, intFmt, textureType }
{
    m_Part.m_Callback = std::move(callback);
}

TextureRequest::TextureRequest(const sf::Image& sfImage, const std::string& filename, bool genMipMaps, ImageInternalFormat intFmt, TextureType textureType, std::function<void()>&& callback)
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
    m_Part.async   = (inAsync && Engine::hardware_concurrency() > 1);
    m_Part.handle  = Engine::Resources::addResource<Texture>(m_Part.m_CPUData.m_Name, m_Part.m_CPUData.m_TextureType, m_Part.m_CPUData.m_IsToBeMipmapped);
    TextureRequest textureRequest(*this);
    auto lambda_cpu = [textureRequest]() mutable {
        //6 file cubemaps and framebuffers are not loaded this way
        if (textureRequest.m_Part.m_CPUData.m_TextureType == TextureType::Texture2D) {
            if (textureRequest.m_FromMemory)
                textureRequest.m_Part.m_CPUData.initFromMemory(textureRequest.m_SFMLImage);
            else
                textureRequest.m_Part.m_CPUData.initFromFile();
        }
        Engine::priv::TextureLoader::LoadCPU(textureRequest.m_Part.m_CPUData, textureRequest.m_Part.handle);
    };
    auto lambda_gpu = [textureRequest]() mutable {
        Engine::priv::TextureLoader::LoadGPU(textureRequest.m_Part.handle);
        textureRequest.m_Part.m_Callback();
    };

    if (m_Part.async || !Engine::priv::threading::isMainThread()) {
        Engine::priv::threading::addJobWithPostCallback(lambda_cpu, lambda_gpu);
    }else{
        lambda_cpu();
        lambda_gpu();
    }
}


#pragma endregion
