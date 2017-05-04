#include "GBuffer.h"
#include "Engine_Resources.h"
#include "Engine_Window.h"
#include "Engine_Renderer.h"

using namespace Engine;

class TextureBuffer::impl final{
    public:
        int m_BufferInternalFormat; int m_BufferFormat; int m_BufferType; int m_BufferAttatchment;
        float m_Divisor;
        void _init(int internalformat, int format, int type, int attatchment,uint width,uint height,float divisor,TextureBuffer* super){
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
        std::unordered_map<uint,boost::weak_ptr<TextureBuffer>> m_Buffers;
        uint m_Width; uint m_Height;
        void _init(uint w,uint h){
			_destruct(); //just incase this method is called on resize, we want to delete any previous buffers

            m_Width = w; m_Height = h;

            glGenFramebuffers(1, &m_fbo);
			Renderer::bindFBO(m_fbo);

			glGenRenderbuffers(1, &m_depth);
            glBindRenderbuffer(GL_RENDERBUFFER, m_depth);// Bind the depth buffer
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_Width, m_Height);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth);

            _constructFramebuffer("BUFFER_DIFFUSE",BUFFER_TYPE_DIFFUSE,m_Width,m_Height);
            _constructFramebuffer("BUFFER_NORMAL",BUFFER_TYPE_NORMAL,m_Width,m_Height);
            _constructFramebuffer("BUFFER_MISC",BUFFER_TYPE_MISC,m_Width,m_Height);
            _constructFramebuffer("BUFFER_LIGHTING",BUFFER_TYPE_LIGHTING,m_Width,m_Height);
            _constructFramebuffer("BUFFER_DEPTH",BUFFER_TYPE_DEPTH,m_Width,m_Height);

			if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
			}
			Renderer::bindFBO(0);

            glGenFramebuffers(1, &m_fbo_bloom);
			Renderer::bindFBO(m_fbo_bloom);

            glGenRenderbuffers(1, &m_depth_fake);
            glBindRenderbuffer(GL_RENDERBUFFER, m_depth_fake);// Bind the depth buffer
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_Width, m_Height);
			glBindRenderbuffer(GL_RENDERBUFFER, 0);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth_fake);

            _constructFramebuffer("BUFFER_BLOOM",BUFFER_TYPE_BLOOM,m_Width,m_Height);
            _constructFramebuffer("BUFFER_FREE2",BUFFER_TYPE_FREE2,m_Width,m_Height);
            _constructFramebuffer("BUFFER_GODSRAYS",BUFFER_TYPE_GODSRAYS,m_Width,m_Height);

			if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE){
			}

            Renderer::bindFBO(0);
        }
        void _constructFramebuffer(std::string n,uint t,uint w,uint h){
            TextureBuffer* tbo = nullptr;
			tbo = new TextureBuffer(n,GBUFFER_TYPES[t],GBUFFER_PIXEL_TYPES[t],GBUFFER_FLOAT_TYPES[t],GBUFFER_ATTACHMENT_TYPES[t],w,h,GBUFFER_DIVISIBLES[t]);
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
        void _start(std::vector<uint>& types,std::string& channels,bool first_fbo){
            if(first_fbo){
				Renderer::bindReadFBO(m_fbo);
				Renderer::bindDrawFBO(m_fbo);
			}
			else{
				Renderer::bindReadFBO(m_fbo_bloom);
				Renderer::bindDrawFBO(m_fbo_bloom);
			}
            bool r,g,b,a;
            if(channels.find("R") != std::string::npos) r=true; else r=false;
            if(channels.find("G") != std::string::npos) g=true; else g=false;
            if(channels.find("B") != std::string::npos) b=true; else b=false;
            if(channels.find("A") != std::string::npos) a=true; else a=false;
            glColorMask(r,g,b,a);
            glDrawBuffers(types.size(), &types[0]);        // Specify what to render an start acquiring
            glClear(GL_COLOR_BUFFER_BIT);                  // Clear the render targets
        }
        void _start(uint t1,std::string& c,bool f){
            std::vector<uint> t;
            t.push_back(m_Buffers.at(t1).lock().get()->attatchment());
            _start(t,c,f);
        }
        void _start(uint t1,uint t2,std::string& c,bool f){
            std::vector<uint> t;
            t.push_back(m_Buffers.at(t1).lock().get()->attatchment());
            t.push_back(m_Buffers.at(t2).lock().get()->attatchment());
            _start(t,c,f);
        }
        void _start(uint t1,uint t2,uint t3,std::string& c,bool f){
            std::vector<uint> t;
            t.push_back(m_Buffers.at(t1).lock().get()->attatchment());
            t.push_back(m_Buffers.at(t2).lock().get()->attatchment());
            t.push_back(m_Buffers.at(t3).lock().get()->attatchment());
            _start(t,c,f);
        }
        void _start(uint t1,uint t2,uint t3,uint t4,std::string& c,bool f){
            std::vector<uint> t;
            t.push_back(m_Buffers.at(t1).lock().get()->attatchment());
            t.push_back(m_Buffers.at(t2).lock().get()->attatchment());
            t.push_back(m_Buffers.at(t3).lock().get()->attatchment());
            t.push_back(m_Buffers.at(t4).lock().get()->attatchment());
            _start(t,c,f);
        }
        void _start(uint t1,uint t2,uint t3,uint t4,uint t5,std::string& c,bool f){
            std::vector<uint> t;
            t.push_back(m_Buffers.at(t1).lock().get()->attatchment());
            t.push_back(m_Buffers.at(t2).lock().get()->attatchment());
            t.push_back(m_Buffers.at(t3).lock().get()->attatchment());
            t.push_back(m_Buffers.at(t4).lock().get()->attatchment());
            t.push_back(m_Buffers.at(t5).lock().get()->attatchment());
            _start(t,c,f);
        }
        void _start(uint t1,uint t2,uint t3,uint t4,uint t5,uint t6,std::string& c,bool f){
            std::vector<uint> t;
            t.push_back(m_Buffers.at(t1).lock().get()->attatchment());
            t.push_back(m_Buffers.at(t2).lock().get()->attatchment());
            t.push_back(m_Buffers.at(t3).lock().get()->attatchment());
            t.push_back(m_Buffers.at(t4).lock().get()->attatchment());
            t.push_back(m_Buffers.at(t5).lock().get()->attatchment());
            t.push_back(m_Buffers.at(t6).lock().get()->attatchment());
            _start(t,c,f);
        }
        void _stop(){
            //glBindFramebuffer(GL_FRAMEBUFFER, 0);
			Renderer::bindFBO(0);
            glColorMask(1,1,1,1);
            glClear(GL_COLOR_BUFFER_BIT);
        }
};
TextureBuffer::TextureBuffer(std::string name,int internalformat, int format, int type, int attatchment,uint width,uint height,float divisor):Texture(name,width,height),m_i(new impl){
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
void GBuffer::start(std::vector<uint>& types,std::string channels,bool first_fbo){m_i->_start(types,channels,first_fbo);}
void GBuffer::start(uint type,std::string channels,bool first_fbo){m_i->_start(type,channels,first_fbo);}
void GBuffer::start(uint type,uint type1,std::string channels,bool first_fbo){m_i->_start(type,type1,channels,first_fbo);}
void GBuffer::start(uint type,uint type1,uint type2,std::string channels,bool first_fbo){m_i->_start(type,type1,type2,channels,first_fbo);}
void GBuffer::start(uint type,uint type1,uint type2,uint type3,std::string channels,bool first_fbo){m_i->_start(type,type1,type2,type3,channels,first_fbo);}
void GBuffer::start(uint type,uint type1,uint type2,uint type3,uint type4,std::string channels,bool first_fbo){m_i->_start(type,type1,type2,type3,type4,channels,first_fbo);}
void GBuffer::start(uint type,uint type1,uint type2,uint type3,uint type4,uint type5,std::string channels,bool first_fbo){m_i->_start(type,type1,type2,type3,type4,type5,channels,first_fbo);}
void GBuffer::stop(){m_i->_stop();}
const std::unordered_map<uint,boost::weak_ptr<TextureBuffer>>& GBuffer::getBuffers() const{ return m_i->m_Buffers; }
Texture* GBuffer::getTexture(uint type){ return m_i->m_Buffers.at(type).lock().get();}
GLuint& GBuffer::getMainFBO(){ return m_i->m_fbo; }
GLuint& GBuffer::getSmallFBO(){ return m_i->m_fbo_bloom; }