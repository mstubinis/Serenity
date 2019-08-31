#include <core/engine/Engine.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/textures/DDS.h>
#include <core/engine/Engine_Window.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/renderer/FramebufferObject.h>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include <SFML/Graphics.hpp>

#include <iostream>
#include <fstream>

using namespace Engine;
using namespace Engine::epriv;
using namespace Engine::epriv::textures;
using namespace std;

Texture* Texture::White    = nullptr;
Texture* Texture::Black    = nullptr;
Texture* Texture::Checkers = nullptr;
Texture* Texture::BRDF     = nullptr;

Texture::Texture(const uint& w, const uint& h, const ImagePixelType::Type& pxlType, const ImagePixelFormat::Format& pxlFormat, const ImageInternalFormat::Format& _internal, const float& divisor){
    m_TextureType = TextureType::RenderTarget;
    uint _width(uint(float(w) * divisor));
    uint _height(uint(float(h) * divisor));
    ImageLoadedStructure* image = new ImageLoadedStructure(_width, _height, pxlType, pxlFormat, _internal);

    init_common(GL_TEXTURE_2D, false);

    m_ImagesDatas.push_back(image);
    setName("RenderTarget");
    load();
}
Texture::Texture(const sf::Image& sfImage, const string& name, const bool& genMipMaps, const ImageInternalFormat::Format& _internal, const GLuint& openglTextureType){
    m_TextureType = TextureType::Texture2D;
    ImageLoadedStructure* image = new ImageLoadedStructure(sfImage, name);
    image->pixelType = ImagePixelType::UNSIGNED_BYTE;
    image->internalFormat = _internal;

    init_common(openglTextureType, genMipMaps);

    TextureLoader::ChoosePixelFormat(image->pixelFormat, image->internalFormat);
    m_ImagesDatas.push_back(image);
    setName(name);
    load();
}
Texture::Texture(const string& filename, const bool& genMipMaps, const ImageInternalFormat::Format& _internal, const GLuint& openglTextureType){
    m_TextureType = TextureType::Texture2D;
    ImageLoadedStructure* image = new ImageLoadedStructure();
    image->filename = filename;
    const string& extension = boost::filesystem::extension(filename);
    init_common(openglTextureType, genMipMaps);
    if (extension == ".dds") {
        TextureLoader::LoadDDSFile(*this, filename, *image);
    }else{
        image->pixelType = ImagePixelType::UNSIGNED_BYTE;
        image->internalFormat = _internal;
    }
    TextureLoader::ChoosePixelFormat(image->pixelFormat, image->internalFormat);
    m_ImagesDatas.insert(m_ImagesDatas.begin(), image);

    setName(filename);
    load();
}
Texture::Texture(const string files[], const string& name, const bool& genMipMaps, const ImageInternalFormat::Format& _internal){
    m_TextureType = TextureType::CubeMap;
    for(uint j = 0; j < 6; ++j){ 
        ImageLoadedStructure* image = new ImageLoadedStructure();
        image->filename = files[j];
        m_ImagesDatas.push_back(image);
    }
    for (auto& sideImage : m_ImagesDatas) {
        m_Type = GL_TEXTURE_CUBE_MAP;
        sideImage->pixelType = ImagePixelType::UNSIGNED_BYTE;
        sideImage->internalFormat = _internal;
        TextureLoader::ChoosePixelFormat(sideImage->pixelFormat, sideImage->internalFormat);
    }
    setName(name);
    load();
}
Texture::~Texture(){
    unload();
    SAFE_DELETE_VECTOR(m_ImagesDatas);
}
void Texture::init_common(const GLuint& _openglTextureType, const bool& _toBeMipmapped) {
    m_Mipmapped = false;
    m_IsToBeMipmapped = _toBeMipmapped;
    m_MinFilter = GL_LINEAR;
    m_Type = _openglTextureType;
}
void Texture::render(const glm::vec2& position, const glm::vec4& color, const float& angle, const glm::vec2& scale, const float& depth){
    if (m_TextureType == TextureType::CubeMap)
        return;
    Renderer::renderTexture(*this, position, color, angle, scale, depth);
}
void Texture::setXWrapping(const TextureWrap::Wrap& wrap){
    Texture::setXWrapping(m_Type, wrap);
}
void Texture::setYWrapping(const TextureWrap::Wrap& wrap){
    Texture::setYWrapping(m_Type, wrap);
}
void Texture::setZWrapping(const TextureWrap::Wrap& wrap){
    Texture::setZWrapping(m_Type, wrap);
}
void Texture::setWrapping(const TextureWrap::Wrap& wrap){
    Texture::setWrapping(m_Type, wrap);
}
void Texture::setMinFilter(const TextureFilter::Filter& filter){
    Texture::setMinFilter(m_Type, filter);
    m_MinFilter = filter;
}
void Texture::setMaxFilter(const TextureFilter::Filter& filter){
    Texture::setMaxFilter(m_Type, filter);
}
void Texture::setFilter(const TextureFilter::Filter& filter){
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
    Renderer::bindTextureForModification(m_Type, m_TextureAddress[0]);
    if(RenderManager::OPENGL_VERSION >= 46){
    	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY, const_cast<GLfloat*>(&anisotropicFiltering));
    	glTexParameterf(m_Type, GL_TEXTURE_MAX_ANISOTROPY, anisotropicFiltering);
    }else{     
        if(OpenGLExtension::supported(OpenGLExtension::ARB_Ansiotropic_Filtering)){
            //glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_ARB, const_cast<GLfloat*>(&anisotropicFiltering));
            //glTexParameterf(m_Type, GL_TEXTURE_MAX_ANISOTROPY_ARB, anisotropicFiltering);
        }else if(OpenGLExtension::supported(OpenGLExtension::EXT_Ansiotropic_Filtering)){
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, const_cast<GLfloat*>(&anisotropicFiltering));
            glTexParameterf(m_Type, GL_TEXTURE_MAX_ANISOTROPY_EXT, anisotropicFiltering);
        }
    }
}

void InternalTexturePublicInterface::LoadCPU(Texture& texture){
    for (auto& image : texture.m_ImagesDatas) {
        if (image->filename != "") {
            bool _do = false;
            if (image->mipmaps.size() == 0) { _do = true; }
            for (auto& mip : image->mipmaps) {
                if (mip.pixels.size() == 0) { _do = true; }
            }
            if (_do) {
                const string& extension = boost::filesystem::extension(image->filename);
                if (extension == ".dds") {
                    TextureLoader::LoadDDSFile(texture, image->filename, *image);
                }else{
                    sf::Image _sfImage;
                    _sfImage.loadFromFile(image->filename);
                    image->load(_sfImage, image->filename);
                }
            }
        }
    }
}
void InternalTexturePublicInterface::LoadGPU(Texture& texture){
    InternalTexturePublicInterface::UnloadGPU(texture);
    if (texture.m_TextureAddress.size() == 0)
        texture.m_TextureAddress.push_back(0);
    Renderer::genAndBindTexture(texture.m_Type, texture.m_TextureAddress[0]);
    switch (texture.m_TextureType) {
        case TextureType::RenderTarget: {
            TextureLoader::LoadTextureFramebufferIntoOpenGL(texture);
            break;
        }case TextureType::Texture1D: {
            break;
        }case TextureType::Texture2D: {
            TextureLoader::LoadTexture2DIntoOpenGL(texture);
            break;
        }case TextureType::Texture3D: {
            break;
        }case TextureType::CubeMap: {
            TextureLoader::LoadTextureCubemapIntoOpenGL(texture);
            break;
        }
        default: { break; }
    }
    if (texture.m_IsToBeMipmapped) {
        TextureLoader::GenerateMipmapsOpenGL(texture);
    }
    texture.EngineResource::load();
}
void InternalTexturePublicInterface::UnloadCPU(Texture& texture){
    for (auto& image : texture.m_ImagesDatas) {
        if (image->filename != "") {
            for (auto& mipmap : image->mipmaps) {
                if (mipmap.pixels.size() == 0) {
                    vector_clear(mipmap.pixels);
                }
            }
        }
    }
    texture.m_Mipmapped = false;
    texture.EngineResource::unload();
}
void InternalTexturePublicInterface::UnloadGPU(Texture& texture){
    for (uint i = 0; i < texture.m_TextureAddress.size(); ++i) {
        glDeleteTextures(1, &texture.m_TextureAddress[i]);
    }
    vector_clear(texture.m_TextureAddress);
}


void Texture::load(){
    if(!isLoaded()){
        auto& _this = *this;
        InternalTexturePublicInterface::LoadCPU(_this);
        InternalTexturePublicInterface::LoadGPU(_this);
        cout << "(Texture) ";
        EngineResource::load();
    }
}
void Texture::unload(){
    if(isLoaded()){
        auto& _this = *this;
        InternalTexturePublicInterface::UnloadGPU(_this);
        InternalTexturePublicInterface::UnloadCPU(_this);
        cout << "(Texture) ";
        EngineResource::unload();
    }
}
void Texture::resize(epriv::FramebufferObject& fbo, const uint& w, const uint& h){
    if (m_TextureType != TextureType::RenderTarget) {
        cout << "Error: Non-framebuffer texture cannot be resized. Returning..." << endl;
        return;
    }
    const float _divisor = fbo.divisor();
    Renderer::bindTextureForModification(m_Type, m_TextureAddress[0]);
    const uint _w = static_cast<uint>(static_cast<float>(w) * _divisor);
    const uint _h = static_cast<uint>(static_cast<float>(h) * _divisor);
    auto& imageData = *m_ImagesDatas[0];
    imageData.mipmaps[0].width = _w;
    imageData.mipmaps[0].height = _h;
    glTexImage2D(m_Type, 0, imageData.internalFormat, _w, _h, 0, imageData.pixelFormat, imageData.pixelType, NULL);
}
const bool Texture::mipmapped() const {
    return m_Mipmapped; 
}
const bool Texture::compressed() const {
    return (m_ImagesDatas[0]->mipmaps[0].compressedSize > 0) ? true : false;
}
const uchar* Texture::pixels(){
    TextureLoader::WithdrawPixelsFromOpenGLMemory(*this); 
    return &(m_ImagesDatas[0]->mipmaps[0].pixels)[0]; 
}
const GLuint& Texture::address(const uint& index) const { 
    return m_TextureAddress[index]; 
}
const uint Texture::numAddresses() const {
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
const ImageInternalFormat::Format Texture::internalFormat() const { 
    return m_ImagesDatas[0]->internalFormat; 
}
const ImagePixelFormat::Format Texture::pixelFormat() const { 
    return m_ImagesDatas[0]->pixelFormat; 
}
const ImagePixelType::Type Texture::pixelType() const { 
    return m_ImagesDatas[0]->pixelType; 
}
