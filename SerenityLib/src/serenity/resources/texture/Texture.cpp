
#include <serenity/renderer/Renderer.h>
#include <serenity/resources/texture/Texture.h>
#include <SFML/Graphics/Image.hpp>
#include <filesystem>

Handle Texture::White    = Handle{};
Handle Texture::Black    = Handle{};
Handle Texture::Checkers = Handle{};
Handle Texture::BRDF     = Handle{};

/*
void Engine::priv::TextureCPUData::initFromMemory(const sf::Image& sfImage) {
    auto& image = m_ImagesDatas[0];
    image.setInternalFormat(image.m_InternalFormat);
    image.load(sfImage, image.m_Filename);
}
*/
void Engine::priv::TextureCPUData::initFromMemory(const uint8_t* pixels, int inWidth, int inHeight) {
    auto& image = m_ImagesDatas[0];
    image.setInternalFormat(image.m_InternalFormat);
    image.load(pixels, inWidth, inHeight, image.m_Filename);
}
void Engine::priv::TextureCPUData::initFromFile() {
    auto& image           = m_ImagesDatas[0];
    std::string extension = std::filesystem::path(image.m_Filename).extension().string();
    if (extension == ".dds") {
        TextureLoader::LoadDDSFile(*this, image);
    }
    image.setInternalFormat(image.m_InternalFormat);
}

Texture::Texture(std::string_view textureName, TextureType textureType, bool mipMap)
    : Resource{ ResourceType::Texture, textureName }
{
    setName(textureName);
    m_CPUData.m_ImagesDatas[0].m_Filename = textureName;
    m_CPUData.m_Name                      = textureName;
    m_CPUData.m_TextureType               = textureType;
    m_CPUData.m_IsToBeMipmapped           = mipMap;
}
Texture::Texture(uint32_t w, uint32_t h, ImagePixelType pxlType, ImagePixelFormat pxlFmt, ImageInternalFormat intFmt, float divisor)
    : Texture{ "MRT", TextureType::RenderTarget, false }
{
    int width    = (int)((float)w * divisor);
    int height   = (int)((float)h * divisor);
    auto& image  = m_CPUData.m_ImagesDatas[0];
    image.load(width, height, pxlType, pxlFmt, intFmt);
    Engine::priv::TextureLoader::LoadGPU(*this); //nothing to load cpu side for frame buffers
}
/*
Texture::Texture(const sf::Image& sfImage, std::string_view name, bool genMipMaps, ImageInternalFormat intFmt, TextureType textureType)
    : Texture{ name, textureType, genMipMaps }
{
    m_CPUData.m_ImagesDatas[0].setInternalFormat(intFmt);

    const auto imgData = sfImage.getPixelsPtr();
    const auto imgSize = sfImage.getSize();

    m_CPUData.initFromMemory(imgData, imgSize.x, imgSize.y);
    Engine::priv::TextureLoader::Load(*this);
}
*/
Texture::Texture(uint8_t* pixels, uint32_t width, uint32_t height, std::string_view name, bool genMipMaps, ImageInternalFormat intFmt, TextureType textureType)
    : Texture{ name, textureType, genMipMaps }
{
    m_CPUData.m_ImagesDatas[0].setInternalFormat(intFmt);
    m_CPUData.initFromMemory(pixels, width, height);
    Engine::priv::TextureLoader::Load(*this);
}


Texture::Texture(std::string_view filename, bool genMipMaps, ImageInternalFormat intFmt, TextureType textureType)
    : Texture{ filename, textureType, genMipMaps }
{
    m_CPUData.m_ImagesDatas[0].setInternalFormat(intFmt);
    m_CPUData.initFromFile();
    Engine::priv::TextureLoader::Load(*this);
}

Texture::Texture(Texture&& other) noexcept 
    : Resource(std::move(other))
    , Engine::priv::TextureBaseClass(std::move(other))
    , m_CPUData                  { std::move(other.m_CPUData) }
{}
Texture& Texture::operator=(Texture&& other) noexcept {
    Resource::operator=(std::move(other));
    Engine::priv::TextureBaseClass::operator=(std::move(other));
    m_CPUData                  = std::move(other.m_CPUData);
    return *this;
}

Texture::~Texture(){
    Engine::priv::TextureLoader::Unload(*this);
}

bool Texture::generateMipmaps() {
    if (*this == false) {
        m_CommandQueue.emplace([this]() { generateMipmaps(); });
        return false;
    }
    internal_bind_if_not_bound(m_CPUData.m_TextureType, m_TextureAddress);
    return Engine::priv::TextureLoader::GenerateMipmapsOpenGL(*this);
}
void Texture::setXWrapping(TextureWrap wrap) {
    if (*this == false) {
        m_CommandQueue.emplace([this, wrap]() { setXWrapping(wrap); });
        return;
    }
    internal_bind_if_not_bound(m_CPUData.m_TextureType, m_TextureAddress);
    Engine::priv::TextureBaseClass::setXWrapping(m_CPUData.m_TextureType, wrap);
}
void Texture::setYWrapping(TextureWrap wrap) {
    if (*this == false) {
        m_CommandQueue.emplace([this, wrap]() { setYWrapping(wrap); });
        return;
    }
    internal_bind_if_not_bound(m_CPUData.m_TextureType, m_TextureAddress);
    Engine::priv::TextureBaseClass::setYWrapping(m_CPUData.m_TextureType, wrap);
}
void Texture::setWrapping(TextureWrap wrap) {
    if (*this == false) {
        m_CommandQueue.emplace([this, wrap]() { setWrapping(wrap); });
        return;
    }
    internal_bind_if_not_bound(m_CPUData.m_TextureType, m_TextureAddress);
    Engine::priv::TextureBaseClass::setWrapping(m_CPUData.m_TextureType, wrap);
}
void Texture::setMinFilter(TextureFilter filter) {
    if (*this == false) {
        m_CommandQueue.emplace([this, filter]() { setMinFilter(filter); });
        return;
    }
    internal_bind_if_not_bound(m_CPUData.m_TextureType, m_TextureAddress);
    Texture::setMinFilter(m_CPUData.m_TextureType, filter);
    m_CPUData.m_MinFilter = filter.toGLType(true);
}
void Texture::setMaxFilter(TextureFilter filter) {
    if (*this == false) {
        m_CommandQueue.emplace([this, filter]() { setMaxFilter(filter); });
        return;
    }
    internal_bind_if_not_bound(m_CPUData.m_TextureType, m_TextureAddress);
    Texture::setMaxFilter(m_CPUData.m_TextureType, filter);
}
void Texture::setFilter(TextureFilter filter) {
    if (*this == false) {
        m_CommandQueue.emplace([this, filter]() { setFilter(filter); });
        return;
    }
    internal_bind_if_not_bound(m_CPUData.m_TextureType, m_TextureAddress);
    Texture::setFilter(m_CPUData.m_TextureType, filter);
}
void Texture::setMinFilter(TextureType type, TextureFilter filter) {
    const auto glType = filter.toGLType(true);
    glTexParameteri(type.toGLType(), GL_TEXTURE_MIN_FILTER, glType);
}
void Texture::setMaxFilter(TextureType type, TextureFilter filter) {
    const auto glType = filter.toGLType(false);
    glTexParameteri(type.toGLType(), GL_TEXTURE_MAG_FILTER, glType);
}
void Texture::setAnisotropicFiltering(float anisotropicFiltering) {
    anisotropicFiltering = glm::clamp(anisotropicFiltering, 1.0f, Engine::priv::OpenGLState::constants.MAX_TEXTURE_MAX_ANISOTROPY);
    if (*this == false) {
        m_CommandQueue.emplace([this, anisotropicFiltering]() { Texture::setAnisotropicFiltering(anisotropicFiltering); });
        return;
    }
    internal_bind_if_not_bound(m_CPUData.m_TextureType, m_TextureAddress);
    internal_anisotropic_filtering(m_CPUData.m_TextureType, anisotropicFiltering);
}
bool Texture::compressed() const {
    ASSERT(m_CPUData.m_ImagesDatas.size() > 0, __FUNCTION__ << "(): m_CPUData.m_ImagesDatas.size() is 0!");
    ASSERT(m_CPUData.m_ImagesDatas[0].m_Mipmaps.size() > 0, __FUNCTION__ << "(): m_CPUData.m_ImagesDatas[0].m_Mipmaps.size() is 0!");
    return m_CPUData.m_ImagesDatas[0].m_Mipmaps[0].compressedSize > 0;
}
uint8_t* Texture::pixels() {
    Engine::priv::TextureLoader::WithdrawPixelsFromOpenGLMemory(*this, 0, 0);
    ASSERT(m_CPUData.m_ImagesDatas.size() > 0, __FUNCTION__ << "(): m_CPUData.m_ImagesDatas.size() is 0!");
    ASSERT(m_CPUData.m_ImagesDatas[0].m_Mipmaps.size() > 0, __FUNCTION__ << "(): m_CPUData.m_ImagesDatas[0].m_Mipmaps.size() is 0!");
    return &(m_CPUData.m_ImagesDatas[0].m_Mipmaps[0].pixels)[0];
}
int Texture::getMaxMipmapLevelsPossible() const noexcept {
    const auto sz = size();
    return glm::max(0, (int)glm::floor(glm::log2(glm::max(sz.x, sz.y))));
}
