#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/renderer/Renderer.h>
#include <core/engine/textures/Texture.h>
#include <SFML/Graphics/Image.hpp>

Handle Texture::White    = Handle{};
Handle Texture::Black    = Handle{};
Handle Texture::Checkers = Handle{};
Handle Texture::BRDF     = Handle{};

void Engine::priv::TextureCPUData::initFromMemory(const sf::Image& sfImage) {
    auto& image = m_ImagesDatas[0];
    image.setInternalFormat(image.m_InternalFormat);
    image.load(sfImage, image.m_Filename);
}
void Engine::priv::TextureCPUData::initFromFile() {
    auto& image           = m_ImagesDatas[0];
    std::string extension = std::filesystem::path(image.m_Filename).extension().string();
    if (extension == ".dds") {
        TextureLoader::LoadDDSFile(*this, image);
    }
    image.setInternalFormat(image.m_InternalFormat);
}
void Engine::priv::TextureCPUData::initFromCubemap(const std::array<std::string, 6>& files, ImageInternalFormat intFmt) {
    auto& image = m_ImagesDatas[0];
    image.m_Filename = files[0];
    for (int j = 1; j < files.size(); ++j) {
        auto& imageInnerLoop = m_ImagesDatas.emplace_back();
        imageInnerLoop.m_Filename = files[j];
    }
    for (auto& sideImage : m_ImagesDatas) {
        sideImage.setInternalFormat(intFmt);
    }
}

Texture::Texture(const std::string& textureName, TextureType textureType, bool mipMap) 
    : Resource{ ResourceType::Texture, textureName }
{
    setName(textureName);
    m_CPUData.m_ImagesDatas[0].m_Filename = textureName;
    m_CPUData.m_Name                      = textureName;
    m_CPUData.m_TextureType               = textureType;
    m_CPUData.m_IsToBeMipmapped           = mipMap;
}
Texture::Texture(unsigned int w, unsigned int h, ImagePixelType pxlType, ImagePixelFormat pxlFmt, ImageInternalFormat intFmt, float divisor) 
    : Texture{ "MRT", TextureType::RenderTarget, false }
{
    int width    = (int)((float)w * divisor);
    int height   = (int)((float)h * divisor);
    auto& image  = m_CPUData.m_ImagesDatas[0];
    image.load(width, height, pxlType, pxlFmt, intFmt);

    Engine::priv::TextureLoader::LoadGPU(*this); //nothing to load cpu side for frame buffers
}
Texture::Texture(const sf::Image& sfImage, const std::string& name, bool genMipMaps, ImageInternalFormat intFmt, TextureType textureType)
    : Texture{ name, textureType, genMipMaps }
{
    m_CPUData.m_ImagesDatas[0].setInternalFormat(intFmt);
    m_CPUData.initFromMemory(sfImage);

    Engine::priv::TextureLoader::Load(*this);
}
Texture::Texture(const std::string& filename, bool genMipMaps, ImageInternalFormat intFmt, TextureType textureType)
    : Texture{ filename, textureType, genMipMaps }
{
    m_CPUData.m_ImagesDatas[0].setInternalFormat(intFmt);
    m_CPUData.initFromFile();

    Engine::priv::TextureLoader::Load(*this);
}
Texture::Texture(const std::array<std::string, 6>& files, const std::string& name, bool genMipMaps, ImageInternalFormat intFmt)
    : Texture{ name, TextureType::CubeMap, genMipMaps }
{
    m_CPUData.initFromCubemap(files, intFmt);

    Engine::priv::TextureLoader::Load(*this);
}


Texture::Texture(Texture&& other) noexcept 
    : Resource(std::move(other))
    , m_CPUData                  { std::move(other.m_CPUData) }
    , m_CommandQueue             { std::move(other.m_CommandQueue) }
    , m_TextureAddress           { std::exchange(other.m_TextureAddress, 0) }
    , m_ConvolutionTextureHandle { std::exchange(other.m_ConvolutionTextureHandle, Handle{}) }
    , m_PreEnvTextureHandle      { std::exchange(other.m_PreEnvTextureHandle, Handle{}) }
{}
Texture& Texture::operator=(Texture&& other) noexcept {
    Resource::operator=(std::move(other));
    m_CPUData                  = std::move(other.m_CPUData);
    m_CommandQueue             = std::move(other.m_CommandQueue);
    m_TextureAddress           = std::exchange(other.m_TextureAddress, 0);
    m_ConvolutionTextureHandle = std::exchange(other.m_ConvolutionTextureHandle, Handle{});
    m_PreEnvTextureHandle      = std::exchange(other.m_PreEnvTextureHandle, Handle{});
    return *this;
}

Texture::~Texture(){
    Engine::priv::TextureLoader::Unload(*this);
}
bool Texture::internal_bind_if_not_bound(unsigned int requestedAddress) noexcept {
    auto whichID = Engine::Renderer::getCurrentlyBoundTextureOfType(requestedAddress);
    if (whichID != requestedAddress) {
        Engine::Renderer::bindTextureForModification(m_CPUData.m_TextureType, requestedAddress);
        return true;
    }
    return false;
}
void Texture::setXWrapping(TextureWrap wrap) {
    if (*this == false) {
        m_CommandQueue.emplace([this, wrap]() { setXWrapping(wrap); });
        return;
    }
    internal_bind_if_not_bound(m_TextureAddress);
    Texture::setXWrapping(m_CPUData.m_TextureType, wrap);
}
void Texture::setYWrapping(TextureWrap wrap) {
    if (*this == false) {
        m_CommandQueue.emplace([this, wrap]() { setYWrapping(wrap); });
        return;
    }
    internal_bind_if_not_bound(m_TextureAddress);
    Texture::setYWrapping(m_CPUData.m_TextureType, wrap);
}
void Texture::setZWrapping(TextureWrap wrap) {
    if (*this == false) {
        m_CommandQueue.emplace([this, wrap]() { setZWrapping(wrap); });
        return;
    }
    internal_bind_if_not_bound(m_TextureAddress);
    Texture::setZWrapping(m_CPUData.m_TextureType, wrap);
}
void Texture::setWrapping(TextureWrap wrap) {
    if (*this == false) {
        m_CommandQueue.emplace([this, wrap]() { setWrapping(wrap); });
        return;
    }
    internal_bind_if_not_bound(m_TextureAddress);
    Texture::setWrapping(m_CPUData.m_TextureType, wrap);
}
void Texture::setMinFilter(TextureFilter filter) {
    if (*this == false) {
        m_CommandQueue.emplace([this, filter]() { setMinFilter(filter); });
        return;
    }
    internal_bind_if_not_bound(m_TextureAddress);
    Texture::setMinFilter(m_CPUData.m_TextureType, filter);
    m_CPUData.m_MinFilter = filter.toGLType(true);
}
void Texture::setMaxFilter(TextureFilter filter) {
    if (*this == false) {
        m_CommandQueue.emplace([this, filter]() { setMaxFilter(filter); });
        return;
    }
    internal_bind_if_not_bound(m_TextureAddress);
    Texture::setMaxFilter(m_CPUData.m_TextureType, filter);
}
void Texture::setFilter(TextureFilter filter) {
    if (*this == false) {
        m_CommandQueue.emplace([this, filter]() { setFilter(filter); });
        return;
    }
    internal_bind_if_not_bound(m_TextureAddress);
    Texture::setFilter(m_CPUData.m_TextureType, filter);
}
void Texture::setXWrapping(TextureType type, TextureWrap wrap) {
    GLCall(glTexParameteri(type.toGLType(), GL_TEXTURE_WRAP_S, wrap.toGLType()));
}
void Texture::setYWrapping(TextureType type, TextureWrap wrap) {
    GLCall(glTexParameteri(type.toGLType(), GL_TEXTURE_WRAP_T, wrap.toGLType()));
}
void Texture::setZWrapping(TextureType type, TextureWrap wrap) {
    if (type.toGLType() != GL_TEXTURE_CUBE_MAP) {
        ENGINE_PRODUCTION_LOG("Texture::setZWrapping() error: type was not GL_TEXTURE_CUBE_MAP!")
        return;
    }
    GLCall(glTexParameteri(type.toGLType(), GL_TEXTURE_WRAP_R, wrap.toGLType()));
}
void Texture::setWrapping(TextureType type, TextureWrap wrap) {
    Texture::setXWrapping(type, wrap);
    Texture::setYWrapping(type, wrap);
    if (type == TextureType::CubeMap) {
        Texture::setZWrapping(type, wrap);
    }
}
void Texture::setMinFilter(TextureType type, TextureFilter filter) {
    GLCall(glTexParameteri(type.toGLType(), GL_TEXTURE_MIN_FILTER, filter.toGLType(true)));
}
void Texture::setMaxFilter(TextureType type, TextureFilter filter) {
    GLCall(glTexParameteri(type.toGLType(), GL_TEXTURE_MAG_FILTER, filter.toGLType(false)));
}
void Texture::setFilter(TextureType type, TextureFilter filter) {
    Texture::setMinFilter(type, filter);
    Texture::setMaxFilter(type, filter);
}
void Texture::setAnisotropicFiltering(float anisotropicFiltering){
    anisotropicFiltering = glm::clamp(anisotropicFiltering, 1.0f, Engine::priv::OpenGLState::MAX_TEXTURE_MAX_ANISOTROPY);

    if (*this == false) {
        m_CommandQueue.emplace([this, anisotropicFiltering]() {
            setAnisotropicFiltering(anisotropicFiltering);
        });
        return;
    }
    internal_bind_if_not_bound(m_TextureAddress);
    if (Engine::priv::RenderModule::OPENGL_VERSION >= 46) {
        GLCall(glTexParameterf(m_CPUData.m_TextureType.toGLType(), GL_TEXTURE_MAX_ANISOTROPY, anisotropicFiltering));
    }else{
        if (Engine::priv::OpenGLExtensions::supported(Engine::priv::OpenGLExtensions::ARB_texture_filter_anisotropic)) {
            GLCall(glTexParameterf(m_CPUData.m_TextureType.toGLType(), GL_TEXTURE_MAX_ANISOTROPY, anisotropicFiltering));
        }else if (Engine::priv::OpenGLExtensions::supported(Engine::priv::OpenGLExtensions::EXT_texture_filter_anisotropic)) {
            GLCall(glTexParameterf(m_CPUData.m_TextureType.toGLType(), GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropicFiltering));
        }
    }
}
glm::vec2 Texture::sizeAsRatio() const {
    float max_val = glm::max((float)size().x, (float)size().y);
    return glm::vec2(glm::vec2(size()) / max_val);
}
bool Texture::compressed() const {
    ASSERT(m_CPUData.m_ImagesDatas.size() > 0, __FUNCTION__ << "(): m_CPUData.m_ImagesDatas.size() is 0!");
    ASSERT(m_CPUData.m_ImagesDatas[0].m_Mipmaps.size() > 0, __FUNCTION__ << "(): m_CPUData.m_ImagesDatas[0].m_Mipmaps.size() is 0!");
    return m_CPUData.m_ImagesDatas[0].m_Mipmaps[0].compressedSize > 0;
}
unsigned char* Texture::pixels() {
    Engine::priv::TextureLoader::WithdrawPixelsFromOpenGLMemory(*this);
    ASSERT(m_CPUData.m_ImagesDatas.size() > 0, __FUNCTION__ << "(): m_CPUData.m_ImagesDatas.size() is 0!");
    ASSERT(m_CPUData.m_ImagesDatas[0].m_Mipmaps.size() > 0, __FUNCTION__ << "(): m_CPUData.m_ImagesDatas[0].m_Mipmaps.size() is 0!");
    return &(m_CPUData.m_ImagesDatas[0].m_Mipmaps[0].pixels)[0];
}
