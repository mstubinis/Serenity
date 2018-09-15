#include "GBuffer.h"
#include "Texture.h"
#include "Engine_Renderer.h"
#include "FramebufferObject.h"

#include <boost/tuple/tuple.hpp>

using namespace Engine;
using namespace std;

vector<boost::tuple<ImageInternalFormat::Format,ImagePixelFormat::Format,ImagePixelType::Type,FramebufferAttatchment::Attatchment>> GBUFFER_TYPE_DATA = [](){
    vector<boost::tuple<ImageInternalFormat::Format,ImagePixelFormat::Format,ImagePixelType::Type,FramebufferAttatchment::Attatchment>> m; m.resize(epriv::GBufferType::_TOTAL);
                                                           //internFormat        //pxl_components                   //pxl_format
    m.at(epriv::GBufferType::Diffuse)  = boost::make_tuple(ImageInternalFormat::RGB8,     ImagePixelFormat::RGB,             ImagePixelType::UNSIGNED_BYTE,  FramebufferAttatchment::Color_0);
    m.at(epriv::GBufferType::Normal)   = boost::make_tuple(ImageInternalFormat::RGBA16F,  ImagePixelFormat::RGBA,            ImagePixelType::FLOAT,  FramebufferAttatchment::Color_1);
    m.at(epriv::GBufferType::Misc)     = boost::make_tuple(ImageInternalFormat::RGBA8,    ImagePixelFormat::RGBA,            ImagePixelType::FLOAT,  FramebufferAttatchment::Color_2);
    m.at(epriv::GBufferType::Lighting) = boost::make_tuple(ImageInternalFormat::RGB16F,   ImagePixelFormat::RGB,             ImagePixelType::FLOAT,  FramebufferAttatchment::Color_3);
    m.at(epriv::GBufferType::Bloom)    = boost::make_tuple(ImageInternalFormat::RGBA4,    ImagePixelFormat::RGBA,            ImagePixelType::UNSIGNED_BYTE,  FramebufferAttatchment::Color_0);
    m.at(epriv::GBufferType::GodRays)  = boost::make_tuple(ImageInternalFormat::RGBA4,    ImagePixelFormat::RGBA,            ImagePixelType::UNSIGNED_BYTE,  FramebufferAttatchment::Color_1);
    m.at(epriv::GBufferType::Depth)    = boost::make_tuple(ImageInternalFormat::Depth24Stencil8,  ImagePixelFormat::DEPTH_STENCIL, ImagePixelType::UNSIGNED_INT_24_8,  FramebufferAttatchment::DepthAndStencil);

    return m;
}();

class epriv::GBuffer::impl final{
    public:
        FramebufferObject *m_FBO, *m_SmallFBO;
        vector<FramebufferTexture*> m_Buffers;
        uint m_Width, m_Height;
        bool _init(const uint& w, const uint& h){
            m_FBO = m_SmallFBO = nullptr;
            _destruct(); //just incase this method is called on resize, we want to delete any previous buffers

            m_Width = w; m_Height = h;

            m_Buffers.resize(GBufferType::_TOTAL,nullptr);

            m_FBO = new FramebufferObject("GBuffer_FBO",m_Width,m_Height,1.0f,2);
            _constructTextureBuffer(m_FBO,GBufferType::Diffuse,   m_Width,m_Height);
            _constructTextureBuffer(m_FBO,GBufferType::Normal,    m_Width,m_Height);
            _constructTextureBuffer(m_FBO,GBufferType::Misc,      m_Width,m_Height);
            _constructTextureBuffer(m_FBO,GBufferType::Lighting,  m_Width,m_Height);
            _constructTextureBuffer(m_FBO,GBufferType::Depth,     m_Width,m_Height);

            if(!m_FBO->check()) return false;

            m_SmallFBO = new FramebufferObject("GBuffer_Small_FBO",m_Width,m_Height,0.25f,2);
            _constructTextureBuffer(m_SmallFBO,GBufferType::Bloom,   m_Width,m_Height);
            _constructTextureBuffer(m_SmallFBO,GBufferType::GodRays, m_Width,m_Height);
            
            if(!m_SmallFBO->check()) return false;

            //this should be better performance wise, but clean up this code a bit
            Texture& depthTexture = *m_Buffers.at(GBufferType::Depth)->texture();
            Renderer::bindTexture(depthTexture.type(),depthTexture.address());
            depthTexture.setFilter(TextureFilter::Nearest);

            Texture& diffuseTexture = *m_Buffers.at(GBufferType::Diffuse)->texture();
            Renderer::bindTexture(diffuseTexture.type(),diffuseTexture.address());
            diffuseTexture.setFilter(TextureFilter::Nearest);

            Texture& normalTexture = *m_Buffers.at(GBufferType::Normal)->texture();
            Renderer::bindTexture(normalTexture.type(),normalTexture.address());
            normalTexture.setFilter(TextureFilter::Nearest);

            Texture& godRaysTexture = *m_Buffers.at(GBufferType::GodRays)->texture();
            Renderer::bindTexture(godRaysTexture.type(),godRaysTexture.address());
            godRaysTexture.setFilter(TextureFilter::Nearest);

            return true;
        }
        void _resize(const uint& w, const uint& h){
            m_Width = w; m_Height = h;
            m_FBO->resize(w,h);
            m_SmallFBO->resize(w,h);
        }
        void _constructTextureBuffer(FramebufferObject* fbo, const uint& t, const uint& w, const uint& h){
            auto& i = GBUFFER_TYPE_DATA.at(t);
            m_Buffers.at(t) = fbo->attatchTexture(new Texture(w,h,i.get<2>(),i.get<1>(),i.get<0>(),fbo->divisor()),i.get<3>());
        }
        void _destruct(){
            m_Width = m_Height = 0;
            SAFE_DELETE(m_FBO);
            SAFE_DELETE(m_SmallFBO);
            Renderer::unbindFBO();
            vector_clear(m_Buffers);
        }
        void _start(uint* types,uint size,const string& channels,bool first_fbo){
            if(first_fbo){ m_FBO->bind(); }
            else{ m_SmallFBO->bind(); }
            GLboolean r,g,b,a;
            channels.find("R") != string::npos ? r = GL_TRUE : r = GL_FALSE;
            channels.find("G") != string::npos ? g = GL_TRUE : g = GL_FALSE;
            channels.find("B") != string::npos ? b = GL_TRUE : b = GL_FALSE;
            channels.find("A") != string::npos ? a = GL_TRUE : a = GL_FALSE;
            glColorMask(r,g,b,a);
            glDrawBuffers(size, types); // Specify what to render an start acquiring
        }
        void _start(const uint& t1, const string& c,bool f){
            uint t[1] = { m_Buffers.at(t1)->attatchment()  };
            _start(t,1,c,f);
        }
        void _start(const uint& t1, const uint& t2, const string& c,bool f){
            uint t[2] = { m_Buffers.at(t1)->attatchment(),m_Buffers.at(t2)->attatchment() };
            _start(t,2,c,f);
        }
        void _start(const uint& t1, const uint& t2, const uint& t3, const string& c,bool f){
            uint t[3] = { m_Buffers.at(t1)->attatchment(),m_Buffers.at(t2)->attatchment(),m_Buffers.at(t3)->attatchment() };
            _start(t,3,c,f);
        }
        void _start(const uint& t1, const uint& t2, const uint& t3, const uint& t4, const string& c,bool f){
            uint t[4] = { m_Buffers.at(t1)->attatchment(),m_Buffers.at(t2)->attatchment(),m_Buffers.at(t3)->attatchment(),m_Buffers.at(t4)->attatchment() };
            _start(t,4,c,f);
        }
        void _start(const uint& t1, const uint& t2, const uint& t3, const uint& t4, const uint& t5, const string& c,bool f){
            uint t[5] = { m_Buffers.at(t1)->attatchment(),m_Buffers.at(t2)->attatchment(),m_Buffers.at(t3)->attatchment(),
            m_Buffers.at(t4)->attatchment(),m_Buffers.at(t5)->attatchment() };
            _start(t,5,c,f);
        }
        void _stop(const GLuint& final_fbo, const GLuint& final_rbo){
            Renderer::bindFBO(final_fbo);
            Renderer::bindRBO(final_rbo); //probably dont even need this. or only implement this if final_rbo != 0
            glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
            glClear(GL_COLOR_BUFFER_BIT);
        }
};
epriv::GBuffer::GBuffer(uint width,uint height):m_i(new impl){ m_i->_init(width,height); }
epriv::GBuffer::~GBuffer(){ m_i->_destruct(); }
void epriv::GBuffer::resize(uint width, uint height){ m_i->_resize(width,height); }
void epriv::GBuffer::start(uint t,string c,bool mainFBO){m_i->_start(t,c,mainFBO);}
void epriv::GBuffer::start(uint t,uint t1,string c,bool mainFBO){m_i->_start(t,t1,c,mainFBO);}
void epriv::GBuffer::start(uint t,uint t1,uint t2,string c,bool mainFBO){m_i->_start(t,t1,t2,c,mainFBO);}
void epriv::GBuffer::start(uint t,uint t1,uint t2,uint t3,string c,bool mainFBO){m_i->_start(t,t1,t2,t3,c,mainFBO);}
void epriv::GBuffer::start(uint t,uint t1,uint t2,uint t3,uint t4,string c,bool mainFBO){m_i->_start(t,t1,t2,t3,t4,c,mainFBO);}
void epriv::GBuffer::stop(GLuint fbo, GLuint rbo){m_i->_stop(fbo,rbo);}
const vector<epriv::FramebufferTexture*>& epriv::GBuffer::getBuffers() const{ return m_i->m_Buffers; }
Texture* epriv::GBuffer::getTexture(uint t){ return m_i->m_Buffers.at(t)->texture();}
epriv::FramebufferTexture* epriv::GBuffer::getBuffer(uint t){ return m_i->m_Buffers.at(t); }
epriv::FramebufferObject* epriv::GBuffer::getMainFBO(){ return m_i->m_FBO; }
epriv::FramebufferObject* epriv::GBuffer::getSmallFBO(){ return m_i->m_SmallFBO; }