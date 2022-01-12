
#include <serenity/renderer/FramebufferObject.h>
#include <serenity/resources/Engine_Resources.h>

using namespace Engine::priv;

namespace {
    void swap_buffers(size_t& fboCurrentIndex, size_t fboSize) {
        fboCurrentIndex = (fboCurrentIndex + 1) % fboSize; //swap buffers
    }
    constexpr const char* debugFramebufferStatusAsStr(const GLenum fbStatus) {
        switch (fbStatus) {
            case GL_FRAMEBUFFER_UNDEFINED: {
                return "UNDEFINED";
            } case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: {
                return "INCOMPLETE_ATTACHMENT";
            } case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: {
                return "INCOMPLETE_MISSING_ATTACHMENT";
            } case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: {
                return "INCOMPLETE_DRAW_BUFFER";
            } case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: {
                return "INCOMPLETE_READ_BUFFER";
            } case GL_FRAMEBUFFER_UNSUPPORTED: {
                return "UNSUPPORTED";
            } case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: {
                return "INCOMPLETE_MULTISAMPLE";
            } case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS: {
                return "INCOMPLETE_LAYER_TARGETS";
            }
        }
        return "";
    }
};

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
    glGenRenderbuffers(1, &m_RBO);
}
RenderbufferObject::~RenderbufferObject(){ 
    glDeleteRenderbuffers(1, &m_RBO);
}
void RenderbufferObject::resize(FramebufferObject& fbo, uint32_t width, uint32_t height){
    Engine::Renderer::bindRBO(m_RBO);
    m_Width  = width; 
    m_Height = height;
    glRenderbufferStorage(GL_RENDERBUFFER, attatchment(), m_Width, m_Height);
    Engine::Renderer::unbindRBO();
}

void RenderbufferObject::bind(){ 
    Engine::Renderer::bindRBO(m_RBO);
}
void RenderbufferObject::unbind(){ 
    Engine::Renderer::unbindRBO();
}

#pragma endregion

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
    m_FramebufferWidth  = uint32_t(float(width) * m_Divisor);
    m_FramebufferHeight = uint32_t(float(height) * m_Divisor);
    m_FBOs.resize(swapBufferCount, 0);
    for (GLuint& fbo : m_FBOs) {
        glGenFramebuffers(1, &fbo);
    }
}
void FramebufferObject::init(uint32_t width, uint32_t height, ImageInternalFormat depthInternalFormat, float divisor, uint32_t swapBufferCount) {
    if (depthInternalFormat == ImageInternalFormat::Depth24Stencil8 || depthInternalFormat == ImageInternalFormat::Depth32FStencil8) {
        attatchRenderBuffer(*this, FramebufferAttatchment::DepthAndStencil, depthInternalFormat);
    } else if (depthInternalFormat == ImageInternalFormat::StencilIndex8) {
        attatchRenderBuffer(*this, FramebufferAttatchment::Stencil, depthInternalFormat);
    } else {
        attatchRenderBuffer(*this, FramebufferAttatchment::Depth, depthInternalFormat);
    }
}
void FramebufferObject::cleanup() {
    for (size_t i = 0; i < m_FBOs.size(); ++i) {
        glDeleteFramebuffers(1, &m_FBOs[i]);
    }
    unbind();
    m_Attatchments.clear();
}
void FramebufferObject::resize(const uint32_t width, const uint32_t height) {
    m_FramebufferWidth  = uint32_t(float(width) * m_Divisor);
    m_FramebufferHeight = uint32_t(float(height) * m_Divisor);
    Engine::Renderer::setViewport(0.0f, 0.0f, m_FramebufferWidth, m_FramebufferHeight);
    for (const GLuint fbo : m_FBOs) {
        Engine::Renderer::bindFBO(fbo);
        for (auto& attatchment : m_Attatchments) {
            attatchment.second->resize(*this, width, height);
        }
    }
}
void FramebufferObject::bind(float x, float y, float width, float height) {
    Engine::Renderer::setViewport(x, y, width <= 0.0f ? m_FramebufferWidth : width, height <= 0.0f ? m_FramebufferHeight : height);
    swap_buffers(m_CurrentFBOIndex, m_FBOs.size());
    Engine::Renderer::bindFBO(*this);
    for (const auto& [idx, attatchment] : m_Attatchments) {
        attatchment->bind();
    }
}
void FramebufferObject::unbind() {
    for (const auto& [idx, attatchment] : m_Attatchments) {
        attatchment->unbind();
    }
    Engine::Renderer::unbindFBO();
}
bool FramebufferObject::checkStatus() {
    for (const GLuint fbo : m_FBOs) {
        Engine::Renderer::bindFBO(fbo);
        #if defined(_DEBUG)
            const GLenum framebufferStatus = glCheckFramebufferStatus(GL_FRAMEBUFFER);
            if (framebufferStatus != GL_FRAMEBUFFER_COMPLETE) {
                ENGINE_PRODUCTION_LOG(__FUNCTION__ << "() error - fbo: " << fbo << ": " << debugFramebufferStatusAsStr(framebufferStatus))
                return false;
            }
        #endif
    }
    return true;
}

#pragma endregion