#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/renderer/Renderer.h>
#include <core/engine/textures/Texture.h>

using namespace Engine;
using namespace Engine::priv;

Texture* Texture::White    = nullptr;
Texture* Texture::Black    = nullptr;
Texture* Texture::Checkers = nullptr;
Texture* Texture::BRDF     = nullptr;

Texture::Texture() : Resource(ResourceType::Texture) {
    TextureLoader::InitCommon(*this, GL_TEXTURE_2D, false);
}
Texture::Texture(unsigned int w, unsigned int h, ImagePixelType pxlType, ImagePixelFormat pxlFormat, ImageInternalFormat internal_, float divisor) : Resource(ResourceType::Texture) {
    TextureLoader::InitFramebuffer(*this, w, h, pxlType, pxlFormat, internal_, divisor);
    InternalTexturePublicInterface::Load(*this);
}
Texture::Texture(const sf::Image& sfImage, const std::string& name, bool genMipMaps, ImageInternalFormat internal_, unsigned int openglTextureType) : Resource(ResourceType::Texture, name) {
    TextureLoader::InitFromMemory(*this, sfImage, name, genMipMaps, internal_, openglTextureType);
    InternalTexturePublicInterface::Load(*this);
}
Texture::Texture(const std::string& filename, bool genMipMaps, ImageInternalFormat internal_, unsigned int openglTextureType) : Resource(ResourceType::Texture, filename) {
    TextureLoader::InitFromFile(*this, filename, genMipMaps, internal_, openglTextureType);
    InternalTexturePublicInterface::Load(*this);
}
Texture::Texture(const std::string files[], const std::string& name, bool genMipMaps, ImageInternalFormat internal_) : Resource(ResourceType::Texture, name) {
    TextureLoader::InitFromFilesCubemap(*this, files, name, genMipMaps, internal_);
    InternalTexturePublicInterface::Load(*this);
}
Texture::~Texture(){
    InternalTexturePublicInterface::Unload(*this);
}
bool Texture::operator==(const bool rhs) const {
    if (rhs == true) {
        return (m_TextureAddresses.size() > 0);
    }
    return !(m_TextureAddresses.size() > 0);
}
Texture::operator bool() const {
    return (m_TextureAddresses.size() > 0);
}

void Texture::setXWrapping(TextureWrap wrap){
    if (*this == false) {
        m_CommandQueue.push([this, wrap]() { setXWrapping(wrap); });
        return;
    }
    //Renderer::bindTextureForModification(m_Type, m_TextureAddress[0]);
    Texture::setXWrapping(m_Type, wrap);
}
void Texture::setYWrapping(TextureWrap wrap){
    if (*this == false) {
        m_CommandQueue.push([this, wrap]() { setYWrapping(wrap); });
        return;
    }
    //Renderer::bindTextureForModification(m_Type, m_TextureAddress[0]);
    Texture::setYWrapping(m_Type, wrap);
}
void Texture::setZWrapping(TextureWrap wrap){
    if (*this == false) {
        m_CommandQueue.push([this, wrap]() { setZWrapping(wrap); });
        return;
    }
    //Renderer::bindTextureForModification(m_Type, m_TextureAddress[0]);
    Texture::setZWrapping(m_Type, wrap);
}
void Texture::setWrapping(TextureWrap wrap){
    if (*this == false) {
        m_CommandQueue.push([this, wrap]() { setWrapping(wrap); });
        return;
    }
    //Renderer::bindTextureForModification(m_Type, m_TextureAddress[0]);
    Texture::setWrapping(m_Type, wrap);
}
void Texture::setMinFilter(TextureFilter filter){
    if (*this == false) {
        m_CommandQueue.push([this, filter]() { setMinFilter(filter); });
        return;
    }
    //Renderer::bindTextureForModification(m_Type, m_TextureAddress[0]);
    Texture::setMinFilter(m_Type, filter);
    m_MinFilter = (unsigned int)filter;
}
void Texture::setMaxFilter(TextureFilter filter){
    if (*this == false) {
        m_CommandQueue.push([this, filter]() { setMaxFilter(filter); });
        return;
    }
    //Renderer::bindTextureForModification(m_Type, m_TextureAddress[0]);
    Texture::setMaxFilter(m_Type, filter);
}
void Texture::setFilter(TextureFilter filter){
    if (*this == false) {
        m_CommandQueue.push([this, filter]() { setFilter(filter); });
        return;
    }
    //Renderer::bindTextureForModification(m_Type, m_TextureAddress[0]);
    Texture::setFilter(m_Type, filter);
}
void Texture::setXWrapping(unsigned int type, TextureWrap wrap){
    unsigned int gl;
    TextureLoader::EnumWrapToGL(gl, wrap);
    glTexParameteri(type, GL_TEXTURE_WRAP_S, gl);
}
void Texture::setYWrapping(unsigned int type, TextureWrap wrap){
    unsigned int gl;
    TextureLoader::EnumWrapToGL(gl, wrap);
    glTexParameteri(type, GL_TEXTURE_WRAP_T, gl);
}
void Texture::setZWrapping(unsigned int type, TextureWrap wrap){
    if (type != GL_TEXTURE_CUBE_MAP) {
        return;
    }
    unsigned int gl;
    TextureLoader::EnumWrapToGL(gl, wrap);
    glTexParameteri(type, GL_TEXTURE_WRAP_R, gl);
}
void Texture::setWrapping(unsigned int type, TextureWrap wrap){
    Texture::setXWrapping(type, wrap);
    Texture::setYWrapping(type, wrap);
    Texture::setZWrapping(type, wrap);
}
void Texture::setMinFilter(unsigned int type, TextureFilter filter){
    unsigned int gl;
    TextureLoader::EnumFilterToGL(gl, filter, true);
    glTexParameteri(type, GL_TEXTURE_MIN_FILTER, gl);
}
void Texture::setMaxFilter(unsigned int type, TextureFilter filter){
    unsigned int gl;
    TextureLoader::EnumFilterToGL(gl, filter, false);
    glTexParameteri(type, GL_TEXTURE_MAG_FILTER, gl);
}
void Texture::setFilter(unsigned int type, TextureFilter filter){
    Texture::setMinFilter(type, filter);
    Texture::setMaxFilter(type, filter);
}
void Texture::setAnisotropicFiltering(float anisotropicFiltering){
    if (*this == false) {
        auto lamda = [this, anisotropicFiltering]() {
            setAnisotropicFiltering(anisotropicFiltering);
        };
        m_CommandQueue.push(lamda);
        return;
    }
    Engine::Renderer::bindTextureForModification(m_Type, m_TextureAddresses[0]);
    if(Engine::priv::Renderer::OPENGL_VERSION >= 46){
    	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, const_cast<GLfloat*>(&anisotropicFiltering));
    	glTexParameterf(m_Type, GL_TEXTURE_MAX_ANISOTROPY, anisotropicFiltering);
    }else{
        if(OpenGLExtensions::supported(OpenGLExtensions::ARB_texture_filter_anisotropic)){
            //glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, const_cast<GLfloat*>(&anisotropicFiltering));
            //glTexParameterf(m_Type, GL_TEXTURE_MAX_ANISOTROPY, anisotropicFiltering);
        }else if(OpenGLExtensions::supported(OpenGLExtensions::EXT_texture_filter_anisotropic)){
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, const_cast<GLfloat*>(&anisotropicFiltering));
            glTexParameterf(m_Type, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropicFiltering);
        }
    }
}
glm::vec2 Texture::sizeAsRatio() const {
    float max_val = glm::max((float)size().x, (float)size().y);
    return glm::vec2(glm::vec2(size()) / max_val);
}
bool Texture::compressed() const {
    if (m_ImagesDatas.size() == 0) {
        return false;
    }
    return (m_ImagesDatas[0].m_Mipmaps[0].compressedSize > 0);
}
unsigned char* Texture::pixels() {
    TextureLoader::WithdrawPixelsFromOpenGLMemory(*this); 
    return &(m_ImagesDatas[0].m_Mipmaps[0].pixels)[0];
}
GLuint Texture::address(unsigned int index) const {
    return (m_TextureAddresses.size() == 0) ? 0 : m_TextureAddresses[index];
}
size_t Texture::numAddresses() const {
    return m_TextureAddresses.size(); 
}
unsigned int Texture::width() const {
    return m_ImagesDatas[0].m_Mipmaps[0].width;
}
unsigned int Texture::height() const {
    return m_ImagesDatas[0].m_Mipmaps[0].height; 
}
glm::uvec2 Texture::size() const {
    return glm::uvec2(width(), height());
}
ImageInternalFormat Texture::internalFormat() const { 
    return m_ImagesDatas[0].m_InternalFormat; 
}
ImagePixelFormat Texture::pixelFormat() const { 
    return m_ImagesDatas[0].m_PixelFormat; 
}
ImagePixelType Texture::pixelType() const { 
    return m_ImagesDatas[0].m_PixelType; 
}