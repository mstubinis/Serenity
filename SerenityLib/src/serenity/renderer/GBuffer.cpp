
#include <serenity/renderer/GBuffer.h>
#include <serenity/renderer/Renderer.h>
#include <serenity/resources/texture/Texture.h>
#include <serenity/scene/Viewport.h>

constexpr std::array<std::tuple<ImageInternalFormat, ImagePixelFormat, ImagePixelType, FramebufferAttatchment>, Engine::priv::GBufferType::_TOTAL> GBUFFER_TYPE_DATA{{
/*Diffuse*/ {ImageInternalFormat::RGB8,    ImagePixelFormat::RGB,  ImagePixelType::UNSIGNED_BYTE, FramebufferAttatchment::Color_0},
/*Normal*/  {ImageInternalFormat::RGBA16F, ImagePixelFormat::RGBA, ImagePixelType::FLOAT,         FramebufferAttatchment::Color_1},//r,g = NormalsOctahedronCompressed, b = MaterialID & AO, a = Packed Metalness / Smoothness 
/*Misc*/    {ImageInternalFormat::RGBA8,   ImagePixelFormat::RGBA, ImagePixelType::UNSIGNED_BYTE, FramebufferAttatchment::Color_2},//r = Glow, g = Specular, b = GodRaysRG (nibbles), a = GodRaysB 
/*Lighting*/{ImageInternalFormat::RGB16F,  ImagePixelFormat::RGB,  ImagePixelType::FLOAT,         FramebufferAttatchment::Color_3},
/*Bloom*/   {ImageInternalFormat::RGBA4,   ImagePixelFormat::RGBA, ImagePixelType::UNSIGNED_BYTE, FramebufferAttatchment::Color_0}, //rgb = bloom, a == ssao blur
/*GodRays*/ {ImageInternalFormat::RGBA4,   ImagePixelFormat::RGBA, ImagePixelType::UNSIGNED_BYTE, FramebufferAttatchment::Color_1}, //rgb = rays, a == ssao blur
/*Depth*/   {ImageInternalFormat::Depth24Stencil8, ImagePixelFormat::DEPTH_STENCIL, ImagePixelType::UNSIGNED_INT_24_8, FramebufferAttatchment::DepthAndStencil},
}};

void Engine::priv::GBuffer::init(uint32_t width, uint32_t height){
    internal_Destruct(); //just incase this method is called on resize, we want to delete any previous buffers

    m_Width  = width;
    m_Height = height;

    m_FBO.cleanup();
    m_SmallFBO.cleanup();

    m_FBO.init(m_Width, m_Height, 1.0f, 2);
    internal_Build_Texture_Buffer(m_FBO, GBufferType::Diffuse,  m_Width, m_Height);
    internal_Build_Texture_Buffer(m_FBO, GBufferType::Normal,   m_Width, m_Height);
    internal_Build_Texture_Buffer(m_FBO, GBufferType::Misc,     m_Width, m_Height);
    internal_Build_Texture_Buffer(m_FBO, GBufferType::Lighting, m_Width, m_Height);
    internal_Build_Texture_Buffer(m_FBO, GBufferType::Depth,    m_Width, m_Height);

    if (!m_FBO.checkStatus()) {
        return;
    }
    m_SmallFBO.init(m_Width, m_Height, 0.5f, 2);
    internal_Build_Texture_Buffer(m_SmallFBO, GBufferType::Bloom,   m_Width, m_Height);
    internal_Build_Texture_Buffer(m_SmallFBO, GBufferType::GodRays, m_Width, m_Height);

    if (!m_SmallFBO.checkStatus()) {
        return;
    }
    //this should be better performance wise, but clean up this code a bit
    auto& depthTexture = m_FramebufferTextures[GBufferType::Depth]->texture();
    Engine::Renderer::bindTextureForModification(depthTexture.getTextureType(), depthTexture.address());
    depthTexture.setFilter(TextureFilter::Nearest);

    auto& diffuseTexture = m_FramebufferTextures[GBufferType::Diffuse]->texture();
    Engine::Renderer::bindTextureForModification(diffuseTexture.getTextureType(), diffuseTexture.address());
    diffuseTexture.setFilter(TextureFilter::Nearest);

    auto& normalTexture = m_FramebufferTextures[GBufferType::Normal]->texture();
    Engine::Renderer::bindTextureForModification(normalTexture.getTextureType(), normalTexture.address());
    normalTexture.setFilter(TextureFilter::Nearest);

    auto& godRaysTexture = m_FramebufferTextures[GBufferType::GodRays]->texture();
    Engine::Renderer::bindTextureForModification(godRaysTexture.getTextureType(), godRaysTexture.address());
    godRaysTexture.setFilter(TextureFilter::Nearest);
}
void Engine::priv::GBuffer::internal_Destruct() {
    m_Width  = 0; 
    m_Height = 0;
    Engine::Renderer::unbindFBO();
    m_FramebufferTextures.fill(nullptr);
}
Engine::priv::GBuffer::~GBuffer() {
    internal_Destruct();
}
bool Engine::priv::GBuffer::resize(uint32_t width, uint32_t height) {
    if (m_Width == width && m_Height == height) {
        return false;
    }
    m_Width  = width;
    m_Height = height;
    m_FBO.resize(width, height);
    m_SmallFBO.resize(width, height);
    return true;
}
void Engine::priv::GBuffer::internal_Build_Texture_Buffer(FramebufferObject& fbo, GBufferType::Type gbufferType, uint32_t w, uint32_t h) {
    const auto [internalFmt, pxlFmt, pxlType, fbAttatch] = GBUFFER_TYPE_DATA[gbufferType];
    m_FramebufferTextures[gbufferType] = fbo.attatchTexture(fbAttatch, w, h, pxlType, pxlFmt, internalFmt, fbo.divisor());
}
uint32_t Engine::priv::GBuffer::internal_get_attatchment(uint32_t buffer) noexcept {
    return buffer != GBufferType::BackBuffer ? m_FramebufferTextures[buffer]->attatchment() : GBufferType::BackBuffer;
}
void Engine::priv::GBuffer::internal_Start(const std::vector<uint32_t>& types, std::string_view channels, bool first_fbo) {
    ASSERT(types.size() > 0, __FUNCTION__ << "(): types was empty!");
    if (types[0] != GBufferType::BackBuffer) {
        first_fbo ? m_FBO.bind() : m_SmallFBO.bind();
        const bool r = channels.find("R") != std::string::npos;
        const bool g = channels.find("G") != std::string::npos;
        const bool b = channels.find("B") != std::string::npos;
        const bool a = channels.find("A") != std::string::npos;

        glDrawBuffers(types[0] != 0 ? GLsizei(types.size()) : GLsizei(0), types.data());
        Engine::Renderer::colorMask(r, g, b, a);
    } else {
        GBuffer::bindBackbuffer(0, 0);
    }
}
void Engine::priv::GBuffer::bindFramebuffers(std::string_view channels, bool mainFBO) {
    internal_Start({ 0 }, channels, mainFBO);
}
void Engine::priv::GBuffer::bindFramebuffers(uint32_t buffer, std::string_view channels, bool mainFBO) {
    internal_Start({ internal_get_attatchment(buffer) }, channels, mainFBO);
}
void Engine::priv::GBuffer::bindFramebuffers(uint32_t t1, uint32_t t2, std::string_view channels, bool mainFBO) {
    internal_Start({
        internal_get_attatchment(t1),
        internal_get_attatchment(t2)
    }, channels, mainFBO);
}
void Engine::priv::GBuffer::bindFramebuffers(uint32_t t1, uint32_t t2, uint32_t t3, std::string_view channels, bool mainFBO) {
    internal_Start({
        internal_get_attatchment(t1),
        internal_get_attatchment(t2),
        internal_get_attatchment(t3)
    }, channels, mainFBO);
}
void Engine::priv::GBuffer::bindFramebuffers(uint32_t t1, uint32_t t2, uint32_t t3, uint32_t t4, std::string_view channels, bool mainFBO) {
    internal_Start({ 
        internal_get_attatchment(t1),
        internal_get_attatchment(t2),
        internal_get_attatchment(t3),
        internal_get_attatchment(t4)
    }, channels, mainFBO);
}
void Engine::priv::GBuffer::bindFramebuffers(uint32_t t1, uint32_t t2, uint32_t t3, uint32_t t4, uint32_t t5, std::string_view channels, bool mainFBO) {
    internal_Start({ 
        internal_get_attatchment(t1),
        internal_get_attatchment(t2),
        internal_get_attatchment(t3),
        internal_get_attatchment(t4),
        internal_get_attatchment(t5)
    }, channels, mainFBO);
}

void Engine::priv::GBuffer::bindBackbuffer(GLuint final_fbo, GLuint final_rbo) {
    Engine::Renderer::bindFBO(final_fbo);
    Engine::Renderer::bindRBO(final_rbo); //probably dont even need this. or only implement this if final_rbo != 0
    Engine::Renderer::colorMask(true, true, true, true);
}
void Engine::priv::GBuffer::bindBackbuffer(const Viewport& viewport, GLuint final_fbo, GLuint final_rbo) {
    GBuffer::bindBackbuffer(final_fbo, final_rbo);
    const auto& viewportDimensions = viewport.getViewportDimensions();
    Engine::Renderer::setViewport(viewportDimensions);
}
Texture& Engine::priv::GBuffer::getTexture(const uint32_t index) const noexcept {
    return m_FramebufferTextures[index]->texture();
}
Engine::priv::FramebufferTexture& Engine::priv::GBuffer::getBuffer(const uint32_t index) const noexcept {
    return *m_FramebufferTextures[index];
}