#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/textures/TextureRequest.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/textures/TextureLoader.h>
#include <core/engine/system/window/Window.h>
#include <core/engine/system/Engine.h>

#include <filesystem>

#pragma region TextureRequest

TextureRequest::TextureRequest(const std::string& filename, bool genMipMaps, ImageInternalFormat intFmt, TextureType textureType) {
    m_Part.fileOrTextureName = filename;
    m_Part.internalFormat    = intFmt;
    m_Part.isToBeMipmapped   = genMipMaps;
    m_Part.textureType       = textureType;
    if (!m_Part.fileOrTextureName.empty()) {
        m_FileData.m_FileExtension = std::filesystem::path(m_Part.fileOrTextureName).extension().string();
        if (std::filesystem::exists(m_Part.fileOrTextureName)) {
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
{
    m_FromMemory = true;
    m_Part.m_Callback = std::move(callback);
    m_Part.fileOrTextureName = filename;
    m_Part.async = false;
    m_Part.internalFormat = intFmt;
    m_Part.isToBeMipmapped = genMipMaps;
    m_Part.textureType = textureType;
}
TextureRequest::TextureRequest(const TextureRequest& other) 
    : m_Part       { other.m_Part }
    , m_FromMemory { other.m_FromMemory }
    , m_SFMLImage  { other.m_SFMLImage }
    , m_FileData   { other.m_FileData }
{}
TextureRequest& TextureRequest::operator=(const TextureRequest& other) {
    m_Part         = other.m_Part;
    m_FromMemory   = other.m_FromMemory;   
    m_SFMLImage    = other.m_SFMLImage;
    m_FileData     = other.m_FileData;
    return *this;
}
TextureRequest::TextureRequest(TextureRequest&& other) noexcept
    : m_Part       { std::move(other.m_Part) }
    , m_FromMemory { std::move(other.m_FromMemory) }
    , m_SFMLImage  { std::move(other.m_SFMLImage) } //copys instead of moves
    , m_FileData   { std::move(other.m_FileData) }
{}
TextureRequest& TextureRequest::operator=(TextureRequest&& other) noexcept {
    m_Part         = std::move(other.m_Part);
    m_FromMemory   = std::move(other.m_FromMemory);
    m_SFMLImage    = std::move(other.m_SFMLImage);//copys instead of moves
    m_FileData     = std::move(other.m_FileData);
    return *this;
}



void TextureRequest::request(bool inAsync) {
    m_Part.async = (inAsync && Engine::hardware_concurrency() > 1);
    Engine::priv::TextureRequestStaticImpl::Request(*this);
}
void Engine::priv::TextureRequestStaticImpl::Request(TextureRequest& request) {
    if (!request.m_Part.fileOrTextureName.empty()) {
        if (request.isFromFile() || request.isFromMemory()) {
            request.m_Part.handle              = Core::m_Engine->m_ResourceManager.m_ResourceModule.emplace<Texture>();
            Texture& textureRef                = *request.m_Part.handle.get<Texture>();
            textureRef.m_CPUData.m_TextureType = request.m_Part.textureType;
            textureRef.setName(request.m_Part.fileOrTextureName);

            auto lambda_cpu = [request]() {
                if (request.m_Part.textureType == TextureType::Texture2D) {
                    if(request.m_FromMemory)
                        Engine::priv::TextureLoader::CPUInitFromMemory(request.m_Part.handle, request.m_SFMLImage, request.m_Part.fileOrTextureName, request.m_Part.isToBeMipmapped, request.m_Part.internalFormat, request.m_Part.textureType);
                    else
                        Engine::priv::TextureLoader::CPUInitFromFile(request.m_Part.handle, request.m_Part.fileOrTextureName, request.m_Part.isToBeMipmapped, request.m_Part.internalFormat, request.m_Part.textureType);
                }
                Engine::priv::InternalTexturePublicInterface::LoadCPU(request.m_Part.handle);
            };
            auto lambda_gpu = [request]() {
                Engine::priv::InternalTexturePublicInterface::LoadGPU(request.m_Part.handle);
                request.m_Part.m_Callback();
            };

            if (request.m_Part.async || std::this_thread::get_id() != Engine::Resources::getWindow().getOpenglThreadID()) {
                Engine::priv::threading::addJobWithPostCallback(lambda_cpu, lambda_gpu);
            }else{
                lambda_cpu();
                lambda_gpu();
            }
        }
    }
}

#pragma endregion
