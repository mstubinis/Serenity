#include <core/engine/renderer/FramebufferObject.h>
#include <core/engine/renderer/Renderer.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/textures/Texture.h>

#include <iostream>

using namespace Engine;
using namespace std;

#pragma region FramebufferObjectAttatchmentBaseClass


priv::FramebufferObjectAttatchment::FramebufferObjectAttatchment(const FramebufferObject& fbo, const FramebufferAttatchment::Attatchment& a, const ImageInternalFormat::Format& i) : m_FBO(fbo){
    m_GL_Attatchment = a;
    m_InternalFormat = i;
}
priv::FramebufferObjectAttatchment::FramebufferObjectAttatchment(const FramebufferObject& fbo, const FramebufferAttatchment::Attatchment& a, const Texture& t) : m_FBO(fbo) {
    m_GL_Attatchment = a;
    m_InternalFormat = t.internalFormat();
}
priv::FramebufferObjectAttatchment::~FramebufferObjectAttatchment(){ 
}
unsigned int priv::FramebufferObjectAttatchment::width() const {
    return m_FBO.width(); 
}
unsigned int priv::FramebufferObjectAttatchment::height() const {
    return m_FBO.height(); 
}
unsigned int priv::FramebufferObjectAttatchment::attatchment() const {
    return m_GL_Attatchment; 
}
void priv::FramebufferObjectAttatchment::resize(FramebufferObject& fbo, const unsigned int& w, const unsigned int& h){
}
GLuint priv::FramebufferObjectAttatchment::address() const {
    return 0; 
}
void priv::FramebufferObjectAttatchment::bind(){
}
void priv::FramebufferObjectAttatchment::unbind(){
}
GLuint priv::FramebufferObjectAttatchment::internalFormat() const {
    return m_InternalFormat; 
}

#pragma endregion

priv::FramebufferTexture::FramebufferTexture(const FramebufferObject& fbo,const FramebufferAttatchment::Attatchment& a, const Texture& t) : FramebufferObjectAttatchment(fbo, a, t){
    m_Texture     = &const_cast<Texture&>(t);
    m_PixelFormat = t.pixelFormat();
    m_PixelType   = t.pixelType();
}
priv::FramebufferTexture::~FramebufferTexture(){ 
    SAFE_DELETE(m_Texture);
}
void priv::FramebufferTexture::resize(FramebufferObject& fbo, const unsigned int& w, const unsigned int& h){
    InternalTexturePublicInterface::Resize(*m_Texture, fbo, w, h);
}
GLuint priv::FramebufferTexture::address() const {
    return m_Texture->address(); 
}
Texture& priv::FramebufferTexture::texture() const {
    return *m_Texture;
}
void priv::FramebufferTexture::bind(){
}
void priv::FramebufferTexture::unbind(){
}

priv::RenderbufferObject::RenderbufferObject(FramebufferObject& f, FramebufferAttatchment::Attatchment a, ImageInternalFormat::Format i) : FramebufferObjectAttatchment(f,a,i) {
    m_Width = m_Height = 0;
    glGenRenderbuffers(1, &m_RBO);
}
priv::RenderbufferObject::~RenderbufferObject(){ 
    m_Width = m_Height = 0;
    glDeleteRenderbuffers(1, &m_RBO);
}
void priv::RenderbufferObject::resize(FramebufferObject& fbo, const unsigned int& w, const unsigned int& h){
    Engine::Renderer::bindRBO(m_RBO);
    m_Width = w; 
    m_Height = h;
    glRenderbufferStorage(GL_RENDERBUFFER, attatchment(), m_Width, m_Height);
    Engine::Renderer::unbindRBO();
}
GLuint priv::RenderbufferObject::address() const {
    return m_RBO; 
}

void priv::RenderbufferObject::bind(){ 
    Engine::Renderer::bindRBO(m_RBO);
}
void priv::RenderbufferObject::unbind(){ 
    Engine::Renderer::unbindRBO();
}

namespace Engine::priv {
    struct FramebufferObjectDefaultBindFunctor final {void operator()(const FramebufferObject* fbo_ptr) const {
        const FramebufferObject& fbo = *fbo_ptr;
        Engine::Renderer::setViewport(0.0f, 0.0f, static_cast<float>(fbo.width()), static_cast<float>(fbo.height()));

        //swap buffers
        ++fbo.m_CurrentFBOIndex;
        if (fbo.m_CurrentFBOIndex >= fbo.m_FBO.size()) {
            fbo.m_CurrentFBOIndex = 0;
        }

        Engine::Renderer::bindFBO(fbo);
        for (const auto& attatchment : fbo.attatchments()) { 
            attatchment.second->bind(); 
        }
    }};
    struct FramebufferObjectDefaultUnbindFunctor final {void operator()(const FramebufferObject* fbo_ptr) const {
        const FramebufferObject& fbo = *fbo_ptr;
        for (auto& attatchment : fbo.attatchments()) { 
            attatchment.second->unbind(); 
        }
        Engine::Renderer::unbindFBO();
        const auto winSize = Resources::getWindowSize();
        Engine::Renderer::setViewport(0.0f, 0.0f, static_cast<float>(winSize.x), static_cast<float>(winSize.y));
    }};
};

priv::FramebufferObjectDefaultBindFunctor   DEFAULT_BIND_FUNCTOR;
priv::FramebufferObjectDefaultUnbindFunctor DEFAULT_UNBIND_FUNCTOR;

priv::FramebufferObject::FramebufferObject() {
    m_FramebufferWidth  = 0;
    m_FramebufferHeight = 0;
    m_Divisor           = 1.0f;
}
priv::FramebufferObject::FramebufferObject(const unsigned int& w, const unsigned int& h, const float& divisor, const unsigned int& swapBufferCount) {
    init(w, h, divisor, swapBufferCount);
}
priv::FramebufferObject::FramebufferObject(const unsigned int& w, const unsigned int& h, const ImageInternalFormat::Format& depthInternalFormat, const float& divisor, const unsigned int& swapBufferCount) : FramebufferObject(w, h, divisor, swapBufferCount) {
    init(w, h, depthInternalFormat, divisor, swapBufferCount);
}
void priv::FramebufferObject::init(const unsigned int& width, const unsigned int& height, const float& divisor, const unsigned int& swapBufferCount) {
    m_CurrentFBOIndex   = 0;
    m_Divisor           = divisor;
    m_FramebufferWidth  = static_cast<unsigned int>(static_cast<float>(width) * m_Divisor);
    m_FramebufferHeight = static_cast<unsigned int>(static_cast<float>(height) * m_Divisor);
    m_FBO.resize(swapBufferCount, GLuint(0));
    for (unsigned int i = 0; i < m_FBO.size(); ++i) {
        glGenFramebuffers(1, &m_FBO[i]);
    }
    setCustomBindFunctor(DEFAULT_BIND_FUNCTOR);
    setCustomUnbindFunctor(DEFAULT_UNBIND_FUNCTOR);
}
void priv::FramebufferObject::init(const unsigned int& width, const unsigned int& height, const ImageInternalFormat::Format& depthInternalFormat, const float& divisor, const unsigned int& swapBufferCount) {
    RenderbufferObject* rbo;
    if (depthInternalFormat == ImageInternalFormat::Depth24Stencil8 || depthInternalFormat == ImageInternalFormat::Depth32FStencil8) {
        rbo = NEW RenderbufferObject(*this, FramebufferAttatchment::DepthAndStencil, depthInternalFormat);
    }else if (depthInternalFormat == ImageInternalFormat::StencilIndex8) {
        rbo = NEW RenderbufferObject(*this, FramebufferAttatchment::Stencil, depthInternalFormat);
    }else {
        rbo = NEW RenderbufferObject(*this, FramebufferAttatchment::Depth, depthInternalFormat);
    }
    attatchRenderBuffer(*rbo);
}
void priv::FramebufferObject::cleanup() {
    SAFE_DELETE_MAP(m_Attatchments);
    for (unsigned int i = 0; i < m_FBO.size(); ++i) {
        glDeleteFramebuffers(1, &m_FBO[i]);
    }
    m_Attatchments.clear();
}
priv::FramebufferObject::~FramebufferObject(){ 
    cleanup();
}
void priv::FramebufferObject::resize(const unsigned int& w, const unsigned int& h){
    m_FramebufferWidth  = static_cast<unsigned int>(static_cast<float>(w) * m_Divisor);
    m_FramebufferHeight = static_cast<unsigned int>(static_cast<float>(h) * m_Divisor);
    Engine::Renderer::setViewport(0.0f, 0.0f, static_cast<float>(m_FramebufferWidth), static_cast<float>(m_FramebufferHeight));
    for (unsigned int i = 0; i < m_FBO.size(); ++i) {
        Engine::Renderer::bindFBO(m_FBO[i]);
        for (auto& attatchment : m_Attatchments) {
            attatchment.second->resize(*this, w, h);
        }
    }
}
priv::FramebufferTexture* priv::FramebufferObject::attatchTexture(Texture* texture, const FramebufferAttatchment::Attatchment attatchment){
    if (m_Attatchments.count(attatchment)) {
        return nullptr;
    }
    FramebufferTexture* framebufferTexture = NEW FramebufferTexture(*this, attatchment, *texture);
    for (unsigned int i = 0; i < m_FBO.size(); ++i) {
        Engine::Renderer::bindFBO(m_FBO[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, framebufferTexture->attatchment(), framebufferTexture->m_Texture->type(), framebufferTexture->m_Texture->address(), 0);
    }
    m_Attatchments.emplace(attatchment, framebufferTexture);
    Engine::Renderer::unbindFBO();
    return framebufferTexture;
}
void priv::FramebufferObject::bind() const {
    m_CustomBindFunctor(this);
}
void priv::FramebufferObject::unbind() const {
    m_CustomUnbindFunctor(this);
}
priv::RenderbufferObject* priv::FramebufferObject::attatchRenderBuffer(priv::RenderbufferObject& rbo){ 
    if (m_Attatchments.count(rbo.attatchment())) {
        return nullptr;
    }
    for (unsigned int i = 0; i < m_FBO.size(); ++i) {
        Engine::Renderer::bindFBO(m_FBO[i]);
        Engine::Renderer::bindRBO(rbo);
        glRenderbufferStorage(GL_RENDERBUFFER, rbo.internalFormat(), width(), height());
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, rbo.internalFormat(), GL_RENDERBUFFER, rbo.address());
        Engine::Renderer::unbindRBO();
    }
    m_Attatchments.emplace(rbo.attatchment(), &rbo);
    Engine::Renderer::unbindRBO();
    Engine::Renderer::unbindFBO();
    return &rbo;
}
unsigned int priv::FramebufferObject::width() const {
    return m_FramebufferWidth; 
}
unsigned int priv::FramebufferObject::height() const {
    return m_FramebufferHeight; 
}
GLuint priv::FramebufferObject::address() const { 
    return m_FBO[m_CurrentFBOIndex]; 
}
unordered_map<unsigned int, priv::FramebufferObjectAttatchment*>& priv::FramebufferObject::attatchments() const {
    return m_Attatchments; 
}
bool priv::FramebufferObject::check(){
    for (const auto fboHandle : m_FBO) {
        Engine::Renderer::bindFBO(fboHandle);
        GLenum err = glCheckFramebufferStatus(GL_FRAMEBUFFER);
        if (err != GL_FRAMEBUFFER_COMPLETE) {
            cout << "Framebuffer completeness in FramebufferObject::impl _check() (index " + to_string(fboHandle) + ") is incomplete!\n";
            cout << "Error is: " << err << "\n";
            return false;
        }
    }
    return true;
}
float priv::FramebufferObject::divisor() const {
    return m_Divisor; 
}