#include "GBuffer.h"
#include "Engine_Resources.h"
#include "Engine_Window.h"
#include "Engine_Renderer.h"
#include "FramebufferObject.h"

#include <boost/tuple/tuple.hpp>

using namespace Engine;
using namespace std;

unordered_map<uint,boost::tuple<float,ImageInternalFormat::Format,ImagePixelFormat::Format,ImagePixelType::Type,FramebufferAttatchment::Attatchment>> _populateGBufferTypesInfo(){
    unordered_map<uint,boost::tuple<float,ImageInternalFormat::Format,ImagePixelFormat::Format,ImagePixelType::Type,FramebufferAttatchment::Attatchment>> m;
                                       //winSizeRatio   //internFormat                 //pxl_components                   //pxl_format
    m[GBufferType::Diffuse]  = boost::make_tuple(1.0f,  ImageInternalFormat::RGB8,     ImagePixelFormat::RGB,             ImagePixelType::FLOAT,  FramebufferAttatchment::Color_0);
    m[GBufferType::Normal]   = boost::make_tuple(1.0f,  ImageInternalFormat::RGBA16F,  ImagePixelFormat::RGBA,            ImagePixelType::FLOAT,  FramebufferAttatchment::Color_1);
    m[GBufferType::Misc]     = boost::make_tuple(1.0f,  ImageInternalFormat::RGBA8,    ImagePixelFormat::RGBA,            ImagePixelType::FLOAT,  FramebufferAttatchment::Color_2);
    m[GBufferType::Lighting] = boost::make_tuple(1.0f,  ImageInternalFormat::RGB16F,   ImagePixelFormat::RGB,             ImagePixelType::FLOAT,  FramebufferAttatchment::Color_3);
    m[GBufferType::Bloom]    = boost::make_tuple(0.5f,  ImageInternalFormat::RGBA8,    ImagePixelFormat::RGBA,            ImagePixelType::FLOAT,  FramebufferAttatchment::Color_0);
    m[GBufferType::GodRays]  = boost::make_tuple(0.5f,  ImageInternalFormat::RGB8,     ImagePixelFormat::RGB,             ImagePixelType::FLOAT,  FramebufferAttatchment::Color_1);
    m[GBufferType::Free2]    = boost::make_tuple(0.5f,  ImageInternalFormat::RGBA8,    ImagePixelFormat::RGBA,            ImagePixelType::FLOAT,  FramebufferAttatchment::Color_2);
    m[GBufferType::Depth]    = boost::make_tuple(1.0f,  ImageInternalFormat::Depth16,  ImagePixelFormat::DEPTH_COMPONENT, ImagePixelType::FLOAT,  FramebufferAttatchment::Depth);

    return m;
}
unordered_map<uint,boost::tuple<float,ImageInternalFormat::Format,ImagePixelFormat::Format,ImagePixelType::Type,FramebufferAttatchment::Attatchment>> GBUFFER_TYPE_DATA = _populateGBufferTypesInfo();

class GBuffer::impl final{
    public:
        FramebufferObject* m_FBO;
        FramebufferObject* m_SmallFBO;
        unordered_map<uint,FramebufferTexture*> m_Buffers;
        uint m_Width; uint m_Height;
        bool _init(uint w,uint h){
            _destruct(); //just incase this method is called on resize, we want to delete any previous buffers

            m_Width = w; m_Height = h;

            m_FBO = new FramebufferObject("GBuffer_FBO",m_Width,m_Height,ImageInternalFormat::Depth16);
            m_FBO->bind();

            _constructFramebuffer(m_FBO,"BUFFER_DIFFUSE", GBufferType::Diffuse, m_Width,m_Height);
            _constructFramebuffer(m_FBO,"BUFFER_NORMAL",  GBufferType::Normal,  m_Width,m_Height);
            _constructFramebuffer(m_FBO,"BUFFER_MISC",    GBufferType::Misc,    m_Width,m_Height);
            _constructFramebuffer(m_FBO,"BUFFER_LIGHTING",GBufferType::Lighting,m_Width,m_Height);
            _constructFramebuffer(m_FBO,"BUFFER_DEPTH",   GBufferType::Depth,   m_Width,m_Height);

            if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
                return false;
            }
            m_SmallFBO = new FramebufferObject("GBuffer_Small_FBO",m_Width,m_Height,ImageInternalFormat::Depth16);
            m_SmallFBO->bind();

            _constructFramebuffer(m_SmallFBO,"BUFFER_BLOOM",   GBufferType::Bloom,   m_Width,m_Height);
            _constructFramebuffer(m_SmallFBO,"BUFFER_FREE2",   GBufferType::Free2,   m_Width,m_Height);
            _constructFramebuffer(m_SmallFBO,"BUFFER_GODSRAYS",GBufferType::GodRays, m_Width,m_Height);

            if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
                return false;
            }
            return true;
        }
        void _resize(uint w, uint h){
            m_Width = w; m_Height = h;
            m_FBO->bind();
            m_FBO->resize(w,h);
            m_SmallFBO->bind();
            m_SmallFBO->resize(w,h);
        }
        void _constructFramebuffer(FramebufferObject* fbo,string n,uint t,uint w,uint h){
            boost::tuple<float,ImageInternalFormat::Format,ImagePixelFormat::Format,ImagePixelType::Type,FramebufferAttatchment::Attatchment>& i = GBUFFER_TYPE_DATA.at(t);
            Texture* texture = new Texture(n,w,h,i.get<1>(),i.get<2>(),i.get<3>(),GL_TEXTURE_2D,i.get<0>());
            m_Buffers.emplace(t,fbo->attatchTexture(texture,i.get<4>(),i.get<0>()));
        }
        void _destruct(){
            m_Width = m_Height = 0;

            delete m_FBO;
            delete m_SmallFBO;

            Renderer::unbindFBO();
        }
        void _start(vector<uint>& types,string& channels,bool first_fbo){
            if(first_fbo){ m_FBO->bind(); }
            else{ m_SmallFBO->bind(); }
            GLboolean r,g,b,a;
            if(channels.find("R") != string::npos) r=GL_TRUE; else r=GL_FALSE;
            if(channels.find("G") != string::npos) g=GL_TRUE; else g=GL_FALSE;
            if(channels.find("B") != string::npos) b=GL_TRUE; else b=GL_FALSE;
            if(channels.find("A") != string::npos) a=GL_TRUE; else a=GL_FALSE;
            glColorMask(r,g,b,a);
            glDrawBuffers(types.size(), &types[0]); // Specify what to render an start acquiring
            //glClear(GL_COLOR_BUFFER_BIT);         // Clear the render targets
            vector_clear(types);
        }
        void _start(uint t1,string& c,bool f){
            vector<uint> t;
            t.push_back(m_Buffers.at(t1)->attatchment());
            _start(t,c,f);
        }
        void _start(uint t1,uint t2,string& c,bool f){
            vector<uint> t;
            t.push_back(m_Buffers.at(t1)->attatchment());
            t.push_back(m_Buffers.at(t2)->attatchment());
            _start(t,c,f);
        }
        void _start(uint t1,uint t2,uint t3,string& c,bool f){
            vector<uint> t;
            t.push_back(m_Buffers.at(t1)->attatchment());
            t.push_back(m_Buffers.at(t2)->attatchment());
            t.push_back(m_Buffers.at(t3)->attatchment());
            _start(t,c,f);
        }
        void _start(uint t1,uint t2,uint t3,uint t4,string& c,bool f){
            vector<uint> t;
            t.push_back(m_Buffers.at(t1)->attatchment());
            t.push_back(m_Buffers.at(t2)->attatchment());
            t.push_back(m_Buffers.at(t3)->attatchment());
            t.push_back(m_Buffers.at(t4)->attatchment());
            _start(t,c,f);
        }
        void _start(uint t1,uint t2,uint t3,uint t4,uint t5,string& c,bool f){
            vector<uint> t;
            t.push_back(m_Buffers.at(t1)->attatchment());
            t.push_back(m_Buffers.at(t2)->attatchment());
            t.push_back(m_Buffers.at(t3)->attatchment());
            t.push_back(m_Buffers.at(t4)->attatchment());
            t.push_back(m_Buffers.at(t5)->attatchment());
            _start(t,c,f);
        }
        void _start(uint t1,uint t2,uint t3,uint t4,uint t5,uint t6,string& c,bool f){
            vector<uint> t;
            t.push_back(m_Buffers.at(t1)->attatchment());
            t.push_back(m_Buffers.at(t2)->attatchment());
            t.push_back(m_Buffers.at(t3)->attatchment());
            t.push_back(m_Buffers.at(t4)->attatchment());
            t.push_back(m_Buffers.at(t5)->attatchment());
            t.push_back(m_Buffers.at(t6)->attatchment());
            _start(t,c,f);
        }
        void _stop(GLuint final_fbo, GLuint final_rbo){
            Renderer::bindFBO(final_fbo);
            Renderer::bindRBO(final_rbo); //probably dont even need this. or only implement this if final_rbo != 0
            glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
            glClear(GL_COLOR_BUFFER_BIT);
        }
};
GBuffer::GBuffer(uint width,uint height):m_i(new impl){
    m_i->_init(width,height);
}
GBuffer::~GBuffer(){
    m_i->_destruct();
}
void GBuffer::resize(uint width, uint height){
    m_i->_resize(width,height);
}
void GBuffer::start(vector<uint>& t,string c,bool mainFBO){m_i->_start(t,c,mainFBO);}
void GBuffer::start(uint t,string c,bool mainFBO){m_i->_start(t,c,mainFBO);}
void GBuffer::start(uint t,uint t1,string c,bool mainFBO){m_i->_start(t,t1,c,mainFBO);}
void GBuffer::start(uint t,uint t1,uint t2,string c,bool mainFBO){m_i->_start(t,t1,t2,c,mainFBO);}
void GBuffer::start(uint t,uint t1,uint t2,uint t3,string c,bool mainFBO){m_i->_start(t,t1,t2,t3,c,mainFBO);}
void GBuffer::start(uint t,uint t1,uint t2,uint t3,uint t4,string c,bool mainFBO){m_i->_start(t,t1,t2,t3,t4,c,mainFBO);}
void GBuffer::start(uint t,uint t1,uint t2,uint t3,uint t4,uint t5,string c,bool mainFBO){m_i->_start(t,t1,t2,t3,t4,t5,c,mainFBO);}
void GBuffer::stop(GLuint fbo, GLuint rbo){m_i->_stop(fbo,rbo);}
const unordered_map<uint,FramebufferTexture*>& GBuffer::getBuffers() const{ return m_i->m_Buffers; }
Texture* GBuffer::getTexture(uint t){ return m_i->m_Buffers.at(t)->texture();}
FramebufferTexture* GBuffer::getBuffer(uint t){ return m_i->m_Buffers.at(t); }
const GLuint& GBuffer::getMainFBO() const{ return m_i->m_FBO->address(); }
const GLuint& GBuffer::getSmallFBO() const{ return m_i->m_SmallFBO->address(); }
