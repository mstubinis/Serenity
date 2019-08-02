#include <core/engine/renderer/GBuffer.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/renderer/FramebufferObject.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/scene/Viewport.h>
#include <core/engine/scene/Camera.h>

#include <boost/tuple/tuple.hpp>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;

vector<boost::tuple<ImageInternalFormat::Format,ImagePixelFormat::Format,ImagePixelType::Type,FramebufferAttatchment::Attatchment>> GBUFFER_TYPE_DATA = [](){
    vector<boost::tuple<ImageInternalFormat::Format,ImagePixelFormat::Format,ImagePixelType::Type,FramebufferAttatchment::Attatchment>> m;
    m.resize(epriv::GBufferType::_TOTAL);
                                                           //internFormat        //pxl_components                   //pxl_format
    m[GBufferType::Diffuse]  = boost::make_tuple(ImageInternalFormat::RGB8,             ImagePixelFormat::RGB,           ImagePixelType::UNSIGNED_BYTE,      FramebufferAttatchment::Color_0);
    //r,g = Normals as Octahedron Compressed. b = MaterialID and AO. a = Packed Metalness / Smoothness 
    m[GBufferType::Normal]   = boost::make_tuple(ImageInternalFormat::RGBA16F,          ImagePixelFormat::RGBA,          ImagePixelType::FLOAT,              FramebufferAttatchment::Color_1);
    //r = OutGlow. g = OutSpecular. b = GodRaysRG (nibbles) a = GodRaysB 
    m[GBufferType::Misc]     = boost::make_tuple(ImageInternalFormat::RGBA8,            ImagePixelFormat::RGBA,          ImagePixelType::UNSIGNED_BYTE,      FramebufferAttatchment::Color_2);
    m[GBufferType::Lighting] = boost::make_tuple(ImageInternalFormat::RGB16F,           ImagePixelFormat::RGB,           ImagePixelType::FLOAT,              FramebufferAttatchment::Color_3);
    m[GBufferType::Bloom]    = boost::make_tuple(ImageInternalFormat::RGBA4,            ImagePixelFormat::RGBA,          ImagePixelType::UNSIGNED_BYTE,      FramebufferAttatchment::Color_0);
    m[GBufferType::GodRays]  = boost::make_tuple(ImageInternalFormat::RGBA4,            ImagePixelFormat::RGBA,          ImagePixelType::UNSIGNED_BYTE,      FramebufferAttatchment::Color_1);
    m[GBufferType::Depth]    = boost::make_tuple(ImageInternalFormat::Depth24Stencil8,  ImagePixelFormat::DEPTH_STENCIL, ImagePixelType::UNSIGNED_INT_24_8,  FramebufferAttatchment::DepthAndStencil);

    return m;
}();
GBuffer::GBuffer(const uint& width, const uint& height){
    m_FBO = m_SmallFBO = nullptr;
    internalDestruct(); //just incase this method is called on resize, we want to delete any previous buffers

    m_Width  = width;
    m_Height = height;

    m_Buffers.resize(GBufferType::_TOTAL);

    m_FBO = new FramebufferObject("GBuffer_FBO", m_Width, m_Height, 1.0f, 2);
    internalBuildTextureBuffer(m_FBO, GBufferType::Diffuse, m_Width, m_Height);
    internalBuildTextureBuffer(m_FBO, GBufferType::Normal, m_Width, m_Height);
    internalBuildTextureBuffer(m_FBO, GBufferType::Misc, m_Width, m_Height);
    internalBuildTextureBuffer(m_FBO, GBufferType::Lighting, m_Width, m_Height);
    internalBuildTextureBuffer(m_FBO, GBufferType::Depth, m_Width, m_Height);

    if (!m_FBO->check()) return;

    m_SmallFBO = new FramebufferObject("GBuffer_Small_FBO", m_Width, m_Height, 0.5f, 2);
    internalBuildTextureBuffer(m_SmallFBO, GBufferType::Bloom, m_Width, m_Height);
    internalBuildTextureBuffer(m_SmallFBO, GBufferType::GodRays, m_Width, m_Height);

    if (!m_SmallFBO->check()) return;

    //this should be better performance wise, but clean up this code a bit
    auto& depthTexture = m_Buffers[GBufferType::Depth]->texture();
    Renderer::bindTextureForModification(depthTexture.type(), depthTexture.address());
    depthTexture.setFilter(TextureFilter::Nearest);

    auto& diffuseTexture = m_Buffers[GBufferType::Diffuse]->texture();
    Renderer::bindTextureForModification(diffuseTexture.type(), diffuseTexture.address());
    diffuseTexture.setFilter(TextureFilter::Nearest);

    auto& normalTexture = m_Buffers[GBufferType::Normal]->texture();
    Renderer::bindTextureForModification(normalTexture.type(), normalTexture.address());
    normalTexture.setFilter(TextureFilter::Nearest);

    auto& godRaysTexture = m_Buffers[GBufferType::GodRays]->texture();
    Renderer::bindTextureForModification(godRaysTexture.type(), godRaysTexture.address());
    godRaysTexture.setFilter(TextureFilter::Nearest);
}
void GBuffer::internalDestruct() {
    m_Width  = 0; 
    m_Height = 0;
    SAFE_DELETE(m_FBO);
    SAFE_DELETE(m_SmallFBO);
    Renderer::unbindFBO();
    vector_clear(m_Buffers);
}
GBuffer::~GBuffer(){ 
    internalDestruct();
}
void GBuffer::internalBuildTextureBuffer(FramebufferObject* fbo, const uint& t, const uint& w, const uint& h) {
    auto& i = GBUFFER_TYPE_DATA[t];
    Texture* texture = new Texture(w, h, i.get<2>(), i.get<1>(), i.get<0>(), fbo->divisor());
    m_Buffers[t] = fbo->attatchTexture(texture, i.get<3>());
}
bool GBuffer::resize(const uint& width, const uint& height){
    if (m_Width == width && m_Height == height)
        return false;
    m_Width = width;
    m_Height = height;
    m_FBO->resize(width, height);
    m_SmallFBO->resize(width, height);
    return true;
}
void GBuffer::internalStart(const uint* types, const uint& size, const string& channels, const bool first_fbo) {
    if (first_fbo) { m_FBO->bind(); }
    else { m_SmallFBO->bind(); }
    bool r, g, b, a;
    channels.find("R") != string::npos ? r = true : r = false;
    channels.find("G") != string::npos ? g = true : g = false;
    channels.find("B") != string::npos ? b = true : b = false;
    channels.find("A") != string::npos ? a = true : a = false;
    glDrawBuffers(size, types);
    Renderer::colorMask(r, g, b, a);
}
void GBuffer::bindFramebuffers(const uint t1, const string& c, const bool mainFBO){
    uint t[1] = { m_Buffers[t1]->attatchment() };
    internalStart(t, 1, c, mainFBO);
}
void GBuffer::bindFramebuffers(const uint t1, const uint t2, const string& c, const bool mainFBO){
    uint t[2] = { m_Buffers[t1]->attatchment(),m_Buffers[t2]->attatchment() };
    internalStart(t, 2, c, mainFBO);
}
void GBuffer::bindFramebuffers(const uint t1, const uint t2, const uint t3, const string& c, const bool mainFBO){
    uint t[3] = { m_Buffers[t1]->attatchment(),m_Buffers[t2]->attatchment(),m_Buffers[t3]->attatchment() };
    internalStart(t, 3, c, mainFBO);
}
void GBuffer::bindFramebuffers(const uint t1, const uint t2, const uint t3, const uint t4, const string& c, const bool mainFBO){
    uint t[4] = { m_Buffers[t1]->attatchment(),m_Buffers[t2]->attatchment(),m_Buffers[t3]->attatchment(),m_Buffers[t4]->attatchment() };
    internalStart(t, 4, c, mainFBO);
}
void GBuffer::bindFramebuffers(const uint t1, const uint t2, const uint t3, const uint t4, const uint t5, const string& c, const bool mainFBO){
    uint t[5] = { m_Buffers[t1]->attatchment(),m_Buffers[t2]->attatchment(),m_Buffers[t3]->attatchment(),m_Buffers[t4]->attatchment(),m_Buffers[t5]->attatchment() };
    internalStart(t, 5, c, mainFBO);
}

void GBuffer::bindBackbuffer(Viewport& viewport, const GLuint final_fbo, const GLuint final_rbo){
    Renderer::bindFBO(final_fbo);
    Renderer::bindRBO(final_rbo); //probably dont even need this. or only implement this if final_rbo != 0
    Renderer::colorMask(true, true, true, true);
    auto& dimensions = viewport.getViewportDimensions();
    Renderer::setViewport(dimensions.x, dimensions.y, dimensions.z, dimensions.w);
}
const vector<FramebufferTexture*>& GBuffer::getBuffers() const{ 
    return m_Buffers; 
}
Texture& GBuffer::getTexture(const uint t){
    return m_Buffers[t]->texture();
}
FramebufferTexture& GBuffer::getBuffer(const uint t){
    return *m_Buffers[t]; 
}
FramebufferObject* GBuffer::getMainFBO(){ 
    return m_FBO; 
}
FramebufferObject* GBuffer::getSmallFBO(){ 
    return m_SmallFBO; 
}