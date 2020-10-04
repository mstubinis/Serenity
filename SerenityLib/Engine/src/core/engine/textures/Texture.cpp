#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/renderer/Renderer.h>
#include <core/engine/textures/Texture.h>

Handle Texture::White    = Handle{};
Handle Texture::Black    = Handle{};
Handle Texture::Checkers = Handle{};
Handle Texture::BRDF     = Handle{};

Texture::Texture(const std::string& name) 
    : Resource{ ResourceType::Texture, name }
{
    m_CPUData.m_ImagesDatas.emplace_back();
    Engine::priv::TextureLoader::CPUInitCommon(m_CPUData, TextureType::Texture2D, false);
}

Texture::Texture(const std::string& textureName, TextureType textureType, unsigned int width, unsigned int height, bool mipMap) 
    : Texture{}
{
    setName(textureName);
    m_CPUData.m_TextureType = textureType;
    Engine::priv::TextureLoader::CPUInitCommon(m_CPUData, m_CPUData.m_TextureType, mipMap);
}
Texture::Texture(unsigned int w, unsigned int h, ImagePixelType pxlType, ImagePixelFormat pxlFormat, ImageInternalFormat internal_, float divisor) 
    : Texture{}
{
    Engine::priv::TextureLoader::CPUInitFramebuffer(m_CPUData, w, h, pxlType, pxlFormat, internal_, divisor);
    Engine::priv::InternalTexturePublicInterface::Load(*this);
}
Texture::Texture(const sf::Image& sfImage, const std::string& name, bool genMipMaps, ImageInternalFormat internal_, TextureType textureType)
    : Texture{ name }
{
    Engine::priv::TextureLoader::CPUInitFromMemory(m_CPUData, sfImage, name, genMipMaps, internal_, textureType);
    Engine::priv::InternalTexturePublicInterface::Load(*this);
}
Texture::Texture(const std::string& filename, bool genMipMaps, ImageInternalFormat internal_, TextureType textureType) 
    : Texture{ filename }
{
    Engine::priv::TextureLoader::CPUInitFromFile(m_CPUData, filename, genMipMaps, internal_, textureType);
    Engine::priv::InternalTexturePublicInterface::Load(*this);
}
Texture::Texture(const std::array<std::string, 6>& files, const std::string& name, bool genMipMaps, ImageInternalFormat internal_) 
    : Texture{ name }
{
    Engine::priv::TextureLoader::CPUInitFromFilesCubemap(m_CPUData, files, name, genMipMaps, internal_);
    Engine::priv::InternalTexturePublicInterface::Load(*this);
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
    Engine::priv::InternalTexturePublicInterface::Unload(*this);
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
    if (type.toGLType() == GL_TEXTURE_CUBE_MAP) {
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
    if (Engine::priv::Renderer::OPENGL_VERSION >= 46) {
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
    return (m_CPUData.m_ImagesDatas.size() == 0) ? false : (m_CPUData.m_ImagesDatas[0].m_Mipmaps[0].compressedSize > 0);
}
unsigned char* Texture::pixels() {
    Engine::priv::TextureLoader::WithdrawPixelsFromOpenGLMemory(*this);
    return &(m_CPUData.m_ImagesDatas[0].m_Mipmaps[0].pixels)[0];
}
