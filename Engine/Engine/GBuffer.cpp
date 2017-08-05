#include "GBuffer.h"
#include "Engine_Resources.h"
#include "Engine_Window.h"
#include "Engine_Renderer.h"

#include <boost/tuple/tuple.hpp>

using namespace Engine;
using namespace std;

unordered_map<uint,boost::tuple<float,GLuint,GLuint,GLuint,GLuint>> _populateGBufferTypesInfo(){
    unordered_map<uint,boost::tuple<float,GLuint,GLuint,GLuint,GLuint>> m;
                                                 //winSizeRatio              //type             //components       //component format //attatchment
    m[GBufferType::Diffuse]  = boost::make_tuple(1.0f,  GL_RGB8,             GL_RGB,            GL_UNSIGNED_BYTE,  GL_COLOR_ATTACHMENT0);
    m[GBufferType::Normal]   = boost::make_tuple(1.0f,  GL_RGBA16F,          GL_RGBA,           GL_FLOAT,          GL_COLOR_ATTACHMENT1);
    m[GBufferType::Misc]     = boost::make_tuple(1.0f,  GL_RGBA8,            GL_RGBA,           GL_UNSIGNED_BYTE,  GL_COLOR_ATTACHMENT2);
    m[GBufferType::Lighting] = boost::make_tuple(1.0f,  GL_RGB16F,           GL_RGB,            GL_FLOAT,          GL_COLOR_ATTACHMENT3);
    m[GBufferType::Bloom]    = boost::make_tuple(0.5f,  GL_RGBA8,            GL_RGBA,           GL_UNSIGNED_BYTE,  GL_COLOR_ATTACHMENT0);
    m[GBufferType::GodRays]  = boost::make_tuple(0.5f,  GL_RGB8,             GL_RGB,            GL_UNSIGNED_BYTE,  GL_COLOR_ATTACHMENT1);
    m[GBufferType::Free2]    = boost::make_tuple(0.5f,  GL_RGBA8,            GL_RGBA,           GL_UNSIGNED_BYTE,  GL_COLOR_ATTACHMENT2);
    m[GBufferType::Depth]    = boost::make_tuple(1.0f,  GL_DEPTH_COMPONENT16,GL_DEPTH_COMPONENT,GL_FLOAT,          GL_DEPTH_ATTACHMENT);
    
    return m;
}
unordered_map<uint,boost::tuple<float,GLuint,GLuint,GLuint,GLuint>> GBUFFER_TYPE_DATA = _populateGBufferTypesInfo();

class TextureBuffer::impl final{
    public:
        GLuint m_BufferInternalFormat;
        GLuint m_BufferFormat;
        GLuint m_BufferType; 
        GLuint m_BufferAttatchment;
        float m_Divisor;
        void _init(GLuint internalformat,GLuint format,GLuint type,GLuint attatchment,uint width,uint height,float divisor,TextureBuffer* super){
            m_BufferInternalFormat = internalformat;
            m_BufferFormat = format;
            m_BufferType = type;
            m_BufferAttatchment = attatchment;
            m_Divisor = divisor;
            _resize(width,height,super);
        }
        void _resize(uint width,uint height,TextureBuffer* super){
            super->_constructAsFramebuffer(width,height,m_Divisor,m_BufferInternalFormat,m_BufferFormat,m_BufferType,m_BufferAttatchment);
        }
};
class GBuffer::impl final{
    public:
        GLuint m_fbo;       GLuint m_depth;
        GLuint m_fbo_bloom; GLuint m_depth_fake;
        unordered_map<uint,boost::weak_ptr<TextureBuffer>> m_Buffers;
        uint m_Width; uint m_Height;
        bool _init(uint w,uint h){
            _destruct(); //just incase this method is called on resize, we want to delete any previous buffers

            m_Width = w; m_Height = h;

            glGenFramebuffers(1, &m_fbo);
            Renderer::bindFBO(m_fbo);

            glGenRenderbuffers(1, &m_depth);
            glBindRenderbuffer(GL_RENDERBUFFER, m_depth);// Bind the depth buffer
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_Width, m_Height);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth);

            _constructFramebuffer("BUFFER_DIFFUSE", GBufferType::Diffuse, m_Width,m_Height);
            _constructFramebuffer("BUFFER_NORMAL",  GBufferType::Normal,  m_Width,m_Height);
            _constructFramebuffer("BUFFER_MISC",    GBufferType::Misc,    m_Width,m_Height);
            _constructFramebuffer("BUFFER_LIGHTING",GBufferType::Lighting,m_Width,m_Height);
            _constructFramebuffer("BUFFER_DEPTH",   GBufferType::Depth,   m_Width,m_Height);

            if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
                return false;
            }
            Renderer::bindFBO(0);

            glGenFramebuffers(1, &m_fbo_bloom);
            Renderer::bindFBO(m_fbo_bloom);

            glGenRenderbuffers(1, &m_depth_fake);
            glBindRenderbuffer(GL_RENDERBUFFER, m_depth_fake);// Bind the depth buffer
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_Width, m_Height);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth_fake);

            _constructFramebuffer("BUFFER_BLOOM",   GBufferType::Bloom,   m_Width,m_Height);
            _constructFramebuffer("BUFFER_FREE2",   GBufferType::Free2,   m_Width,m_Height);
            _constructFramebuffer("BUFFER_GODSRAYS",GBufferType::GodRays, m_Width,m_Height);

            if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
                return false;
            }
            Renderer::bindFBO(0);
            return true;
        }
        void _constructFramebuffer(string n,uint t,uint w,uint h){
            boost::tuple<float,GLuint,GLuint,GLuint,GLuint>& i = GBUFFER_TYPE_DATA.at(t);
            TextureBuffer* tbo = nullptr;
            tbo = new TextureBuffer(n,i.get<1>(),i.get<2>(),i.get<3>(),i.get<4>(),w,h,i.get<0>());
            boost::weak_ptr<TextureBuffer> ptr = boost::dynamic_pointer_cast<TextureBuffer>(Resources::getTexturePtr(tbo->name()));
            m_Buffers.emplace(t,ptr);
        }
        void _destruct(){
            for(auto buffer:m_Buffers){ buffer.second.reset(); }
            m_Buffers.clear();
            m_Width = m_Height = 0;

            Renderer::bindFBO(m_fbo);
            glDeleteRenderbuffers(1, &m_depth);
            glDeleteFramebuffers(1, &m_fbo);
            Renderer::bindFBO(0);

            Renderer::bindFBO(m_fbo_bloom);
            glDeleteRenderbuffers(1, &m_depth_fake);
            glDeleteFramebuffers(1, &m_fbo_bloom);
            Renderer::bindFBO(0);
        }
        void _start(vector<uint>& types,string& channels,bool first_fbo){
            if(first_fbo){
                Renderer::bindReadFBO(m_fbo);
                Renderer::bindDrawFBO(m_fbo);
            }
            else{
                Renderer::bindReadFBO(m_fbo_bloom);
                Renderer::bindDrawFBO(m_fbo_bloom);
            }
            GLboolean r,g,b,a;
            if(channels.find("R") != string::npos) r=GL_TRUE; else r=GL_FALSE;
            if(channels.find("G") != string::npos) g=GL_TRUE; else g=GL_FALSE;
            if(channels.find("B") != string::npos) b=GL_TRUE; else b=GL_FALSE;
            if(channels.find("A") != string::npos) a=GL_TRUE; else a=GL_FALSE;
            glColorMask(r,g,b,a);
            glDrawBuffers(types.size(), &types[0]); // Specify what to render an start acquiring
            glClear(GL_COLOR_BUFFER_BIT);           // Clear the render targets
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
        void _stop(){
            Renderer::bindFBO(0);
            glColorMask(1,1,1,1);
            glClear(GL_COLOR_BUFFER_BIT);
        }
};
TextureBuffer::TextureBuffer(string name,GLuint internalformat, GLuint format, GLuint type, GLuint attatchment,uint width,uint height,float divisor):Texture(name,width,height),m_i(new impl){
    m_i->_init(internalformat,format,type,attatchment,width,height,divisor,this);
}
TextureBuffer::~TextureBuffer(){
}
const float TextureBuffer::divisor() const{ return m_i->m_Divisor; }
const int TextureBuffer::attatchment() const{ return m_i->m_BufferAttatchment; }
GBuffer::GBuffer(uint width,uint height):m_i(new impl){ 
    m_i->_init(width,height);
}
GBuffer::~GBuffer(){
    m_i->_destruct();
}
void GBuffer::resize(uint width, uint height){
    m_i->_stop();
    m_i->_init(width,height);
}
void GBuffer::start(vector<uint>& types,string channels,bool first_fbo){m_i->_start(types,channels,first_fbo);}
void GBuffer::start(uint type,string channels,bool first_fbo){m_i->_start(type,channels,first_fbo);}
void GBuffer::start(uint type,uint type1,string channels,bool first_fbo){m_i->_start(type,type1,channels,first_fbo);}
void GBuffer::start(uint type,uint type1,uint type2,string channels,bool first_fbo){m_i->_start(type,type1,type2,channels,first_fbo);}
void GBuffer::start(uint type,uint type1,uint type2,uint type3,string channels,bool first_fbo){m_i->_start(type,type1,type2,type3,channels,first_fbo);}
void GBuffer::start(uint type,uint type1,uint type2,uint type3,uint type4,string channels,bool first_fbo){m_i->_start(type,type1,type2,type3,type4,channels,first_fbo);}
void GBuffer::start(uint type,uint type1,uint type2,uint type3,uint type4,uint type5,string channels,bool first_fbo){m_i->_start(type,type1,type2,type3,type4,type5,channels,first_fbo);}
void GBuffer::stop(){m_i->_stop();}
const unordered_map<uint,boost::weak_ptr<TextureBuffer>>& GBuffer::getBuffers() const{ return m_i->m_Buffers; }
Texture* GBuffer::getTexture(uint type){ return m_i->m_Buffers.at(type).lock().get();}
const GLuint& GBuffer::getMainFBO() const{ return m_i->m_fbo; }
const GLuint& GBuffer::getSmallFBO() const{ return m_i->m_fbo_bloom; }
