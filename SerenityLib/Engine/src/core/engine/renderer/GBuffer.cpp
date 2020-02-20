#include <core/engine/renderer/GBuffer.h>
#include <core/engine/renderer/Renderer.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/scene/Viewport.h>
#include <core/engine/scene/Camera.h>

#include <boost/tuple/tuple.hpp>

using namespace std;

Engine::priv::GBuffer::GBuffer() {
    m_Width = 0;
    m_Height = 0;
}


void Engine::priv::GBuffer::init(const uint& width, const uint& height){
    internalDestruct(); //just incase this method is called on resize, we want to delete any previous buffers

    m_Width  = width;
    m_Height = height;

    m_Buffers.resize(GBufferType::_TOTAL, nullptr);

    m_FBO.init(m_Width, m_Height, 1.0f, 2);
    internalBuildTextureBuffer(m_FBO, GBufferType::Diffuse, m_Width, m_Height);
    internalBuildTextureBuffer(m_FBO, GBufferType::Normal, m_Width, m_Height);
    internalBuildTextureBuffer(m_FBO, GBufferType::Misc, m_Width, m_Height);
    internalBuildTextureBuffer(m_FBO, GBufferType::Lighting, m_Width, m_Height);
    internalBuildTextureBuffer(m_FBO, GBufferType::Depth, m_Width, m_Height);

    if (!m_FBO.check()) 
        return;

    m_SmallFBO.init(m_Width, m_Height, 0.5f, 2);
    internalBuildTextureBuffer(m_SmallFBO, GBufferType::Bloom, m_Width, m_Height);
    internalBuildTextureBuffer(m_SmallFBO, GBufferType::GodRays, m_Width, m_Height);

    if (!m_SmallFBO.check()) 
        return;

    //this should be better performance wise, but clean up this code a bit
    auto& depthTexture = m_Buffers[GBufferType::Depth]->texture();
    Engine::Renderer::bindTextureForModification(depthTexture.type(), depthTexture.address());
    depthTexture.setFilter(TextureFilter::Nearest);

    auto& diffuseTexture = m_Buffers[GBufferType::Diffuse]->texture();
    Engine::Renderer::bindTextureForModification(diffuseTexture.type(), diffuseTexture.address());
    diffuseTexture.setFilter(TextureFilter::Nearest);

    auto& normalTexture = m_Buffers[GBufferType::Normal]->texture();
    Engine::Renderer::bindTextureForModification(normalTexture.type(), normalTexture.address());
    normalTexture.setFilter(TextureFilter::Nearest);

    auto& godRaysTexture = m_Buffers[GBufferType::GodRays]->texture();
    Engine::Renderer::bindTextureForModification(godRaysTexture.type(), godRaysTexture.address());
    godRaysTexture.setFilter(TextureFilter::Nearest);
}
void Engine::priv::GBuffer::internalDestruct() {
    m_Width  = 0; 
    m_Height = 0;
    //SAFE_DELETE(m_FBO);
    //SAFE_DELETE(m_SmallFBO);
    Engine::Renderer::unbindFBO();
    vector_clear(m_Buffers);
}
Engine::priv::GBuffer::~GBuffer(){
    internalDestruct();
}
const bool Engine::priv::GBuffer::resize(const uint& width, const uint& height) {
    if (m_Width == width && m_Height == height)
        return false;
    m_Width = width;
    m_Height = height;
    m_FBO.resize(width, height);
    m_SmallFBO.resize(width, height);
    return true;
}
void Engine::priv::GBuffer::internalBuildTextureBuffer(FramebufferObject& fbo, const GBufferType::Type gbufferType, const uint& w, const uint& h) {
    vector<boost::tuple<ImageInternalFormat::Format, ImagePixelFormat::Format, ImagePixelType::Type, FramebufferAttatchment::Attatchment>> GBUFFER_TYPE_DATA = []() {
        vector<boost::tuple<ImageInternalFormat::Format, ImagePixelFormat::Format, ImagePixelType::Type, FramebufferAttatchment::Attatchment>> m;
        m.resize(static_cast<uint>(GBufferType::_TOTAL));
        //internFormat        //pxl_components                   //pxl_format
        m[GBufferType::Diffuse] = boost::make_tuple(ImageInternalFormat::RGB8, ImagePixelFormat::RGB, ImagePixelType::UNSIGNED_BYTE, FramebufferAttatchment::Color_0);
        //r,g = Normals as Octahedron Compressed. b = MaterialID and AO. a = Packed Metalness / Smoothness 
        m[GBufferType::Normal] = boost::make_tuple(ImageInternalFormat::RGBA16F, ImagePixelFormat::RGBA, ImagePixelType::FLOAT, FramebufferAttatchment::Color_1);
        //r = OutGlow. g = OutSpecular. b = GodRaysRG (nibbles) a = GodRaysB 
        m[GBufferType::Misc] = boost::make_tuple(ImageInternalFormat::RGBA8, ImagePixelFormat::RGBA, ImagePixelType::UNSIGNED_BYTE, FramebufferAttatchment::Color_2);
        m[GBufferType::Lighting] = boost::make_tuple(ImageInternalFormat::RGB16F, ImagePixelFormat::RGB, ImagePixelType::FLOAT, FramebufferAttatchment::Color_3);
        m[GBufferType::Bloom] = boost::make_tuple(ImageInternalFormat::RGBA4, ImagePixelFormat::RGBA, ImagePixelType::UNSIGNED_BYTE, FramebufferAttatchment::Color_0);
        m[GBufferType::GodRays] = boost::make_tuple(ImageInternalFormat::RGBA4, ImagePixelFormat::RGBA, ImagePixelType::UNSIGNED_BYTE, FramebufferAttatchment::Color_1);
        m[GBufferType::Depth] = boost::make_tuple(ImageInternalFormat::Depth24Stencil8, ImagePixelFormat::DEPTH_STENCIL, ImagePixelType::UNSIGNED_INT_24_8, FramebufferAttatchment::DepthAndStencil);

        return m;
    }();
    const auto i = GBUFFER_TYPE_DATA[gbufferType];

    const auto attatchment = i.get<3>();
    Texture* texture = NEW Texture(w, h, i.get<2>(), i.get<1>(), i.get<0>(), fbo.divisor());
    m_Buffers[static_cast<uint>(gbufferType)] = fbo.attatchTexture(texture, attatchment);
}
void Engine::priv::GBuffer::internalStart(const unsigned int* types, const unsigned int& size, const string& channels, const bool first_fbo) {
     (first_fbo) ? m_FBO.bind() : m_SmallFBO.bind();
    bool r, g, b, a;
    channels.find("R") != string::npos ? r = true : r = false;
    channels.find("G") != string::npos ? g = true : g = false;
    channels.find("B") != string::npos ? b = true : b = false;
    channels.find("A") != string::npos ? a = true : a = false;
    glDrawBuffers(size, types);
    Engine::Renderer::colorMask(r, g, b, a);
}
void Engine::priv::GBuffer::bindFramebuffers(const unsigned int t1, const string& c, const bool mainFBO){
    const unsigned int t[1] = { m_Buffers[t1]->attatchment() };
    internalStart(t, 1, c, mainFBO);
}
void Engine::priv::GBuffer::bindFramebuffers(const unsigned int t1, const unsigned int t2, const string& c, const bool mainFBO){
    const unsigned int t[2] = { m_Buffers[t1]->attatchment(),m_Buffers[t2]->attatchment() };
    internalStart(t, 2, c, mainFBO);
}
void Engine::priv::GBuffer::bindFramebuffers(const unsigned int t1, const unsigned int t2, const unsigned int t3, const string& c, const bool mainFBO){
    const unsigned int t[3] = { m_Buffers[t1]->attatchment(),m_Buffers[t2]->attatchment(),m_Buffers[t3]->attatchment() };
    internalStart(t, 3, c, mainFBO);
}
void Engine::priv::GBuffer::bindFramebuffers(const unsigned int t1, const unsigned int t2, const unsigned int t3, const unsigned int t4, const string& c, const bool mainFBO){
    const unsigned int t[4] = { m_Buffers[t1]->attatchment(),m_Buffers[t2]->attatchment(),m_Buffers[t3]->attatchment(),m_Buffers[t4]->attatchment() };
    internalStart(t, 4, c, mainFBO);
}
void Engine::priv::GBuffer::bindFramebuffers(const unsigned int t1, const unsigned int t2, const unsigned int t3, const unsigned int t4, const unsigned int t5, const string& c, const bool mainFBO){
    const unsigned int t[5] = { m_Buffers[t1]->attatchment(),m_Buffers[t2]->attatchment(),m_Buffers[t3]->attatchment(),m_Buffers[t4]->attatchment(),m_Buffers[t5]->attatchment() };
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
    return m_Buffers; 
}
Texture& Engine::priv::GBuffer::getTexture(const uint t) const {
    return m_Buffers[t]->texture();
}
Engine::priv::FramebufferTexture& Engine::priv::GBuffer::getBuffer(const uint t) const {
    return *m_Buffers[t]; 
}
const Engine::priv::FramebufferObject& Engine::priv::GBuffer::getMainFBO() const {
    return m_FBO; 
}
const Engine::priv::FramebufferObject& Engine::priv::GBuffer::getSmallFBO() const {
    return m_SmallFBO; 
}