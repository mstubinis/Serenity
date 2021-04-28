
#include <serenity/renderer/Renderer.h>
#include <serenity/resources/texture/TextureCubemap.h>
#include <SFML/Graphics/Image.hpp>
#include <filesystem>

void Engine::priv::TextureCubemapCPUData::initFromFile() {
    auto& image = m_ImagesDatas[0];
    std::string extension = std::filesystem::path(image.m_Filename).extension().string();
    if (extension == ".dds") {
        TextureLoaderCubemap::LoadDDSFile(*this, image);
    }
    image.setInternalFormat(image.m_InternalFormat);
}
void Engine::priv::TextureCubemapCPUData::initFromCubemap(const std::array<std::string_view, 6>& files, ImageInternalFormat intFmt) {
    for (int i = 0; i < files.size(); ++i) {
        auto& image = i >= m_ImagesDatas.size() ? m_ImagesDatas.emplace_back() : m_ImagesDatas[i];
        image.m_Filename = files[i];
    }
    for (auto& sideImage : m_ImagesDatas) {
        sideImage.setInternalFormat(intFmt);
    }
}

TextureCubemap::TextureCubemap(std::string_view textureName, bool mipMap)
    : Resource{ ResourceType::TextureCubemap, textureName }
{
    setName(textureName);
    m_CPUData.m_ImagesDatas[0].m_Filename = textureName;
    m_CPUData.m_Name                      = textureName;
    m_CPUData.m_IsToBeMipmapped           = mipMap;
}
TextureCubemap::TextureCubemap(std::string_view filename, bool genMipMaps, ImageInternalFormat intFmt)
    : TextureCubemap{ filename, genMipMaps }
{
    m_CPUData.m_ImagesDatas[0].setInternalFormat(intFmt);
    m_CPUData.initFromFile();

    Engine::priv::TextureLoaderCubemap::Load(*this);
}
TextureCubemap::TextureCubemap(const std::array<std::string_view, 6>& files, std::string_view name, bool genMipMaps, ImageInternalFormat intFmt)
    : TextureCubemap{ name, genMipMaps }
{
    m_CPUData.initFromCubemap(files, intFmt);
    Engine::priv::TextureLoaderCubemap::Load(*this);
}
TextureCubemap::TextureCubemap(TextureCubemap&& other) noexcept
    : Resource(std::move(other))
    , Engine::priv::TextureBaseClass(std::move(other))
    , m_CPUData                 { std::move(other.m_CPUData) }
    , m_ConvolutionTextureHandle{ std::exchange(other.m_ConvolutionTextureHandle, Handle{}) }
    , m_PreEnvTextureHandle     { std::exchange(other.m_PreEnvTextureHandle, Handle{}) }
{}
TextureCubemap& TextureCubemap::operator=(TextureCubemap&& other) noexcept {
    Resource::operator=(std::move(other));
    Engine::priv::TextureBaseClass::operator=(std::move(other));
    m_CPUData                  = std::move(other.m_CPUData);
    m_ConvolutionTextureHandle = std::exchange(other.m_ConvolutionTextureHandle, Handle{});
    m_PreEnvTextureHandle      = std::exchange(other.m_PreEnvTextureHandle, Handle{});
    return *this;
}

TextureCubemap::~TextureCubemap() {
    Engine::priv::TextureLoaderCubemap::Unload(*this);
}
bool TextureCubemap::generateMipmaps() {
    if (*this == false) {
        m_CommandQueue.emplace([this]() { generateMipmaps(); });
        return false;
    }
    internal_bind_if_not_bound(TextureType::CubeMap, m_TextureAddress);
    return Engine::priv::TextureLoaderCubemap::GenerateMipmapsOpenGL(*this);
}
void TextureCubemap::setXWrapping(TextureWrap wrap) {
    if (*this == false) {
        m_CommandQueue.emplace([this, wrap]() { setXWrapping(wrap); });
        return;
    }
    internal_bind_if_not_bound(TextureType::CubeMap, m_TextureAddress);
    Engine::priv::TextureBaseClass::setXWrapping(TextureType::CubeMap, wrap);
}
void TextureCubemap::setYWrapping(TextureWrap wrap) {
    if (*this == false) {
        m_CommandQueue.emplace([this, wrap]() { setYWrapping(wrap); });
        return;
    }
    internal_bind_if_not_bound(TextureType::CubeMap, m_TextureAddress);
    Engine::priv::TextureBaseClass::setYWrapping(TextureType::CubeMap, wrap);
}
void TextureCubemap::setZWrapping(TextureWrap wrap) {
    if (*this == false) {
        m_CommandQueue.emplace([this, wrap]() { setZWrapping(wrap); });
        return;
    }
    internal_bind_if_not_bound(TextureType::CubeMap, m_TextureAddress);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, wrap.toGLType());
}
void TextureCubemap::setWrapping(TextureWrap wrap) {
    if (*this == false) {
        m_CommandQueue.emplace([this, wrap]() { setWrapping(wrap); });
        return;
    }
    internal_bind_if_not_bound(TextureType::CubeMap, m_TextureAddress);
    setXWrapping(wrap);
    setYWrapping(wrap);
    setZWrapping(wrap);
}
void TextureCubemap::setMinFilter(TextureFilter filter) {
    if (*this == false) {
        m_CommandQueue.emplace([this, filter]() { setMinFilter(filter); });
        return;
    }
    internal_bind_if_not_bound(TextureType::CubeMap, m_TextureAddress);

    const auto glType = filter.toGLType(true);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, glType);

    m_CPUData.m_MinFilter = filter.toGLType(true);
}
void TextureCubemap::setMaxFilter(TextureFilter filter) {
    if (*this == false) {
        m_CommandQueue.emplace([this, filter]() { setMaxFilter(filter); });
        return;
    }
    internal_bind_if_not_bound(TextureType::CubeMap, m_TextureAddress);

    const auto glType = filter.toGLType(false);
    GLCall(glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, glType));
}
void TextureCubemap::setFilter(TextureFilter filter) {
    if (*this == false) {
        m_CommandQueue.emplace([this, filter]() { setFilter(filter); });
        return;
    }
    internal_bind_if_not_bound(TextureType::CubeMap, m_TextureAddress);
    TextureCubemap::setMinFilter(filter);
    TextureCubemap::setMaxFilter(filter);
}
void TextureCubemap::setAnisotropicFiltering(float anisotropicFiltering) {
    anisotropicFiltering = glm::clamp(anisotropicFiltering, 1.0f, Engine::priv::OpenGLState::constants.MAX_TEXTURE_MAX_ANISOTROPY);
    if (*this == false) {
        m_CommandQueue.emplace([this, anisotropicFiltering]() { TextureCubemap::setAnisotropicFiltering(anisotropicFiltering); });
        return;
    }
    internal_bind_if_not_bound(TextureType::CubeMap, m_TextureAddress);
    internal_anisotropic_filtering(TextureType::CubeMap, anisotropicFiltering);
}
bool TextureCubemap::compressed() const {
    ASSERT(m_CPUData.m_ImagesDatas.size() > 0, __FUNCTION__ << "(): m_CPUData.m_ImagesDatas.size() is 0!");
    ASSERT(m_CPUData.m_ImagesDatas[0].m_Mipmaps.size() > 0, __FUNCTION__ << "(): m_CPUData.m_ImagesDatas[0].m_Mipmaps.size() is 0!");
    return m_CPUData.m_ImagesDatas[0].m_Mipmaps[0].compressedSize > 0;
}
uint8_t* TextureCubemap::pixels() {
    Engine::priv::TextureLoaderCubemap::WithdrawPixelsFromOpenGLMemory(*this, 0, 0);
    ASSERT(m_CPUData.m_ImagesDatas.size() > 0, __FUNCTION__ << "(): m_CPUData.m_ImagesDatas.size() is 0!");
    ASSERT(m_CPUData.m_ImagesDatas[0].m_Mipmaps.size() > 0, __FUNCTION__ << "(): m_CPUData.m_ImagesDatas[0].m_Mipmaps.size() is 0!");
    return &(m_CPUData.m_ImagesDatas[0].m_Mipmaps[0].pixels)[0];
}
int TextureCubemap::getMaxMipmapLevelsPossible() const noexcept {
    const auto sz = size();
    return glm::max(0, (int)glm::floor(glm::log2(glm::max(sz.x, sz.y))));
}