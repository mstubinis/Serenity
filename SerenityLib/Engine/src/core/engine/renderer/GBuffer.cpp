#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/renderer/GBuffer.h>
#include <core/engine/renderer/Renderer.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/scene/Viewport.h>

constexpr std::array<std::tuple<ImageInternalFormat, ImagePixelFormat, ImagePixelType, FramebufferAttatchment>, Engine::priv::GBufferType::_TOTAL> GBUFFER_TYPE_DATA{{
    {ImageInternalFormat::RGB8, ImagePixelFormat::RGB, ImagePixelType::UNSIGNED_BYTE, FramebufferAttatchment::Color_0},
    {ImageInternalFormat::RGBA16F, ImagePixelFormat::RGBA, ImagePixelType::FLOAT, FramebufferAttatchment::Color_1},//r,g = NormalsOctahedronCompressed, b = MaterialID & AO, a = Packed Metalness / Smoothness 
    {ImageInternalFormat::RGBA8, ImagePixelFormat::RGBA, ImagePixelType::UNSIGNED_BYTE, FramebufferAttatchment::Color_2},//r = OutGlow, g = OutSpecular, b = GodRaysRG (nibbles), a = GodRaysB 
    {ImageInternalFormat::RGB16F, ImagePixelFormat::RGB, ImagePixelType::FLOAT, FramebufferAttatchment::Color_3},
    {ImageInternalFormat::RGBA4, ImagePixelFormat::RGBA, ImagePixelType::UNSIGNED_BYTE, FramebufferAttatchment::Color_0},
    {ImageInternalFormat::RGBA4, ImagePixelFormat::RGBA, ImagePixelType::UNSIGNED_BYTE, FramebufferAttatchment::Color_1},
    {ImageInternalFormat::Depth24Stencil8, ImagePixelFormat::DEPTH_STENCIL, ImagePixelType::UNSIGNED_INT_24_8, FramebufferAttatchment::DepthAndStencil},
}};

void Engine::priv::GBuffer::init(unsigned int width, unsigned int height){
    internalDestruct(); //just incase this method is called on resize, we want to delete any previous buffers

    m_Width  = width;
    m_Height = height;

    m_FBO.cleanup();
    m_SmallFBO.cleanup();

    m_FramebufferTextures.fill(nullptr);

    m_FBO.init(m_Width, m_Height, 1.0f, 2);
    internalBuildTextureBuffer(m_FBO, GBufferType::Diffuse, m_Width, m_Height);
    internalBuildTextureBuffer(m_FBO, GBufferType::Normal, m_Width, m_Height);
    internalBuildTextureBuffer(m_FBO, GBufferType::Misc, m_Width, m_Height);
    internalBuildTextureBuffer(m_FBO, GBufferType::Lighting, m_Width, m_Height);
    internalBuildTextureBuffer(m_FBO, GBufferType::Depth, m_Width, m_Height);

    if (!m_FBO.check()) {
        return;
    }
    m_SmallFBO.init(m_Width, m_Height, 0.5f, 2);
    internalBuildTextureBuffer(m_SmallFBO, GBufferType::Bloom, m_Width, m_Height);
    internalBuildTextureBuffer(m_SmallFBO, GBufferType::GodRays, m_Width, m_Height);

    if (!m_SmallFBO.check()) {
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
void Engine::priv::GBuffer::internalDestruct() {
    m_Width  = 0; 
    m_Height = 0;
    Engine::Renderer::unbindFBO();
    m_FramebufferTextures.fill(nullptr);
}
Engine::priv::GBuffer::~GBuffer(){
    internalDestruct();
}
bool Engine::priv::GBuffer::resize(unsigned int width, unsigned int height) {
    if (m_Width == width && m_Height == height) {
        return false;
    }
    m_Width  = width;
    m_Height = height;
    m_FBO.resize(width, height);
    m_SmallFBO.resize(width, height);
    return true;
}
void Engine::priv::GBuffer::internalBuildTextureBuffer(FramebufferObject& fbo, GBufferType::Type gbufferType, unsigned int w, unsigned int h) {
    const auto [internalFmt, pxlFmt, pxlType, fbAttatch] = GBUFFER_TYPE_DATA[gbufferType];
    Texture* texture  = NEW Texture(w, h, pxlType, pxlFmt, internalFmt, fbo.divisor());
    m_FramebufferTextures[(unsigned int)gbufferType] = fbo.attatchTexture(texture, fbAttatch);
}
void Engine::priv::GBuffer::internalStart(std::vector<unsigned int>& types, unsigned int size, std::string_view channels, bool first_fbo) {
    (first_fbo) ? m_FBO.bind() : m_SmallFBO.bind();
    bool r = (channels.find("R") != std::string::npos);
    bool g = (channels.find("G") != std::string::npos);
    bool b = (channels.find("B") != std::string::npos);
    bool a = (channels.find("A") != std::string::npos);

    GLCall(glDrawBuffers(size, types.data()));
    Engine::Renderer::colorMask(r, g, b, a);
}
void Engine::priv::GBuffer::bindFramebuffers(std::string_view c, bool mainFBO) {
    std::vector<unsigned int> t = { 0 };
    internalStart(t, 0, c, mainFBO);
}
void Engine::priv::GBuffer::bindFramebuffers(unsigned int buffer, std::string_view c, bool mainFBO) {
    std::vector<unsigned int> buffers_as_slots = { m_FramebufferTextures[buffer]->attatchment() };
    internalStart(buffers_as_slots, 1, c, mainFBO);
}
void Engine::priv::GBuffer::bindFramebuffers(unsigned int t1, unsigned int t2, std::string_view c, bool mainFBO){
    std::vector<unsigned int> buffers_as_slots = { m_FramebufferTextures[t1]->attatchment(), m_FramebufferTextures[t2]->attatchment() };
    internalStart(buffers_as_slots, 2, c, mainFBO);
}
void Engine::priv::GBuffer::bindFramebuffers(unsigned int t1, unsigned int t2, unsigned int t3, std::string_view c, bool mainFBO){
    std::vector<unsigned int> buffers_as_slots = { m_FramebufferTextures[t1]->attatchment(), m_FramebufferTextures[t2]->attatchment(), m_FramebufferTextures[t3]->attatchment() };
    internalStart(buffers_as_slots, 3, c, mainFBO);
}
void Engine::priv::GBuffer::bindFramebuffers(unsigned int t1, unsigned int t2, unsigned int t3, unsigned int t4, std::string_view c, bool mainFBO){
    std::vector<unsigned int> buffers_as_slots = { m_FramebufferTextures[t1]->attatchment(), m_FramebufferTextures[t2]->attatchment(), m_FramebufferTextures[t3]->attatchment(), m_FramebufferTextures[t4]->attatchment() };
    internalStart(buffers_as_slots, 4, c, mainFBO);
}
void Engine::priv::GBuffer::bindFramebuffers(unsigned int t1, unsigned int t2, unsigned int t3, unsigned int t4, unsigned int t5, std::string_view c, bool mainFBO){
    std::vector<unsigned int> buffers_as_slots = { m_FramebufferTextures[t1]->attatchment(), m_FramebufferTextures[t2]->attatchment(), m_FramebufferTextures[t3]->attatchment(), m_FramebufferTextures[t4]->attatchment(), m_FramebufferTextures[t5]->attatchment() };
    internalStart(buffers_as_slots, 5, c, mainFBO);
}


void Engine::priv::GBuffer::bindBackbuffer(const Viewport& viewport, GLuint final_fbo, GLuint final_rbo){
    Engine::Renderer::bindFBO(final_fbo);
    Engine::Renderer::bindRBO(final_rbo); //probably dont even need this. or only implement this if final_rbo != 0
    Engine::Renderer::colorMask(true, true, true, true);
    const auto& dimensions = viewport.getViewportDimensions();
    Engine::Renderer::setViewport(dimensions.x, dimensions.y, dimensions.z, dimensions.w);
}
Texture& Engine::priv::GBuffer::getTexture(unsigned int t) const {
    return m_FramebufferTextures[t]->texture();
}
Engine::priv::FramebufferTexture& Engine::priv::GBuffer::getBuffer(unsigned int t) const {
    return *m_FramebufferTextures[t];
}