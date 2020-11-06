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
uint32_t priv::FramebufferObjectAttatchment::width() const {
    return m_FBO.width(); 
}
uint32_t priv::FramebufferObjectAttatchment::height() const {
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
void priv::FramebufferTexture::resize(FramebufferObject& fbo, uint32_t width, uint32_t height){
    TextureLoader::Resize(*m_Texture, fbo, width, height);
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
void priv::RenderbufferObject::resize(FramebufferObject& fbo, uint32_t width, uint32_t height){
    Engine::Renderer::bindRBO(m_RBO);
    m_Width  = width; 
    m_Height = height;
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
        fbo_ptr->m_CurrentFBOIndex = (fbo_ptr->m_CurrentFBOIndex + 1) % fbo_ptr->m_FBO.size(); //swap buffers
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

priv::FramebufferObject::FramebufferObject(uint32_t w, uint32_t h, float divisor, uint32_t swapBufferCount) {
    init(w, h, divisor, swapBufferCount);
}
priv::FramebufferObject::FramebufferObject(uint32_t w, uint32_t h, ImageInternalFormat depthInternalFormat, float divisor, uint32_t swapBufferCount)
    : FramebufferObject{ w, h, divisor, swapBufferCount }
{
    init(w, h, depthInternalFormat, divisor, swapBufferCount);
}
priv::FramebufferObject::~FramebufferObject() {
    cleanup();
}
void priv::FramebufferObject::init(uint32_t width, uint32_t height, float divisor, uint32_t swapBufferCount) {
    m_CurrentFBOIndex   = 0;
    m_Divisor           = divisor;
    m_FramebufferWidth  = (uint32_t)((float)width * m_Divisor);
    m_FramebufferHeight = (uint32_t)((float)height * m_Divisor);
    m_FBO.resize(swapBufferCount, GLuint(0));
    for (size_t i = 0; i < m_FBO.size(); ++i) {
        GLCall(glGenFramebuffers(1, &m_FBO[i]));
    }
    setCustomBindFunctor(FramebufferObjectDefaultBindFunctor());
    setCustomUnbindFunctor(FramebufferObjectDefaultUnbindFunctor());
}
void priv::FramebufferObject::init(uint32_t width, uint32_t height, ImageInternalFormat depthInternalFormat, float divisor, uint32_t swapBufferCount) {
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
void priv::FramebufferObject::resize(uint32_t w, uint32_t h){
    m_FramebufferWidth  = (uint32_t)((float)w * m_Divisor);
    m_FramebufferHeight = (uint32_t)((float)h * m_Divisor);
    Engine::Renderer::setViewport(0.0f, 0.0f, (float)m_FramebufferWidth, (float)m_FramebufferHeight);
    for (size_t i = 0; i < m_FBO.size(); ++i) {
        Engine::Renderer::bindFBO(m_FBO[i]);
        for (auto& attatchment : m_Attatchments) {
            attatchment.second->resize(*this, w, h);
        }
    }
}
priv::FramebufferTexture* priv::FramebufferObject::attatchTexture(Texture* texture, FramebufferAttatchment attatchment){
    if (m_Attatchments.contains((uint32_t)attatchment)) {
        return nullptr;
    }
    FramebufferTexture* framebufferTexture = NEW FramebufferTexture(*this, attatchment, *texture);
    for (size_t i = 0; i < m_FBO.size(); ++i) {
        Engine::Renderer::bindFBO(m_FBO[i]);
        GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, framebufferTexture->attatchment(), framebufferTexture->m_Texture->getTextureType().toGLType(), framebufferTexture->m_Texture->address(), 0));
    }
    m_Attatchments.emplace(
        std::piecewise_construct,
        std::forward_as_tuple((uint32_t)attatchment),
        std::forward_as_tuple(framebufferTexture)
    );
    Engine::Renderer::unbindFBO();
    return framebufferTexture;
}
priv::RenderbufferObject* priv::FramebufferObject::attatchRenderBuffer(priv::RenderbufferObject& rbo){ 
    if (m_Attatchments.contains(rbo.attatchment())) {
        return nullptr;
    }
    for (size_t i = 0; i < m_FBO.size(); ++i) {
        Engine::Renderer::bindFBO(m_FBO[i]);
        Engine::Renderer::bindRBO(rbo);
        GLCall(glRenderbufferStorage(GL_RENDERBUFFER, rbo.internalFormat(), width(), height()));
        GLCall(glFramebufferRenderbuffer(GL_FRAMEBUFFER, rbo.internalFormat(), GL_RENDERBUFFER, rbo.address()));
        Engine::Renderer::unbindRBO();
    }
    m_Attatchments.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(rbo.attatchment()),
        std::forward_as_tuple(&rbo)
    );
    Engine::Renderer::unbindRBO();
    Engine::Renderer::unbindFBO();
    return &rbo;
}
bool priv::FramebufferObject::checkStatus() {
    for (const auto fboHandle : m_FBO) {
        Engine::Renderer::bindFBO(fboHandle);
        GLCall(GLenum framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER));
        if (framebufferStatus != GL_FRAMEBUFFER_COMPLETE) {
            ENGINE_PRODUCTION_LOG("Framebuffer completeness in FramebufferObject::impl _check() (index " + std::to_string(fboHandle) + ") is incomplete!")
            ENGINE_PRODUCTION_LOG("Error is: " << framebufferStatus)
            return false;
        }
    }
    return true;
}