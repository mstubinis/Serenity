
#include <serenity/renderer/GBuffer.h>
#include <serenity/renderer/Renderer.h>
#include <serenity/resources/texture/Texture.h>
#include <serenity/scene/Viewport.h>

#include <serenity/renderer/opengl/APIStateOpenGL.h>
#include <serenity/renderer/direct3d/APIStateDirectX.h>

namespace Engine::priv {
    class GBuffer::Impl {
        public:
            static void Start(GBuffer& gBuffer, std::vector<uint32_t>&& types, std::string_view channels, size_t fboIndex, float x, float y, float width, float height) {
                assert(!types.empty());
                if (types[0] != GBufferType::BackBuffer) {
                    gBuffer.m_FBOs[fboIndex].bind(x, y, width, height);
                    Engine::opengl::drawBuffers(types[0] != 0 ? types.size() : 0, types.data());
                    Engine::opengl::colorMask(channels.contains("R"), channels.contains("G"), channels.contains("B"), channels.contains("A"));
                } else {
                    gBuffer.bindBackbuffer(0, 0);
                }
            }
            static uint32_t GetAttachment(GBuffer& gBuffer, uint32_t buffer) noexcept {
                return buffer != GBufferType::BackBuffer ? gBuffer.m_FramebufferTextures[buffer]->attatchment() : GBufferType::BackBuffer;
            }
    };
}
namespace Engine::priv {
    GBuffer::GBuffer(uint32_t width, uint32_t height) 
        : m_Width{ width }
        , m_Height{ height }
    {
    }
    GBuffer::~GBuffer() {
        //Engine::opengl::unbindFBO();
    }
    Engine::priv::FramebufferObject& GBuffer::createFBO(float divisor, uint32_t swapBufferCount) {
        assert(m_Width > 0 && m_Height > 0);
        return m_FBOs.emplace_back(m_Width, m_Height, divisor, swapBufferCount);
    }
    Texture& GBuffer::createRenderTarget(size_t index, FramebufferObject& fbo, FramebufferAttatchment fbAttatch, ImageInternalFormat intFmt, ImagePixelFormat pxlFmt, ImagePixelType pxlType, std::string textureName) {
        assert(m_Width > 0 && m_Height > 0);
        m_FramebufferTextures[index] = fbo.attatchTexture(fbAttatch, m_Width, m_Height, pxlType, pxlFmt, intFmt, fbo.divisor());
        m_FramebufferTextures[index]->texture().setName(std::move(textureName));
        Engine::opengl::bindTexture(m_FramebufferTextures[index]->texture());
        return m_FramebufferTextures[index]->texture();
    }
    bool GBuffer::resize(uint32_t width, uint32_t height) {
        if (m_Width == width && m_Height == height) {
            return false;
        }
        m_Width = width;
        m_Height = height;

        for (auto& fbo : m_FBOs) {
            fbo.resize(width, height);
        }
        return true;
    }
    void GBuffer::bindFramebuffers(std::string_view channels, size_t fboIndex, float x, float y, float width, float height) {
        Impl::Start(*this, { 0 }, channels, fboIndex, x, y, width, height);
    }
    void GBuffer::bindFramebuffers(std::vector<uint32_t>&& buffers, std::string_view channels, size_t fboIndex, float x, float y, float width, float height) {
        for (auto& buffer : buffers) {
            buffer = Impl::GetAttachment(*this, buffer);
        }
        Impl::Start(*this, std::move(buffers), channels, fboIndex, x, y, width, height);
    }


    void GBuffer::bindBackbuffer(GLuint final_fbo, GLuint final_rbo) {
        Engine::opengl::bindFBO(final_fbo);
        Engine::opengl::bindRBO(final_rbo); //probably dont even need this. or only implement this if final_rbo != 0
        Engine::opengl::colorMask(true, true, true, true);
    }
    void GBuffer::bindBackbuffer(const Viewport& viewport, GLuint final_fbo, GLuint final_rbo) {
        GBuffer::bindBackbuffer(final_fbo, final_rbo);
        Engine::opengl::setViewport(viewport);
    }
}

