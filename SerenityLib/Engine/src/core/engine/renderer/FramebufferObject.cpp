#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/renderer/FramebufferObject.h>
#include <core/engine/renderer/Renderer.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/textures/Texture.h>

using namespace Engine;

#pragma region FramebufferObjectAttatchmentBaseClass

priv::FramebufferObjectAttatchment::FramebufferObjectAttatchment(const FramebufferObject& fbo, FramebufferAttatchment a, ImageInternalFormat i) 
    : m_FBO{ fbo }
    , m_GL_Attatchment{ (GLuint)a }
    , m_InternalFormat{ (GLuint)i }
{}
priv::FramebufferObjectAttatchment::FramebufferObjectAttatchment(const FramebufferObject& fbo, FramebufferAttatchment a, const Texture& t) 
    : FramebufferObjectAttatchment{ fbo, a, t.internalFormat() }
{}
unsigned int priv::FramebufferObjectAttatchment::width() const {
    return m_FBO.width(); 
}
unsigned int priv::FramebufferObjectAttatchment::height() const {
    return m_FBO.height(); 
}

#pragma endregion

priv::FramebufferTexture::FramebufferTexture(const FramebufferObject& fbo, FramebufferAttatchment a, const Texture& t) 
    : FramebufferObjectAttatchment{ fbo, a, t }
    , m_Texture{ &const_cast<Texture&>(t) }
    , m_PixelFormat{ (GLuint)t.pixelFormat() }
    , m_PixelType{ (GLuint)t.pixelType() }
{}
priv::FramebufferTexture::~FramebufferTexture(){ 
    SAFE_DELETE(m_Texture);
}
void priv::FramebufferTexture::resize(FramebufferObject& fbo, unsigned int w, unsigned int h){
    TextureLoader::Resize(*m_Texture, fbo, w, h);
}
GLuint priv::FramebufferTexture::address() const {
    return m_Texture->address(); 
}
Texture& priv::FramebufferTexture::texture() const {
    return *m_Texture;
}

priv::RenderbufferObject::RenderbufferObject(FramebufferObject& f, FramebufferAttatchment a, ImageInternalFormat i) 
    : FramebufferObjectAttatchment{ f,a,i }
{
    GLCall(glGenRenderbuffers(1, &m_RBO));
}
priv::RenderbufferObject::~RenderbufferObject(){ 
    GLCall(glDeleteRenderbuffers(1, &m_RBO));
}
void priv::RenderbufferObject::resize(FramebufferObject& fbo, unsigned int w, unsigned int h){
    Engine::Renderer::bindRBO(m_RBO);
    m_Width  = w; 
    m_Height = h;
    GLCall(glRenderbufferStorage(GL_RENDERBUFFER, attatchment(), m_Width, m_Height));
    Engine::Renderer::unbindRBO();
}

void priv::RenderbufferObject::bind(){ 
    Engine::Renderer::bindRBO(m_RBO);
}
void priv::RenderbufferObject::unbind(){ 
    Engine::Renderer::unbindRBO();
}

namespace Engine::priv {
    struct FramebufferObjectDefaultBindFunctor final { void operator()(const FramebufferObject* fbo_ptr) const {
        Engine::Renderer::setViewport(0.0f, 0.0f, (float)fbo_ptr->width(), (float)fbo_ptr->height());

        //swap buffers
        ++fbo_ptr->m_CurrentFBOIndex;
        if (fbo_ptr->m_CurrentFBOIndex >= fbo_ptr->m_FBO.size()) {
            fbo_ptr->m_CurrentFBOIndex = 0;
        }

        Engine::Renderer::bindFBO(*fbo_ptr);
        for (const auto& attatchment : fbo_ptr->attatchments()) {
            attatchment.second->bind(); 
        }
    }};
    struct FramebufferObjectDefaultUnbindFunctor final { void operator()(const FramebufferObject* fbo_ptr) const {
        for (auto& attatchment : fbo_ptr->attatchments()) {
            attatchment.second->unbind(); 
        }
        Engine::Renderer::unbindFBO();
        const auto winSize = Resources::getWindowSize();
        Engine::Renderer::setViewport(0.0f, 0.0f, (float)winSize.x, (float)winSize.y);
    }};
};

priv::FramebufferObject::FramebufferObject(unsigned int w, unsigned int h, float divisor, unsigned int swapBufferCount) {
    init(w, h, divisor, swapBufferCount);
}
priv::FramebufferObject::FramebufferObject(unsigned int w, unsigned int h, ImageInternalFormat depthInternalFormat, float divisor, unsigned int swapBufferCount) 
    : FramebufferObject{ w, h, divisor, swapBufferCount }
{
    init(w, h, depthInternalFormat, divisor, swapBufferCount);
}
priv::FramebufferObject::~FramebufferObject() {
    cleanup();
}
void priv::FramebufferObject::init(unsigned int width, unsigned int height, float divisor, unsigned int swapBufferCount) {
    m_CurrentFBOIndex   = 0;
    m_Divisor           = divisor;
    m_FramebufferWidth  = (unsigned int)((float)width * m_Divisor);
    m_FramebufferHeight = (unsigned int)((float)height * m_Divisor);
    m_FBO.resize(swapBufferCount, GLuint(0));
    for (size_t i = 0; i < m_FBO.size(); ++i) {
        GLCall(glGenFramebuffers(1, &m_FBO[i]));
    }
    setCustomBindFunctor(FramebufferObjectDefaultBindFunctor());
    setCustomUnbindFunctor(FramebufferObjectDefaultUnbindFunctor());
}
void priv::FramebufferObject::init(unsigned int width, unsigned int height, ImageInternalFormat depthInternalFormat, float divisor, unsigned int swapBufferCount) {
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
    for (size_t i = 0; i < m_FBO.size(); ++i) {
        GLCall(glDeleteFramebuffers(1, &m_FBO[i]));
    }
}
void priv::FramebufferObject::resize(unsigned int w, unsigned int h){
    m_FramebufferWidth  = (unsigned int)((float)w * m_Divisor);
    m_FramebufferHeight = (unsigned int)((float)h * m_Divisor);
    Engine::Renderer::setViewport(0.0f, 0.0f, (float)m_FramebufferWidth, (float)m_FramebufferHeight);
    for (size_t i = 0; i < m_FBO.size(); ++i) {
        Engine::Renderer::bindFBO(m_FBO[i]);
        for (auto& attatchment : m_Attatchments) {
            attatchment.second->resize(*this, w, h);
        }
    }
}
priv::FramebufferTexture* priv::FramebufferObject::attatchTexture(Texture* texture, FramebufferAttatchment attatchment){
    if (m_Attatchments.count((unsigned int)attatchment)) {
        return nullptr;
    }
    FramebufferTexture* framebufferTexture = NEW FramebufferTexture(*this, attatchment, *texture);
    for (size_t i = 0; i < m_FBO.size(); ++i) {
        Engine::Renderer::bindFBO(m_FBO[i]);
        GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, framebufferTexture->attatchment(), framebufferTexture->m_Texture->getTextureType().toGLType(), framebufferTexture->m_Texture->address(), 0));
    }
    m_Attatchments.emplace((unsigned int)attatchment, framebufferTexture);
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
    for (size_t i = 0; i < m_FBO.size(); ++i) {
        Engine::Renderer::bindFBO(m_FBO[i]);
        Engine::Renderer::bindRBO(rbo);
        GLCall(glRenderbufferStorage(GL_RENDERBUFFER, rbo.internalFormat(), width(), height()));
        GLCall(glFramebufferRenderbuffer(GL_FRAMEBUFFER, rbo.internalFormat(), GL_RENDERBUFFER, rbo.address()));
        Engine::Renderer::unbindRBO();
    }
    m_Attatchments.emplace(rbo.attatchment(), &rbo);
    Engine::Renderer::unbindRBO();
    Engine::Renderer::unbindFBO();
    return &rbo;
}
GLuint priv::FramebufferObject::address() const { 
    return m_FBO[m_CurrentFBOIndex]; 
}
bool priv::FramebufferObject::check(){
    for (const auto fboHandle : m_FBO) {
        Engine::Renderer::bindFBO(fboHandle);
        GLCall(GLenum err = glCheckFramebufferStatus(GL_FRAMEBUFFER));
        if (err != GL_FRAMEBUFFER_COMPLETE) {
            ENGINE_PRODUCTION_LOG("Framebuffer completeness in FramebufferObject::impl _check() (index " + std::to_string(fboHandle) + ") is incomplete!")
            ENGINE_PRODUCTION_LOG("Error is: " << err)
            return false;
        }
    }
    return true;
}