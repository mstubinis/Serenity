#include <core/engine/renderer/FramebufferObject.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/textures/Texture.h>

#include <iostream>

using namespace Engine;
using namespace std;

#pragma region FramebufferObjectAttatchmentBaseClass


epriv::FramebufferObjectAttatchment::FramebufferObjectAttatchment(const FramebufferObject& fbo, const FramebufferAttatchment::Attatchment& a, const ImageInternalFormat::Format& i) : m_FBO(fbo){
    m_GL_Attatchment = a;
    m_InternalFormat = i;
}
epriv::FramebufferObjectAttatchment::FramebufferObjectAttatchment(const FramebufferObject& fbo, const FramebufferAttatchment::Attatchment& a, const Texture& t) : m_FBO(fbo) {
    m_GL_Attatchment = a;
    m_InternalFormat = t.internalFormat();
}
epriv::FramebufferObjectAttatchment::~FramebufferObjectAttatchment(){ 
}
const uint epriv::FramebufferObjectAttatchment::width() const {
    return m_FBO.width(); 
}
const uint epriv::FramebufferObjectAttatchment::height() const {
    return m_FBO.height(); 
}
uint epriv::FramebufferObjectAttatchment::attatchment(){ 
    return m_GL_Attatchment; 
}
void epriv::FramebufferObjectAttatchment::resize(FramebufferObject& fbo, const uint& w, const uint& h){
}
const GLuint epriv::FramebufferObjectAttatchment::address() const {
    return 0; 
}
void epriv::FramebufferObjectAttatchment::bind(){
}
void epriv::FramebufferObjectAttatchment::unbind(){
}
GLuint epriv::FramebufferObjectAttatchment::internalFormat(){ 
    return m_InternalFormat; 
}

#pragma endregion

epriv::FramebufferTexture::FramebufferTexture(const FramebufferObject& fbo,const FramebufferAttatchment::Attatchment& a,const Texture& t):FramebufferObjectAttatchment(fbo,a,t){
    m_Texture     = &const_cast<Texture&>(t);
    m_PixelFormat = t.pixelFormat();
    m_PixelType   = t.pixelType();
}
epriv::FramebufferTexture::~FramebufferTexture(){ 
    SAFE_DELETE(m_Texture);
}
void epriv::FramebufferTexture::resize(FramebufferObject& fbo, const uint& w, const uint& h){
    InternalTexturePublicInterface::Resize(*m_Texture, fbo, w, h);
}
const GLuint epriv::FramebufferTexture::address() const {
    return m_Texture->address(); 
}
Texture& epriv::FramebufferTexture::texture() {
    return *m_Texture;
}
void epriv::FramebufferTexture::bind(){
}
void epriv::FramebufferTexture::unbind(){
}

epriv::RenderbufferObject::RenderbufferObject(FramebufferObject& f,FramebufferAttatchment::Attatchment a,ImageInternalFormat::Format i):FramebufferObjectAttatchment(f,a,i){
    m_Width = m_Height = 0;
    glGenRenderbuffers(1, &m_RBO);
}
epriv::RenderbufferObject::~RenderbufferObject(){ 
    m_Width = m_Height = 0;
    glDeleteRenderbuffers(1, &m_RBO);
}
void epriv::RenderbufferObject::resize(FramebufferObject& fbo, const uint& w, const uint& h){
    Renderer::bindRBO(m_RBO);
    m_Width = w;  m_Height = h;
    glRenderbufferStorage(GL_RENDERBUFFER, attatchment(), m_Width, m_Height);
    Renderer::unbindRBO();
}
const GLuint epriv::RenderbufferObject::address() const {
    return m_RBO; 
}

void epriv::RenderbufferObject::bind(){ 
    Renderer::bindRBO(m_RBO); 
}
void epriv::RenderbufferObject::unbind(){ 
    Renderer::unbindRBO(); 
}

namespace Engine {
    namespace epriv {
        struct FramebufferObjectDefaultBindFunctor final {void operator()(BindableResource* r) const {
            FramebufferObject& fbo = *static_cast<FramebufferObject*>(r);
            Renderer::setViewport(0.0f, 0.0f, static_cast<float>(fbo.width()), static_cast<float>(fbo.height()));

            //swap buffers
            ++fbo.m_CurrentFBOIndex;
            if (fbo.m_CurrentFBOIndex >= fbo.m_FBO.size())
                fbo.m_CurrentFBOIndex = 0;

            Renderer::bindFBO(fbo);
            for (auto& attatchment : fbo.attatchments()) { 
                attatchment.second->bind(); 
            }
        }};
        struct FramebufferObjectDefaultUnbindFunctor final {void operator()(BindableResource* r) const {
            FramebufferObject& fbo = *static_cast<FramebufferObject*>(r);
            for (auto& attatchment : fbo.attatchments()) { 
                attatchment.second->unbind(); 
            }
            Renderer::unbindFBO();
            const auto winSize = Resources::getWindowSize();
            Renderer::setViewport(0.0f, 0.0f, static_cast<float>(winSize.x), static_cast<float>(winSize.y));
        }};
    };
};

epriv::FramebufferObjectDefaultBindFunctor   DEFAULT_BIND_FUNCTOR;
epriv::FramebufferObjectDefaultUnbindFunctor DEFAULT_UNBIND_FUNCTOR;

epriv::FramebufferObject::FramebufferObject(const string& name, const uint& w, const uint& h, const float& divisor, const uint& swapBufferCount):BindableResource(ResourceType::Empty, name){
    m_CurrentFBOIndex   = 0;
    m_Divisor           = divisor;
    m_FramebufferWidth  = static_cast<uint>(static_cast<float>(w) * m_Divisor);
    m_FramebufferHeight = static_cast<uint>(static_cast<float>(h) * m_Divisor);
    m_FBO.resize(swapBufferCount, GLuint(0));
    for (uint i = 0; i < m_FBO.size(); ++i)
        glGenFramebuffers(1, &m_FBO[i]);
    setCustomBindFunctor(DEFAULT_BIND_FUNCTOR);
    setCustomUnbindFunctor(DEFAULT_UNBIND_FUNCTOR);
}
epriv::FramebufferObject::FramebufferObject(const string& name, const uint& w, const uint& h, const ImageInternalFormat::Format& depthInternalFormat, const float& divisor, const uint& swapBufferCount):FramebufferObject(name,w,h,divisor,swapBufferCount){
    RenderbufferObject* rbo;
    if (depthInternalFormat == ImageInternalFormat::Depth24Stencil8 || depthInternalFormat == ImageInternalFormat::Depth32FStencil8)
        rbo = NEW RenderbufferObject(*this, FramebufferAttatchment::DepthAndStencil, depthInternalFormat);
    else if (depthInternalFormat == ImageInternalFormat::StencilIndex8)
        rbo = NEW RenderbufferObject(*this, FramebufferAttatchment::Stencil, depthInternalFormat);
    else
        rbo = NEW RenderbufferObject(*this, FramebufferAttatchment::Depth, depthInternalFormat);
    attatchRenderBuffer(*rbo);
}
epriv::FramebufferObject::~FramebufferObject(){ 
    SAFE_DELETE_MAP(m_Attatchments);
    for (uint i = 0; i < m_FBO.size(); ++i)
        glDeleteFramebuffers(1, &m_FBO[i]);
}
void epriv::FramebufferObject::resize(const uint& w, const uint& h){
    m_FramebufferWidth  = static_cast<uint>(static_cast<float>(w) * m_Divisor);
    m_FramebufferHeight = static_cast<uint>(static_cast<float>(h) * m_Divisor);
    Renderer::setViewport(0.0f, 0.0f, static_cast<float>(m_FramebufferWidth), static_cast<float>(m_FramebufferHeight));
    for (uint i = 0; i < m_FBO.size(); ++i) {
        Renderer::bindFBO(m_FBO[i]);
        for (auto& attatchment : m_Attatchments) {
            attatchment.second->resize(*this, w, h);
        }
    }
}
epriv::FramebufferTexture* epriv::FramebufferObject::attatchTexture(Texture* texture, const FramebufferAttatchment::Attatchment attatchment){
    if (m_Attatchments.count(attatchment))
        return nullptr;
    FramebufferTexture* framebufferTexture = NEW FramebufferTexture(*this, attatchment, *texture);
    for (uint i = 0; i < m_FBO.size(); ++i) {
        Renderer::bindFBO(m_FBO[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, framebufferTexture->attatchment(), framebufferTexture->m_Texture->type(), framebufferTexture->m_Texture->address(), 0);
    }
    m_Attatchments.emplace(attatchment, framebufferTexture);
    Renderer::unbindFBO();
    return framebufferTexture;
}
epriv::RenderbufferObject* epriv::FramebufferObject::attatchRenderBuffer(epriv::RenderbufferObject& rbo){ 
    if (m_Attatchments.count(rbo.attatchment()))
        return nullptr; 
    for (uint i = 0; i < m_FBO.size(); ++i) {
        Renderer::bindFBO(m_FBO[i]);
        Renderer::bindRBO(rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, rbo.internalFormat(), width(), height());
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, rbo.internalFormat(), GL_RENDERBUFFER, rbo.address());
        Renderer::unbindRBO();
    }
    m_Attatchments.emplace(rbo.attatchment(), &rbo);
    Renderer::unbindRBO();
    Renderer::unbindFBO();
    return &rbo;
}
const uint epriv::FramebufferObject::width() const {
    return m_FramebufferWidth; 
}
const uint epriv::FramebufferObject::height() const {
    return m_FramebufferHeight; 
}
const GLuint epriv::FramebufferObject::address() const { 
    return m_FBO[m_CurrentFBOIndex]; 
}
unordered_map<uint,epriv::FramebufferObjectAttatchment*>& epriv::FramebufferObject::attatchments(){ 
    return m_Attatchments; 
}
const bool epriv::FramebufferObject::check(){
    for (auto& fbo : m_FBO) {
        Renderer::bindFBO(fbo);
        GLenum err = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (err != GL_FRAMEBUFFER_COMPLETE) {
            cout << "Framebuffer completeness in FramebufferObject::impl _check() (index " + to_string(fbo) + ") is incomplete!" << endl; 
            cout << "Error is: " << err << endl;
            return false;
        }
    }
    return true;
}
const float epriv::FramebufferObject::divisor() const {
    return m_Divisor; 
}