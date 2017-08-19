#include "GBuffer.h"
#include "Engine_Resources.h"
#include "Engine_Window.h"
#include "Engine_Renderer.h"

#include <boost/tuple/tuple.hpp>

using namespace Engine;
using namespace std;

unordered_map<uint,boost::tuple<float,uint,uint,uint,uint>> _populateGBufferTypesInfo(){
    unordered_map<uint,boost::tuple<float,uint,uint,uint,uint>> m;
                                       //winSizeRatio   //internFormat                       //pxl_components                         //pxl_format
    m[GBufferType::Diffuse]  = boost::make_tuple(1.0f,  uint(ImageInternalFormat::RGB8),     uint(ImagePixelFormat::RGB),             uint(ImagePixelType::FLOAT),  uint(FramebufferAttatchment::Color_0));
    m[GBufferType::Normal]   = boost::make_tuple(1.0f,  uint(ImageInternalFormat::RGBA16F),  uint(ImagePixelFormat::RGBA),            uint(ImagePixelType::FLOAT),  uint(FramebufferAttatchment::Color_1));
    m[GBufferType::Misc]     = boost::make_tuple(1.0f,  uint(ImageInternalFormat::RGBA8),    uint(ImagePixelFormat::RGBA),            uint(ImagePixelType::FLOAT),  uint(FramebufferAttatchment::Color_2));
    m[GBufferType::Lighting] = boost::make_tuple(1.0f,  uint(ImageInternalFormat::RGB16F),   uint(ImagePixelFormat::RGB),             uint(ImagePixelType::FLOAT),  uint(FramebufferAttatchment::Color_3));
    m[GBufferType::Bloom]    = boost::make_tuple(0.5f,  uint(ImageInternalFormat::RGBA8),    uint(ImagePixelFormat::RGBA),            uint(ImagePixelType::FLOAT),  uint(FramebufferAttatchment::Color_0));
    m[GBufferType::GodRays]  = boost::make_tuple(0.5f,  uint(ImageInternalFormat::RGB8),     uint(ImagePixelFormat::RGB),             uint(ImagePixelType::FLOAT),  uint(FramebufferAttatchment::Color_1));
    m[GBufferType::Free2]    = boost::make_tuple(0.5f,  uint(ImageInternalFormat::RGBA8),    uint(ImagePixelFormat::RGBA),            uint(ImagePixelType::FLOAT),  uint(FramebufferAttatchment::Color_2));
    m[GBufferType::Depth]    = boost::make_tuple(1.0f,  uint(ImageInternalFormat::Depth16),  uint(ImagePixelFormat::DEPTH_COMPONENT), uint(ImagePixelType::FLOAT),  uint(FramebufferAttatchment::Depth));
    
    return m;
}
unordered_map<uint,boost::tuple<float,uint,uint,uint,uint>> GBUFFER_TYPE_DATA = _populateGBufferTypesInfo();

class TextureBuffer::impl final{
    public:
        FramebufferAttatchment::Attatchment m_BufferAttatchment;
        float m_Divisor;
        void _init(FramebufferAttatchment::Attatchment attatchment,float divisor,TextureBuffer* super){
            m_BufferAttatchment = attatchment;
            m_Divisor = divisor;
            glFramebufferTexture2D(GL_FRAMEBUFFER,FramebufferAttatchment::at(m_BufferAttatchment),super->type(),super->address(),0);
        }
        void _resize(uint width,uint height,TextureBuffer* super){
            super->decrementUseCount();
            super->unload();
            super->load();
            glFramebufferTexture2D(GL_FRAMEBUFFER,FramebufferAttatchment::at(m_BufferAttatchment),super->type(),super->address(),0);
        }
};
class GBuffer::impl final{
    public:
        GLuint m_FBO;       GLuint m_RBO;
        GLuint m_FBO_bloom; GLuint m_RBO_bloom;
        unordered_map<uint,boost::weak_ptr<TextureBuffer>> m_Buffers;
        uint m_Width; uint m_Height;
        bool _init(uint w,uint h){
            _destruct(); //just incase this method is called on resize, we want to delete any previous buffers

            m_Width = w; m_Height = h;

            glGenFramebuffers(1, &m_FBO);
            glGenRenderbuffers(1, &m_RBO);

            Renderer::bindFBO(m_FBO);
            Renderer::bindRBO(m_RBO);// Bind the depth buffer
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, m_Width, m_Height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_RBO);

            _constructFramebuffer("BUFFER_DIFFUSE", GBufferType::Diffuse, m_Width,m_Height);
            _constructFramebuffer("BUFFER_NORMAL",  GBufferType::Normal,  m_Width,m_Height);
            _constructFramebuffer("BUFFER_MISC",    GBufferType::Misc,    m_Width,m_Height);
            _constructFramebuffer("BUFFER_LIGHTING",GBufferType::Lighting,m_Width,m_Height);
            _constructFramebuffer("BUFFER_DEPTH",   GBufferType::Depth,   m_Width,m_Height);

            if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
                return false;
            }
            Renderer::unbindRBO(); //was moved to down here
            Renderer::unbindFBO();

            glGenFramebuffers(1, &m_FBO_bloom);
            glGenRenderbuffers(1, &m_RBO_bloom);

            Renderer::bindFBO(m_FBO_bloom);
            Renderer::bindRBO(m_RBO_bloom);// Bind the depth buffer
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, m_Width, m_Height);//replace GL_DEPTH_COMPONENT with GL_DEPTH_COMPONENT16 or GL_DEPTH_COMPONENT24 or GL_DEPTH_COMPONENT32
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_RBO_bloom);

            _constructFramebuffer("BUFFER_BLOOM",   GBufferType::Bloom,   m_Width,m_Height);
            _constructFramebuffer("BUFFER_FREE2",   GBufferType::Free2,   m_Width,m_Height);
            _constructFramebuffer("BUFFER_GODSRAYS",GBufferType::GodRays, m_Width,m_Height);

            if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
                return false;
            }
            Renderer::unbindRBO(); //was moved to down here
            Renderer::unbindFBO();
            return true;
        }
        void _constructFramebuffer(string n,uint t,uint w,uint h){
            boost::tuple<float,uint,uint,uint,uint>& i = GBUFFER_TYPE_DATA.at(t);
            TextureBuffer* tbo = new TextureBuffer(n,w,h,
                ImageInternalFormat::Format(i.get<1>()),ImagePixelFormat::Format(i.get<2>()),
                ImagePixelType::Type(i.get<3>()),FramebufferAttatchment::Attatchment(i.get<4>()),i.get<0>());
            boost::weak_ptr<TextureBuffer> ptr = boost::dynamic_pointer_cast<TextureBuffer>(Resources::getTexturePtr(tbo->name()));
            m_Buffers.emplace(t,ptr);
        }
        void _destruct(){
            for(auto buffer:m_Buffers){ buffer.second.reset(); }
            m_Buffers.clear();
            m_Width = m_Height = 0;

            glDeleteRenderbuffers(1, &m_RBO);
            glDeleteFramebuffers(1, &m_FBO);

            glDeleteRenderbuffers(1, &m_RBO_bloom);
            glDeleteFramebuffers(1, &m_FBO_bloom);
            Renderer::unbindFBO();
        }
        void _start(vector<uint>& types,string& channels,bool first_fbo){
            if(first_fbo){ Renderer::bindFBO(m_FBO); }
            else{ Renderer::bindFBO(m_FBO_bloom); }
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
            t.push_back(m_Buffers.at(t1).lock().get()->attatchment());
            _start(t,c,f);
        }
        void _start(uint t1,uint t2,string& c,bool f){
            vector<uint> t;
            t.push_back(m_Buffers.at(t1).lock().get()->attatchment());
            t.push_back(m_Buffers.at(t2).lock().get()->attatchment());
            _start(t,c,f);
        }
        void _start(uint t1,uint t2,uint t3,string& c,bool f){
            vector<uint> t;
            t.push_back(m_Buffers.at(t1).lock().get()->attatchment());
            t.push_back(m_Buffers.at(t2).lock().get()->attatchment());
            t.push_back(m_Buffers.at(t3).lock().get()->attatchment());
            _start(t,c,f);
        }
        void _start(uint t1,uint t2,uint t3,uint t4,string& c,bool f){
            vector<uint> t;
            t.push_back(m_Buffers.at(t1).lock().get()->attatchment());
            t.push_back(m_Buffers.at(t2).lock().get()->attatchment());
            t.push_back(m_Buffers.at(t3).lock().get()->attatchment());
            t.push_back(m_Buffers.at(t4).lock().get()->attatchment());
            _start(t,c,f);
        }
        void _start(uint t1,uint t2,uint t3,uint t4,uint t5,string& c,bool f){
            vector<uint> t;
            t.push_back(m_Buffers.at(t1).lock().get()->attatchment());
            t.push_back(m_Buffers.at(t2).lock().get()->attatchment());
            t.push_back(m_Buffers.at(t3).lock().get()->attatchment());
            t.push_back(m_Buffers.at(t4).lock().get()->attatchment());
            t.push_back(m_Buffers.at(t5).lock().get()->attatchment());
            _start(t,c,f);
        }
        void _start(uint t1,uint t2,uint t3,uint t4,uint t5,uint t6,string& c,bool f){
            vector<uint> t;
            t.push_back(m_Buffers.at(t1).lock().get()->attatchment());
            t.push_back(m_Buffers.at(t2).lock().get()->attatchment());
            t.push_back(m_Buffers.at(t3).lock().get()->attatchment());
            t.push_back(m_Buffers.at(t4).lock().get()->attatchment());
            t.push_back(m_Buffers.at(t5).lock().get()->attatchment());
            t.push_back(m_Buffers.at(t6).lock().get()->attatchment());
            _start(t,c,f);
        }
        void _stop(GLuint final_fbo, GLuint final_rbo){
            Renderer::bindFBO(final_fbo);
            Renderer::bindRBO(final_rbo); //probably dont even need this. or only implement this if final_rbo != 0
            glColorMask(GL_TRUE,GL_TRUE,GL_TRUE,GL_TRUE);
            glClear(GL_COLOR_BUFFER_BIT);
        }
};
TextureBuffer::TextureBuffer(std::string n,uint w,uint h,ImageInternalFormat::Format _if,ImagePixelFormat::Format pf,ImagePixelType::Type pt,FramebufferAttatchment::Attatchment a,float d):Texture(n,w,h,_if,pf,pt,GL_TEXTURE_2D,d),m_i(new impl){
    m_i->_init(a,d,this);
}
TextureBuffer::~TextureBuffer(){
}
const float TextureBuffer::divisor() const{ return m_i->m_Divisor; }
const int TextureBuffer::attatchment() const{ return FramebufferAttatchment::at(m_i->m_BufferAttatchment); }
GBuffer::GBuffer(uint width,uint height):m_i(new impl){ 
    m_i->_init(width,height);
}
GBuffer::~GBuffer(){
    m_i->_destruct();
}
void GBuffer::resize(uint width, uint height){
    stop();
    m_i->_init(width,height);
}
void GBuffer::start(vector<uint>& t,string c,bool mainFBO){m_i->_start(t,c,mainFBO);}
void GBuffer::start(uint t,string c,bool mainFBO){m_i->_start(t,c,mainFBO);}
void GBuffer::start(uint t,uint t1,string c,bool mainFBO){m_i->_start(t,t1,c,mainFBO);}
void GBuffer::start(uint t,uint t1,uint t2,string c,bool mainFBO){m_i->_start(t,t1,t2,c,mainFBO);}
void GBuffer::start(uint t,uint t1,uint t2,uint t3,string c,bool mainFBO){m_i->_start(t,t1,t2,t3,c,mainFBO);}
void GBuffer::start(uint t,uint t1,uint t2,uint t3,uint t4,string c,bool mainFBO){m_i->_start(t,t1,t2,t3,t4,c,mainFBO);}
void GBuffer::start(uint t,uint t1,uint t2,uint t3,uint t4,uint t5,string c,bool mainFBO){m_i->_start(t,t1,t2,t3,t4,t5,c,mainFBO);}
void GBuffer::stop(GLuint fbo, GLuint rbo){m_i->_stop(fbo,rbo);}
const unordered_map<uint,boost::weak_ptr<TextureBuffer>>& GBuffer::getBuffers() const{ return m_i->m_Buffers; }
Texture* GBuffer::getTexture(uint t){ return m_i->m_Buffers.at(t).lock().get();}
TextureBuffer* GBuffer::getBuffer(uint t){ return m_i->m_Buffers.at(t).lock().get();}
const GLuint& GBuffer::getMainFBO() const{ return m_i->m_FBO; }
const GLuint& GBuffer::getSmallFBO() const{ return m_i->m_FBO_bloom; }
