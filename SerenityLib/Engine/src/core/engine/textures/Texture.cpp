#include <core/engine/system/Engine.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/textures/DDS.h>
#include <core/engine/system/window/Window.h>
#include <core/engine/math/Engine_Math.h>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <iostream>
#include <fstream>

using namespace Engine;
using namespace Engine::priv;
using namespace Engine::priv::textures;
using namespace std;

Texture* Texture::White    = nullptr;
Texture* Texture::Black    = nullptr;
Texture* Texture::Checkers = nullptr;
Texture* Texture::BRDF     = nullptr;

Texture::Texture() : Resource(ResourceType::Texture) {
    TextureLoader::InitCommon(*this, GL_TEXTURE_2D, false);
}
Texture::Texture(unsigned int w, unsigned int h, ImagePixelType::Type pxlType, ImagePixelFormat::Format pxlFormat, ImageInternalFormat::Format internal_, float divisor) : Resource(ResourceType::Texture) {
    TextureLoader::InitFramebuffer(*this, w, h, pxlType, pxlFormat, internal_, divisor);
    InternalTexturePublicInterface::Load(*this);
}
Texture::Texture(const sf::Image& sfImage, const string& name, bool genMipMaps, ImageInternalFormat::Format internal_, unsigned int openglTextureType) : Resource(ResourceType::Texture, name) {
    TextureLoader::InitFromMemory(*this, sfImage, name, genMipMaps, internal_, openglTextureType);
    InternalTexturePublicInterface::Load(*this);
}
Texture::Texture(const string& filename, bool genMipMaps, ImageInternalFormat::Format internal_, unsigned int openglTextureType) : Resource(ResourceType::Texture, filename) {
    TextureLoader::InitFromFile(*this, filename, genMipMaps, internal_, openglTextureType);
    InternalTexturePublicInterface::Load(*this);
}
Texture::Texture(const string files[], const string& name, bool genMipMaps, ImageInternalFormat::Format internal_) : Resource(ResourceType::Texture, name) {
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

void Texture::setXWrapping(TextureWrap::Wrap wrap){
    if (*this == false) {
        m_CommandQueue.push([this, wrap]() { setXWrapping(wrap); });
        return;
    }
    //Renderer::bindTextureForModification(m_Type, m_TextureAddress[0]);
    Texture::setXWrapping(m_Type, wrap);
}
void Texture::setYWrapping(TextureWrap::Wrap wrap){
    if (*this == false) {
        m_CommandQueue.push([this, wrap]() { setYWrapping(wrap); });
        return;
    }
    //Renderer::bindTextureForModification(m_Type, m_TextureAddress[0]);
    Texture::setYWrapping(m_Type, wrap);
}
void Texture::setZWrapping(TextureWrap::Wrap wrap){
    if (*this == false) {
        m_CommandQueue.push([this, wrap]() { setZWrapping(wrap); });
        return;
    }
    //Renderer::bindTextureForModification(m_Type, m_TextureAddress[0]);
    Texture::setZWrapping(m_Type, wrap);
}
void Texture::setWrapping(TextureWrap::Wrap wrap){
    if (*this == false) {
        m_CommandQueue.push([this, wrap]() { setWrapping(wrap); });
        return;
    }
    //Renderer::bindTextureForModification(m_Type, m_TextureAddress[0]);
    Texture::setWrapping(m_Type, wrap);
}
void Texture::setMinFilter(TextureFilter::Filter filter){
    if (*this == false) {
        m_CommandQueue.push([this, filter]() { setMinFilter(filter); });
        return;
    }
    //Renderer::bindTextureForModification(m_Type, m_TextureAddress[0]);
    Texture::setMinFilter(m_Type, filter);
    m_MinFilter = filter;
}
void Texture::setMaxFilter(TextureFilter::Filter filter){
    if (*this == false) {
        m_CommandQueue.push([this, filter]() { setMaxFilter(filter); });
        return;
    }
    //Renderer::bindTextureForModification(m_Type, m_TextureAddress[0]);
    Texture::setMaxFilter(m_Type, filter);
}
void Texture::setFilter(TextureFilter::Filter filter){
    if (*this == false) {
        m_CommandQueue.push([this, filter]() { setFilter(filter); });
        return;
    }
    //Renderer::bindTextureForModification(m_Type, m_TextureAddress[0]);
    Texture::setFilter(m_Type, filter);
}
void Texture::setXWrapping(unsigned int type, TextureWrap::Wrap wrap){
    unsigned int gl;
    TextureLoader::EnumWrapToGL(gl, wrap);
    glTexParameteri(type, GL_TEXTURE_WRAP_S, gl);
}
void Texture::setYWrapping(unsigned int type, TextureWrap::Wrap wrap){
    unsigned int gl;
    TextureLoader::EnumWrapToGL(gl, wrap);
    glTexParameteri(type, GL_TEXTURE_WRAP_T, gl);
}
void Texture::setZWrapping(unsigned int type, TextureWrap::Wrap wrap){
    if (type != GL_TEXTURE_CUBE_MAP) {
        return;
    }
    unsigned int gl;
    TextureLoader::EnumWrapToGL(gl, wrap);
    glTexParameteri(type, GL_TEXTURE_WRAP_R, gl);
}
void Texture::setWrapping(unsigned int type, TextureWrap::Wrap wrap){
    Texture::setXWrapping(type, wrap);
    Texture::setYWrapping(type, wrap);
    Texture::setZWrapping(type, wrap);
}
void Texture::setMinFilter(unsigned int type, TextureFilter::Filter filter){
    unsigned int gl;
    TextureLoader::EnumFilterToGL(gl, filter, true);
    glTexParameteri(type, GL_TEXTURE_MIN_FILTER, gl);
}
void Texture::setMaxFilter(unsigned int type, TextureFilter::Filter filter){
    unsigned int gl;
    TextureLoader::EnumFilterToGL(gl, filter, false);
    glTexParameteri(type, GL_TEXTURE_MAG_FILTER, gl);
}
void Texture::setFilter(unsigned int type, TextureFilter::Filter filter){
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

bool Texture::mipmapped() const {
    return m_Mipmapped; 
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
unsigned int Texture::type() const {
    return m_Type; 
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
ImageInternalFormat::Format Texture::internalFormat() const { 
    return m_ImagesDatas[0].m_InternalFormat; 
}
ImagePixelFormat::Format Texture::pixelFormat() const { 
    return m_ImagesDatas[0].m_PixelFormat; 
}
ImagePixelType::Type Texture::pixelType() const { 
    return m_ImagesDatas[0].m_PixelType; 
}