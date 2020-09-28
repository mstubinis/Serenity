#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/renderer/Renderer.h>
#include <core/engine/textures/Texture.h>

Texture* Texture::White    = nullptr;
Texture* Texture::Black    = nullptr;
Texture* Texture::Checkers = nullptr;
Texture* Texture::BRDF     = nullptr;

Texture::Texture() 
    : Resource{ ResourceType::Texture }
{
    Engine::priv::TextureLoader::InitCommon(*this, GL_TEXTURE_2D, false);
}

Texture::Texture(const std::string& textureName, TextureType textureType, unsigned int width, unsigned int height, bool mipMap) 
    : Resource{ ResourceType::Texture }
{
    setName(textureName);
    m_TextureType = textureType;
    m_Type = Engine::priv::TextureLoader::OpenglTypeFromTextureType(textureType);
    Engine::priv::TextureLoader::InitCommon(*this, m_Type, mipMap);
}
Texture::Texture(unsigned int w, unsigned int h, ImagePixelType pxlType, ImagePixelFormat pxlFormat, ImageInternalFormat internal_, float divisor) 
    : Resource{ ResourceType::Texture }
{
    Engine::priv::TextureLoader::InitFramebuffer(*this, w, h, pxlType, pxlFormat, internal_, divisor);
    Engine::priv::InternalTexturePublicInterface::Load(*this);
}
Texture::Texture(const sf::Image& sfImage, const std::string& name, bool genMipMaps, ImageInternalFormat internal_, unsigned int openglTextureType) 
    : Resource{ ResourceType::Texture, name }
{
    Engine::priv::TextureLoader::InitFromMemory(*this, sfImage, name, genMipMaps, internal_, openglTextureType);
    Engine::priv::InternalTexturePublicInterface::Load(*this);
}
Texture::Texture(const std::string& filename, bool genMipMaps, ImageInternalFormat internal_, unsigned int openglTextureType) 
    : Resource{ ResourceType::Texture, filename }
{
    Engine::priv::TextureLoader::InitFromFile(*this, filename, genMipMaps, internal_, openglTextureType);
    Engine::priv::InternalTexturePublicInterface::Load(*this);
}
Texture::Texture(const std::string files[], const std::string& name, bool genMipMaps, ImageInternalFormat internal_) 
    : Resource{ ResourceType::Texture, name }
{
    Engine::priv::TextureLoader::InitFromFilesCubemap(*this, files, name, genMipMaps, internal_);
    Engine::priv::InternalTexturePublicInterface::Load(*this);
}
Texture::~Texture(){
    Engine::priv::InternalTexturePublicInterface::Unload(*this);
}
bool Texture::internal_bind_if_not_bound(unsigned int requestedAddress) noexcept {
    auto whichID = Engine::Renderer::getCurrentlyBoundTextureOfType(requestedAddress);
    if (whichID != requestedAddress) {
        Engine::Renderer::bindTextureForModification(m_Type, requestedAddress);
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
    Texture::setXWrapping(m_Type, wrap);
}
void Texture::setYWrapping(TextureWrap wrap) {
    if (*this == false) {
        m_CommandQueue.emplace([this, wrap]() { setYWrapping(wrap); });
        return;
    }
    internal_bind_if_not_bound(m_TextureAddress);
    Texture::setYWrapping(m_Type, wrap);
}
void Texture::setZWrapping(TextureWrap wrap) {
    if (*this == false) {
        m_CommandQueue.emplace([this, wrap]() { setZWrapping(wrap); });
        return;
    }
    internal_bind_if_not_bound(m_TextureAddress);
    Texture::setZWrapping(m_Type, wrap);
}
void Texture::setWrapping(TextureWrap wrap) {
    if (*this == false) {
        m_CommandQueue.emplace([this, wrap]() { setWrapping(wrap); });
        return;
    }
    internal_bind_if_not_bound(m_TextureAddress);
    Texture::setWrapping(m_Type, wrap);
}
void Texture::setMinFilter(TextureFilter filter) {
    if (*this == false) {
        m_CommandQueue.emplace([this, filter]() { setMinFilter(filter); });
        return;
    }
    internal_bind_if_not_bound(m_TextureAddress);
    Texture::setMinFilter(m_Type, filter);
    m_MinFilter = (unsigned int)filter;
}
void Texture::setMaxFilter(TextureFilter filter) {
    if (*this == false) {
        m_CommandQueue.emplace([this, filter]() { setMaxFilter(filter); });
        return;
    }
    internal_bind_if_not_bound(m_TextureAddress);
    Texture::setMaxFilter(m_Type, filter);
}
void Texture::setFilter(TextureFilter filter) {
    if (*this == false) {
        m_CommandQueue.emplace([this, filter]() { setFilter(filter); });
        return;
    }
    internal_bind_if_not_bound(m_TextureAddress);
    Texture::setFilter(m_Type, filter);
}
void Texture::setXWrapping(unsigned int type, TextureWrap wrap) {
    unsigned int glXWrapEnum;
    Engine::priv::TextureLoader::EnumWrapToGL(glXWrapEnum, wrap);
    GLCall(glTexParameteri(type, GL_TEXTURE_WRAP_S, glXWrapEnum));
}
void Texture::setYWrapping(unsigned int type, TextureWrap wrap) {
    unsigned int glYWrapEnum;
    Engine::priv::TextureLoader::EnumWrapToGL(glYWrapEnum, wrap);
    GLCall(glTexParameteri(type, GL_TEXTURE_WRAP_T, glYWrapEnum));
}
void Texture::setZWrapping(unsigned int type, TextureWrap wrap) {
    if (type != GL_TEXTURE_CUBE_MAP) {
        ENGINE_PRODUCTION_LOG("Texture::setZWrapping() error: type was not GL_TEXTURE_CUBE_MAP!")
        return;
    }
    unsigned int glZWrapEnum;
    Engine::priv::TextureLoader::EnumWrapToGL(glZWrapEnum, wrap);
    GLCall(glTexParameteri(type, GL_TEXTURE_WRAP_R, glZWrapEnum));
}
void Texture::setWrapping(unsigned int type, TextureWrap wrap) {
    Texture::setXWrapping(type, wrap);
    Texture::setYWrapping(type, wrap);
    if (type == GL_TEXTURE_CUBE_MAP) {
        Texture::setZWrapping(type, wrap);
    }
}
void Texture::setMinFilter(unsigned int type, TextureFilter filter) {
    unsigned int glMinFilterEnum;
    Engine::priv::TextureLoader::EnumFilterToGL(glMinFilterEnum, filter, true);
    GLCall(glTexParameteri(type, GL_TEXTURE_MIN_FILTER, glMinFilterEnum));
}
void Texture::setMaxFilter(unsigned int type, TextureFilter filter) {
    unsigned int glMaxFilterEnum;
    Engine::priv::TextureLoader::EnumFilterToGL(glMaxFilterEnum, filter, false);
    GLCall(glTexParameteri(type, GL_TEXTURE_MAG_FILTER, glMaxFilterEnum));
}
void Texture::setFilter(unsigned int type, TextureFilter filter) {
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
        GLCall(glTexParameterf(m_Type, GL_TEXTURE_MAX_ANISOTROPY, anisotropicFiltering));
    }else{
        if (Engine::priv::OpenGLExtensions::supported(Engine::priv::OpenGLExtensions::ARB_texture_filter_anisotropic)) {
            GLCall(glTexParameterf(m_Type, GL_TEXTURE_MAX_ANISOTROPY, anisotropicFiltering));
        }else if (Engine::priv::OpenGLExtensions::supported(Engine::priv::OpenGLExtensions::EXT_texture_filter_anisotropic)) {
            GLCall(glTexParameterf(m_Type, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropicFiltering));
        }
    }
}
glm::vec2 Texture::sizeAsRatio() const {
    float max_val = glm::max((float)size().x, (float)size().y);
    return glm::vec2(glm::vec2(size()) / max_val);
}
bool Texture::compressed() const {
    return (m_ImagesDatas.size() == 0) ? false : (m_ImagesDatas[0].m_Mipmaps[0].compressedSize > 0);
}
unsigned char* Texture::pixels() {
    Engine::priv::TextureLoader::WithdrawPixelsFromOpenGLMemory(*this);
    return &(m_ImagesDatas[0].m_Mipmaps[0].pixels)[0];
}
