#include <core/engine/renderer/GBuffer.h>
#include <core/engine/renderer/Renderer.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/scene/Viewport.h>
#include <core/engine/scene/Camera.h>

using namespace std;

vector<tuple<ImageInternalFormat::Format, ImagePixelFormat::Format, ImagePixelType::Type, FramebufferAttatchment::Attatchment>> POPULATE() {
    vector<tuple<ImageInternalFormat::Format, ImagePixelFormat::Format, ImagePixelType::Type, FramebufferAttatchment::Attatchment>> m;
    m.resize(Engine::priv::GBufferType::_TOTAL);
    //internFormat        //pxl_components                   //pxl_format
    m[Engine::priv::GBufferType::Diffuse]   = make_tuple(ImageInternalFormat::RGB8, ImagePixelFormat::RGB, ImagePixelType::UNSIGNED_BYTE, FramebufferAttatchment::Color_0);
    //r,g = Normals as Octahedron Compressed. b = MaterialID and AO. a = Packed Metalness / Smoothness 
    m[Engine::priv::GBufferType::Normal]    = make_tuple(ImageInternalFormat::RGBA16F, ImagePixelFormat::RGBA, ImagePixelType::FLOAT, FramebufferAttatchment::Color_1);
    //r = OutGlow. g = OutSpecular. b = GodRaysRG (nibbles) a = GodRaysB 
    m[Engine::priv::GBufferType::Misc]      = make_tuple(ImageInternalFormat::RGBA8, ImagePixelFormat::RGBA, ImagePixelType::UNSIGNED_BYTE, FramebufferAttatchment::Color_2);
    m[Engine::priv::GBufferType::Lighting]  = make_tuple(ImageInternalFormat::RGB16F, ImagePixelFormat::RGB, ImagePixelType::FLOAT, FramebufferAttatchment::Color_3);
    m[Engine::priv::GBufferType::Bloom]     = make_tuple(ImageInternalFormat::RGBA4, ImagePixelFormat::RGBA, ImagePixelType::UNSIGNED_BYTE, FramebufferAttatchment::Color_0);
    m[Engine::priv::GBufferType::GodRays]   = make_tuple(ImageInternalFormat::RGBA4, ImagePixelFormat::RGBA, ImagePixelType::UNSIGNED_BYTE, FramebufferAttatchment::Color_1);
    m[Engine::priv::GBufferType::Depth]     = make_tuple(ImageInternalFormat::Depth24Stencil8, ImagePixelFormat::DEPTH_STENCIL, ImagePixelType::UNSIGNED_INT_24_8, FramebufferAttatchment::DepthAndStencil);

    return m;
}
vector<tuple<ImageInternalFormat::Format, ImagePixelFormat::Format, ImagePixelType::Type, FramebufferAttatchment::Attatchment>> GBUFFER_TYPE_DATA;


void Engine::priv::GBuffer::init(const unsigned int width, const unsigned int height){
    internalDestruct(); //just incase this method is called on resize, we want to delete any previous buffers

    m_Width  = width;
    m_Height = height;

    m_FBO.cleanup();
    m_SmallFBO.cleanup();

    m_FramebufferTextures.resize(GBufferType::_TOTAL, nullptr);

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
    Engine::Renderer::bindTextureForModification(depthTexture.type(), depthTexture.address());
    depthTexture.setFilter(TextureFilter::Nearest);

    auto& diffuseTexture = m_FramebufferTextures[GBufferType::Diffuse]->texture();
    Engine::Renderer::bindTextureForModification(diffuseTexture.type(), diffuseTexture.address());
    diffuseTexture.setFilter(TextureFilter::Nearest);

    auto& normalTexture = m_FramebufferTextures[GBufferType::Normal]->texture();
    Engine::Renderer::bindTextureForModification(normalTexture.type(), normalTexture.address());
    normalTexture.setFilter(TextureFilter::Nearest);

    auto& godRaysTexture = m_FramebufferTextures[GBufferType::GodRays]->texture();
    Engine::Renderer::bindTextureForModification(godRaysTexture.type(), godRaysTexture.address());
    godRaysTexture.setFilter(TextureFilter::Nearest);
}
void Engine::priv::GBuffer::internalDestruct() {
    m_Width  = 0; 
    m_Height = 0;
    Engine::Renderer::unbindFBO();
    m_FramebufferTextures.clear();
}
Engine::priv::GBuffer::~GBuffer(){
    internalDestruct();
}
const bool Engine::priv::GBuffer::resize(const unsigned int width, const unsigned int height) {
    if (m_Width == width && m_Height == height) {
        return false;
    }
    m_Width  = width;
    m_Height = height;
    m_FBO.resize(width, height);
    m_SmallFBO.resize(width, height);
    return true;
}
void Engine::priv::GBuffer::internalBuildTextureBuffer(FramebufferObject& fbo, const GBufferType::Type gbufferType, const unsigned int w, const unsigned int h) {
    if (GBUFFER_TYPE_DATA.size() == 0) {
        GBUFFER_TYPE_DATA = POPULATE();
    }
    const auto i           = GBUFFER_TYPE_DATA[gbufferType];
    Texture* texture       = NEW Texture(w, h, get<2>(i), get<1>(i), get<0>(i), fbo.divisor());
    m_FramebufferTextures[static_cast<unsigned int>(gbufferType)] = fbo.attatchTexture(texture, get<3>(i));
}
void Engine::priv::GBuffer::internalStart(const unsigned int* types, const unsigned int size, string_view channels, const bool first_fbo) {
     (first_fbo) ? m_FBO.bind() : m_SmallFBO.bind();
    bool r, g, b, a;
    channels.find("R") != string::npos ? r = true : r = false;
    channels.find("G") != string::npos ? g = true : g = false;
    channels.find("B") != string::npos ? b = true : b = false;
    channels.find("A") != string::npos ? a = true : a = false;
    glDrawBuffers(size, types);
    Engine::Renderer::colorMask(r, g, b, a);
}
void Engine::priv::GBuffer::bindFramebuffers(const unsigned int t1, string_view c, const bool mainFBO){
    const unsigned int t[1] = { m_FramebufferTextures[t1]->attatchment() };
    internalStart(t, 1, c, mainFBO);
}
void Engine::priv::GBuffer::bindFramebuffers(const unsigned int t1, const unsigned int t2, string_view c, const bool mainFBO){
    const unsigned int t[2] = { m_FramebufferTextures[t1]->attatchment(), m_FramebufferTextures[t2]->attatchment() };
    internalStart(t, 2, c, mainFBO);
}
void Engine::priv::GBuffer::bindFramebuffers(const unsigned int t1, const unsigned int t2, const unsigned int t3, string_view c, const bool mainFBO){
    const unsigned int t[3] = { m_FramebufferTextures[t1]->attatchment(), m_FramebufferTextures[t2]->attatchment(), m_FramebufferTextures[t3]->attatchment() };
    internalStart(t, 3, c, mainFBO);
}
void Engine::priv::GBuffer::bindFramebuffers(const unsigned int t1, const unsigned int t2, const unsigned int t3, const unsigned int t4, string_view c, const bool mainFBO){
    const unsigned int t[4] = { m_FramebufferTextures[t1]->attatchment(), m_FramebufferTextures[t2]->attatchment(), m_FramebufferTextures[t3]->attatchment(), m_FramebufferTextures[t4]->attatchment() };
    internalStart(t, 4, c, mainFBO);
}
void Engine::priv::GBuffer::bindFramebuffers(const unsigned int t1, const unsigned int t2, const unsigned int t3, const unsigned int t4, const unsigned int t5, string_view c, const bool mainFBO){
    const unsigned int t[5] = { m_FramebufferTextures[t1]->attatchment(), m_FramebufferTextures[t2]->attatchment(), m_FramebufferTextures[t3]->attatchment(), m_FramebufferTextures[t4]->attatchment(), m_FramebufferTextures[t5]->attatchment() };
    internalStart(t, 5, c, mainFBO);
}

void Engine::priv::GBuffer::bindBackbuffer(const Viewport& viewport, const GLuint final_fbo, const GLuint final_rbo){
    Engine::Renderer::bindFBO(final_fbo);
    Engine::Renderer::bindRBO(final_rbo); //probably dont even need this. or only implement this if final_rbo != 0
    Engine::Renderer::colorMask(true, true, true, true);
    const auto& dimensions = viewport.getViewportDimensions();
    Engine::Renderer::setViewport(dimensions.x, dimensions.y, dimensions.z, dimensions.w);
}
const vector<Engine::priv::FramebufferTexture*>& Engine::priv::GBuffer::getBuffers() const{
    return m_FramebufferTextures;
}
Texture& Engine::priv::GBuffer::getTexture(const unsigned int t) const {
    return m_FramebufferTextures[t]->texture();
}
Engine::priv::FramebufferTexture& Engine::priv::GBuffer::getBuffer(const unsigned int t) const {
    return *m_FramebufferTextures[t];
}
const Engine::priv::FramebufferObject& Engine::priv::GBuffer::getMainFBO() const {
    return m_FBO; 
}
const Engine::priv::FramebufferObject& Engine::priv::GBuffer::getSmallFBO() const {
    return m_SmallFBO; 
}
const unsigned int Engine::priv::GBuffer::width() const {
    return m_Width;
}
const unsigned int Engine::priv::GBuffer::height() const {
    return m_Height;
}