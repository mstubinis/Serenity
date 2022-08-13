
#include <serenity/renderer/Renderer.h>
#include <serenity/resources/texture/TextureCubemap.h>
#include <SFML/Graphics/Image.hpp>

#include <serenity/renderer/opengl/APIStateOpenGL.h>
#include <serenity/resources/texture/DDS.h>

#include <filesystem>

class TextureCubemap::Impl {
public:
    static void Load(TextureCubemap& cubemap) {
        if (!cubemap.isLoaded()) {
            Engine::priv::TextureLoaderCubemap::LoadCPU(cubemap.m_CPUData, Handle{});
            Engine::priv::TextureLoaderCubemap::LoadGPU(cubemap);
        }
    }
    static void Unload(TextureCubemap& cubemap) {
        if (cubemap.isLoaded()) {
            Engine::priv::TextureLoaderCubemap::UnloadGPU(cubemap);
        }
    }
    static bool AddToCommandQueue(TextureCubemap& cubemap, auto func, auto&&... args) {
#ifdef TEXTURE_COMMAND_QUEUE
        if (cubemap == false) {
            cubemap.m_CommandQueue.emplace([&func, &cubemap, ... args = std::forward<decltype(args)>(args)]() {
                ((std::addressof(cubemap))->*func)(args...);
            });
            return true;
        }
        return false;
#else
        assert(cubemap == true);
        return false;
#endif
    }
};

namespace Engine::priv {
    void TextureCubemapCPUData::initFromFile() {
        auto& image = m_ImagesDatas[0];
        const std::string extension = std::filesystem::path(image.m_Filename).extension().string();
        if (extension == ".dds") {
            Engine::priv::LoadDDSFile(*this, image);
        }
        image.setInternalFormat(image.m_InternalFormat);
    }
    void TextureCubemapCPUData::initFromCubemap(const std::array<std::string_view, 6>& files, ImageInternalFormat intFmt) {
        for (int i = 0; i < files.size(); ++i) {
            auto& image = i >= m_ImagesDatas.size() ? m_ImagesDatas.emplace_back() : m_ImagesDatas[i];
            image.m_Filename = files[i];
        }
        for (auto& sideImage : m_ImagesDatas) {
            sideImage.setInternalFormat(intFmt);
        }
    }
}

TextureCubemap::TextureCubemap(std::string_view textureName, bool mipMap)
    : Resource{ ResourceType::TextureCubemap, textureName }
{
    setName(std::string{ textureName });
    m_CPUData.m_ImagesDatas[0].m_Filename = textureName;
    m_CPUData.m_Name                      = textureName;
    m_CPUData.m_IsToBeMipmapped           = mipMap;
}
TextureCubemap::TextureCubemap(std::string_view filename, bool genMipMaps, ImageInternalFormat internalFormat)
    : TextureCubemap{ filename, genMipMaps }
{
    m_CPUData.m_ImagesDatas[0].setInternalFormat(internalFormat);
    m_CPUData.initFromFile();

    Impl::Load(*this);
}
TextureCubemap::TextureCubemap(const std::array<std::string_view, 6>& files, std::string_view name, bool genMipMaps, ImageInternalFormat internalFormat)
    : TextureCubemap{ name, genMipMaps }
{
    m_CPUData.initFromCubemap(files, internalFormat);
    Impl::Load(*this);
}
TextureCubemap::TextureCubemap(TextureCubemap&& other) noexcept
    : Resource(std::move(other))
    , Engine::priv::TextureBaseClass(std::move(other))
    , m_CPUData                 { std::move(other.m_CPUData) }
    , m_ConvolutionTextureHandle{ std::move(other.m_ConvolutionTextureHandle) }
    , m_PreEnvTextureHandle     { std::move(other.m_PreEnvTextureHandle) }
{}
TextureCubemap& TextureCubemap::operator=(TextureCubemap&& other) noexcept {
    if (this != &other) {
        Resource::operator=(std::move(other));
        Engine::priv::TextureBaseClass::operator=(std::move(other));
        m_CPUData                  = std::move(other.m_CPUData);
        m_ConvolutionTextureHandle = std::move(other.m_ConvolutionTextureHandle);
        m_PreEnvTextureHandle      = std::move(other.m_PreEnvTextureHandle);
    }
    return *this;
}

TextureCubemap::~TextureCubemap() {
    Impl::Unload(*this);
}
bool TextureCubemap::generateMipmaps() {
    if (Impl::AddToCommandQueue(*this, &TextureCubemap::generateMipmaps)) {
        return false;
    }
    return Engine::opengl::generateMipmaps(*this);
}
void TextureCubemap::setXWrapping(TextureWrap wrap) {
    if (Impl::AddToCommandQueue(*this, &TextureCubemap::setXWrapping, wrap)) {
        return;
    }
    Engine::opengl::setXWrapping(*this, wrap);
}
void TextureCubemap::setYWrapping(TextureWrap wrap) {
    if (Impl::AddToCommandQueue(*this, &TextureCubemap::setYWrapping, wrap)) {
        return;
    }
    Engine::opengl::setYWrapping(*this, wrap);
}
void TextureCubemap::setZWrapping(TextureWrap wrap) {
    if (Impl::AddToCommandQueue(*this, &TextureCubemap::setZWrapping, wrap)) {
        return;
    }
    Engine::opengl::setZWrapping(*this, wrap);
}
void TextureCubemap::setWrapping(TextureWrap wrap) {
    if (Impl::AddToCommandQueue(*this, &TextureCubemap::setWrapping, wrap)) {
        return;
    }
    Engine::opengl::setWrapping(*this, wrap);
}
void TextureCubemap::setMinFilter(TextureFilter filter) {
    if (Impl::AddToCommandQueue(*this, &TextureCubemap::setMinFilter, filter)) {
        return;
    }
    Engine::opengl::setMinFilter(*this, filter);
}
void TextureCubemap::setMaxFilter(TextureFilter filter) {
    if (Impl::AddToCommandQueue(*this, &TextureCubemap::setMaxFilter, filter)) {
        return;
    }
    Engine::opengl::setMaxFilter(*this, filter);
}
void TextureCubemap::setFilter(TextureFilter filter) {
    if (Impl::AddToCommandQueue(*this, &TextureCubemap::setFilter, filter)) {
        return;
    }
    Engine::opengl::setFilter(*this, filter);
}
void TextureCubemap::setAnisotropicFiltering(float anisotropicFiltering) {
    if (Impl::AddToCommandQueue(*this, &TextureCubemap::setAnisotropicFiltering, anisotropicFiltering)) {
        return;
    }
    Engine::opengl::setAnisotropicFiltering(*this, anisotropicFiltering);
}
const uint8_t* TextureCubemap::pixels() {
    if (!m_CPUData.m_ImagesDatas.empty() && !m_CPUData.m_ImagesDatas[0].m_Mipmaps.empty()) {
        return m_CPUData.m_ImagesDatas[0].m_Mipmaps[0].pixels.data();
    }
    return Engine::opengl::getPixels(*this);
}