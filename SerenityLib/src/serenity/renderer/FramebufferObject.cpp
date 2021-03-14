
#include <serenity/renderer/FramebufferObject.h>
#include <serenity/resources/Engine_Resources.h>

using namespace Engine::priv;

#pragma region FramebufferObjectAttatchmentBaseClass

FramebufferObjectAttatchment::FramebufferObjectAttatchment(const FramebufferObject& fbo, FramebufferAttatchment a, ImageInternalFormat i) 
    : m_FBO{ fbo }
    , m_GL_Attatchment{ (GLuint)a }
    , m_InternalFormat{ (GLuint)i }
{}
FramebufferObjectAttatchment::FramebufferObjectAttatchment(const FramebufferObject& fbo, FramebufferAttatchment a, const Texture& t) 
    : FramebufferObjectAttatchment{ fbo, a, t.internalFormat() }
{}
uint32_t FramebufferObjectAttatchment::width() const {
    return m_FBO.width(); 
}
uint32_t FramebufferObjectAttatchment::height() const {
    return m_FBO.height(); 
}

#pragma endregion

#pragma region FrameBufferTexture

FramebufferTexture::FramebufferTexture(const FramebufferObject& fbo, FramebufferAttatchment a, Texture* t) 
    : FramebufferObjectAttatchment{ fbo, a, *t }
    , m_PixelFormat{ (GLuint)t->pixelFormat() }
    , m_PixelType{ (GLuint)t->pixelType() }
{
    m_Texture = std::unique_ptr<Texture>(t);
}
void FramebufferTexture::resize(FramebufferObject& fbo, uint32_t width, uint32_t height){
    TextureLoader::Resize(*m_Texture, fbo, width, height);
}
GLuint FramebufferTexture::address() const {
    return m_Texture->address(); 
}

#pragma endregion

#pragma region RenderbufferObject

RenderbufferObject::RenderbufferObject(FramebufferObject& f, FramebufferAttatchment a, ImageInternalFormat i) 
    : FramebufferObjectAttatchment{ f, a, i }
{
    GLCall(glGenRenderbuffers(1, &m_RBO));
}
RenderbufferObject::~RenderbufferObject(){ 
    GLCall(glDeleteRenderbuffers(1, &m_RBO));
}
void RenderbufferObject::resize(FramebufferObject& fbo, uint32_t width, uint32_t height){
    Engine::Renderer::bindRBO(m_RBO);
    m_Width  = width; 
    m_Height = height;
    GLCall(glRenderbufferStorage(GL_RENDERBUFFER, attatchment(), m_Width, m_Height));
    Engine::Renderer::unbindRBO();
}

void RenderbufferObject::bind(){ 
    Engine::Renderer::bindRBO(m_RBO);
}
void RenderbufferObject::unbind(){ 
    Engine::Renderer::unbindRBO();
}

#pragma endregion

namespace Engine::priv {
    struct FramebufferObjectDefaultBindFunctor final { void operator()(const FramebufferObject* fbo) const {
        Engine::Renderer::setViewport(0.0f, 0.0f, (float)fbo->width(), (float)fbo->height());
        fbo->m_CurrentFBOIndex = (fbo->m_CurrentFBOIndex + 1) % fbo->m_FBOs.size(); //swap buffers
        Engine::Renderer::bindFBO(*fbo);
        for (const auto& [idx, attatchment] : fbo->attatchments()) {
            attatchment->bind(); 
        }
    }};
    struct FramebufferObjectDefaultUnbindFunctor final { void operator()(const FramebufferObject* fbo) const {
        for (const auto& [idx, attatchment] : fbo->attatchments()) {
            attatchment->unbind(); 
        }
        Engine::Renderer::unbindFBO();
        const auto winSize = Resources::getWindowSize();
        Engine::Renderer::setViewport(0.0f, 0.0f, (float)winSize.x, (float)winSize.y);
    }};
};

#pragma region FramebufferObject

FramebufferObject::FramebufferObject(uint32_t w, uint32_t h, float divisor, uint32_t swapBufferCount) {
    init(w, h, divisor, swapBufferCount);
}
FramebufferObject::FramebufferObject(uint32_t w, uint32_t h, ImageInternalFormat depthInternalFormat, float divisor, uint32_t swapBufferCount)
    : FramebufferObject{ w, h, divisor, swapBufferCount }
{
    init(w, h, depthInternalFormat, divisor, swapBufferCount);
}
FramebufferObject::~FramebufferObject() {
    cleanup();
}
void FramebufferObject::init(uint32_t width, uint32_t height, float divisor, uint32_t swapBufferCount) {
    m_CurrentFBOIndex   = 0;
    m_Divisor           = divisor;
    m_FramebufferWidth  = (uint32_t)((float)width * m_Divisor);
    m_FramebufferHeight = (uint32_t)((float)height * m_Divisor);
    m_FBOs.resize(swapBufferCount, GLuint(0));
    for (auto& fbo : m_FBOs) {
        GLCall(glGenFramebuffers(1, &fbo));
    }
    setCustomBindFunctor(FramebufferObjectDefaultBindFunctor());
    setCustomUnbindFunctor(FramebufferObjectDefaultUnbindFunctor());
}
void FramebufferObject::init(uint32_t width, uint32_t height, ImageInternalFormat depthInternalFormat, float divisor, uint32_t swapBufferCount) {
    if (depthInternalFormat == ImageInternalFormat::Depth24Stencil8 || depthInternalFormat == ImageInternalFormat::Depth32FStencil8) {
        attatchRenderBuffer(*this, FramebufferAttatchment::DepthAndStencil, depthInternalFormat);
    }else if (depthInternalFormat == ImageInternalFormat::StencilIndex8) {
        attatchRenderBuffer(*this, FramebufferAttatchment::Stencil, depthInternalFormat);
    }else {
        attatchRenderBuffer(*this, FramebufferAttatchment::Depth, depthInternalFormat);
    }
}
void FramebufferObject::cleanup() {
    for (size_t i = 0; i < m_FBOs.size(); ++i) {
        GLCall(glDeleteFramebuffers(1, &m_FBOs[i]));
    }
}
void FramebufferObject::resize(uint32_t w, uint32_t h){
    m_FramebufferWidth  = (uint32_t)((float)w * m_Divisor);
    m_FramebufferHeight = (uint32_t)((float)h * m_Divisor);
    Engine::Renderer::setViewport(0.0f, 0.0f, (float)m_FramebufferWidth, (float)m_FramebufferHeight);
    for (const auto fbo : m_FBOs) {
        Engine::Renderer::bindFBO(fbo);
        for (auto& attatchment : m_Attatchments) {
            attatchment.second->resize(*this, w, h);
        }
    }
}
bool FramebufferObject::checkStatus() {
    for (const auto fbo : m_FBOs) {
        Engine::Renderer::bindFBO(fbo);
        GLCall(GLenum framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER));
        if (framebufferStatus != GL_FRAMEBUFFER_COMPLETE) {
            ENGINE_PRODUCTION_LOG("Framebuffer completeness in FramebufferObject::impl _check() (index " + std::to_string(fbo) + ") is incomplete!")
            ENGINE_PRODUCTION_LOG("Error is: " << framebufferStatus)
            return false;
        }
    }
    return true;
}

#pragma endregion