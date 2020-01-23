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

Texture::Texture() : EngineResource(ResourceType::Texture) {
    TextureLoader::InitCommon(*this, GL_TEXTURE_2D, false);
}
Texture::Texture(const uint& w, const uint& h, const ImagePixelType::Type& pxlType, const ImagePixelFormat::Format& pxlFormat, const ImageInternalFormat::Format& _internal, const float& divisor) : EngineResource(ResourceType::Texture) {
    TextureLoader::InitFramebuffer(*this, w, h, pxlType, pxlFormat, _internal, divisor);
    InternalTexturePublicInterface::Load(*this);
}
Texture::Texture(const sf::Image& sfImage, const string& name, const bool& genMipMaps, const ImageInternalFormat::Format& _internal, const GLuint& openglTextureType) : EngineResource(ResourceType::Texture, name) {
    TextureLoader::InitFromMemory(*this, sfImage, name, genMipMaps, _internal, openglTextureType);
    InternalTexturePublicInterface::Load(*this);
}
Texture::Texture(const string& filename, const bool& genMipMaps, const ImageInternalFormat::Format& _internal, const GLuint& openglTextureType) : EngineResource(ResourceType::Texture, filename) {
    TextureLoader::InitFromFile(*this, filename, genMipMaps, _internal, openglTextureType);
    InternalTexturePublicInterface::Load(*this);
}
Texture::Texture(const string files[], const string& name, const bool& genMipMaps, const ImageInternalFormat::Format& _internal) : EngineResource(ResourceType::Texture, name) {
    TextureLoader::InitFromFilesCubemap(*this, files, name, genMipMaps, _internal);
    InternalTexturePublicInterface::Load(*this);
}
Texture::~Texture(){
    InternalTexturePublicInterface::Unload(*this);
    //SAFE_DELETE_VECTOR(m_ImagesDatas); //TODO: see if this is needed
}
void Texture::render(const glm::vec2& position, const glm::vec4& color, const float& angle, const glm::vec2& scale, const float& depth){
    if (m_TextureType == TextureType::CubeMap)
        return;
    Engine::Renderer::renderTexture(*this, position, color, angle, scale, depth);
}
void Texture::setXWrapping(const TextureWrap::Wrap& wrap){
    //Renderer::bindTextureForModification(m_Type, m_TextureAddress[0]);
    Texture::setXWrapping(m_Type, wrap);
}
void Texture::setYWrapping(const TextureWrap::Wrap& wrap){
    //Renderer::bindTextureForModification(m_Type, m_TextureAddress[0]);
    Texture::setYWrapping(m_Type, wrap);
}
void Texture::setZWrapping(const TextureWrap::Wrap& wrap){
    //Renderer::bindTextureForModification(m_Type, m_TextureAddress[0]);
    Texture::setZWrapping(m_Type, wrap);
}
void Texture::setWrapping(const TextureWrap::Wrap& wrap){
    //Renderer::bindTextureForModification(m_Type, m_TextureAddress[0]);
    Texture::setWrapping(m_Type, wrap);
}
void Texture::setMinFilter(const TextureFilter::Filter& filter){
    //Renderer::bindTextureForModification(m_Type, m_TextureAddress[0]);
    Texture::setMinFilter(m_Type, filter);
    m_MinFilter = filter;
}
void Texture::setMaxFilter(const TextureFilter::Filter& filter){
    //Renderer::bindTextureForModification(m_Type, m_TextureAddress[0]);
    Texture::setMaxFilter(m_Type, filter);
}
void Texture::setFilter(const TextureFilter::Filter& filter){
    //Renderer::bindTextureForModification(m_Type, m_TextureAddress[0]);
    Texture::setFilter(m_Type, filter);
}
void Texture::setXWrapping(const GLuint& type, const TextureWrap::Wrap& wrap){
    GLuint gl;
    TextureLoader::EnumWrapToGL(gl, wrap);
    glTexParameteri(type, GL_TEXTURE_WRAP_S, gl);
}
void Texture::setYWrapping(const GLuint& type, const TextureWrap::Wrap& wrap){
    GLuint gl;
    TextureLoader::EnumWrapToGL(gl, wrap);
    glTexParameteri(type, GL_TEXTURE_WRAP_T, gl);
}
void Texture::setZWrapping(const GLuint& type, const TextureWrap::Wrap& wrap){
    if (type != GL_TEXTURE_CUBE_MAP)
        return;
    GLuint gl;
    TextureLoader::EnumWrapToGL(gl, wrap);
    glTexParameteri(type, GL_TEXTURE_WRAP_R, gl);
}
void Texture::setWrapping(const GLuint& type, const TextureWrap::Wrap& wrap){
    Texture::setXWrapping(type, wrap);
    Texture::setYWrapping(type, wrap);
    Texture::setZWrapping(type, wrap);
}
void Texture::setMinFilter(const GLuint& type, const TextureFilter::Filter& filter){
    GLuint gl;
    TextureLoader::EnumFilterToGL(gl, filter, true);
    glTexParameteri(type, GL_TEXTURE_MIN_FILTER, gl);
}
void Texture::setMaxFilter(const GLuint& type, const TextureFilter::Filter& filter){
    GLuint gl;
    TextureLoader::EnumFilterToGL(gl, filter, false);
    glTexParameteri(type, GL_TEXTURE_MAG_FILTER, gl);
}
void Texture::setFilter(const GLuint& type, const TextureFilter::Filter& filter){
    Texture::setMinFilter(type, filter);
    Texture::setMaxFilter(type, filter);
}
void Texture::setAnisotropicFiltering(const float& anisotropicFiltering){
    if (m_TextureAddress.size() == 0)
        return;
    Engine::Renderer::bindTextureForModification(m_Type, m_TextureAddress[0]);
    if(Engine::priv::Renderer::OPENGL_VERSION >= 46){
    	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, const_cast<GLfloat*>(&anisotropicFiltering));
    	glTexParameterf(m_Type, GL_TEXTURE_MAX_ANISOTROPY, anisotropicFiltering);
    }else{     
        if(OpenGLExtensions::supported(OpenGLExtensions::ARB_Ansiotropic_Filtering)){
            //glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_ARB, const_cast<GLfloat*>(&anisotropicFiltering));
            //glTexParameterf(m_Type, GL_TEXTURE_MAX_ANISOTROPY_ARB, anisotropicFiltering);
        }else if(OpenGLExtensions::supported(OpenGLExtensions::EXT_Ansiotropic_Filtering)){
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, const_cast<GLfloat*>(&anisotropicFiltering));
            glTexParameterf(m_Type, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropicFiltering);
        }
    }
}

const bool Texture::mipmapped() const {
    return m_Mipmapped; 
}
const bool Texture::compressed() const {
    if (m_ImagesDatas.size() == 0)
        return false;
    const auto& img = m_ImagesDatas[0].get();
    auto& mip       = img->mipmaps[0];
    return (mip.compressedSize > 0) ? true : false;
}
const uchar* Texture::pixels(){
    TextureLoader::WithdrawPixelsFromOpenGLMemory(*this); 
    return &(m_ImagesDatas[0]->mipmaps[0].pixels)[0]; 
}
const GLuint& Texture::address(const uint& index) const { 
    return m_TextureAddress[index]; 
}
const size_t Texture::numAddresses() const {
    return m_TextureAddress.size(); 
}
const GLuint Texture::type() const { 
    return m_Type; 
}
const uint Texture::width() const { 
    return m_ImagesDatas[0]->mipmaps[0].width; 
}
const uint Texture::height() const { 
    return m_ImagesDatas[0]->mipmaps[0].height; 
}
const glm::uvec2 Texture::size() const {
    return glm::uvec2(width(), height());
}

const ImageInternalFormat::Format Texture::internalFormat() const { 
    return m_ImagesDatas[0]->internalFormat; 
}
const ImagePixelFormat::Format Texture::pixelFormat() const { 
    return m_ImagesDatas[0]->pixelFormat; 
}
const ImagePixelType::Type Texture::pixelType() const { 
    return m_ImagesDatas[0]->pixelType; 
}
