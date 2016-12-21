#include <unordered_map>
#include <GL/glew.h>
#include <GL/GL.h>
#include "GBuffer.h"

class TextureBuffer::impl final{
    public:
        int m_BufferInternalFormat;
        int m_BufferFormat;
        int m_BufferType;
        int m_BufferAttatchment;

		float m_SizeScalar;
        void _init(int internalformat, int format, int type, int attatchment,uint width,uint height,float sizeScalar,TextureBuffer* super){
            m_BufferInternalFormat = internalformat;
            m_BufferFormat = format;
            m_BufferType = type;
            m_BufferAttatchment = attatchment;
			m_SizeScalar = sizeScalar;
			glGenTextures(1, &(super->address()));
			_resize(width,height,super);
        }
        void _resize(uint width,uint height,TextureBuffer* super){
			super->_constructAsFramebuffer(width,height,m_SizeScalar,m_BufferInternalFormat,m_BufferFormat,m_BufferType,m_BufferAttatchment);
        }
};
class GBuffer::impl final{
    public:
        GLuint m_fbo;
        GLuint m_depth;

        GLuint m_fbo_bloom;
        GLuint m_depth_fake;

        std::unordered_map<uint,TextureBuffer*> m_Buffers;
        uint m_width; uint m_height;
        void _init(uint w,uint h){
            m_width = w; m_height = h;

            glGenFramebuffers(1, &m_fbo);
            glGenRenderbuffers(1, &m_depth);
            glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
            glBindRenderbuffer(GL_RENDERBUFFER, m_depth);// Bind the depth buffer
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_width, m_height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth);

			TextureBuffer* tbo = new TextureBuffer("BUFFER_DIFFUSE",GBUFFER_TYPES[BUFFER_TYPE_DIFFUSE],GBUFFER_PIXEL_TYPES[BUFFER_TYPE_DIFFUSE],GBUFFER_FLOAT_TYPES[BUFFER_TYPE_DIFFUSE],GBUFFER_ATTACHMENT_TYPES[BUFFER_TYPE_DIFFUSE],m_width,m_height,GBUFFER_DIVISIBLES[BUFFER_TYPE_DIFFUSE]);
            m_Buffers[BUFFER_TYPE_DIFFUSE] = tbo;
			tbo = new TextureBuffer("BUFFER_NORMAL",GBUFFER_TYPES[BUFFER_TYPE_NORMAL],GBUFFER_PIXEL_TYPES[BUFFER_TYPE_NORMAL],GBUFFER_FLOAT_TYPES[BUFFER_TYPE_NORMAL],GBUFFER_ATTACHMENT_TYPES[BUFFER_TYPE_NORMAL],m_width,m_height,GBUFFER_DIVISIBLES[BUFFER_TYPE_NORMAL]);
            m_Buffers[BUFFER_TYPE_NORMAL] = tbo;
			tbo = new TextureBuffer("BUFFER_MISC",GBUFFER_TYPES[BUFFER_TYPE_MISC],GBUFFER_PIXEL_TYPES[BUFFER_TYPE_MISC],GBUFFER_FLOAT_TYPES[BUFFER_TYPE_MISC],GBUFFER_ATTACHMENT_TYPES[BUFFER_TYPE_MISC],m_width,m_height,GBUFFER_DIVISIBLES[BUFFER_TYPE_MISC]);
            m_Buffers[BUFFER_TYPE_MISC] = tbo;
			tbo = new TextureBuffer("BUFFER_POSITION",GBUFFER_TYPES[BUFFER_TYPE_POSITION],GBUFFER_PIXEL_TYPES[BUFFER_TYPE_POSITION],GBUFFER_FLOAT_TYPES[BUFFER_TYPE_POSITION],GBUFFER_ATTACHMENT_TYPES[BUFFER_TYPE_POSITION],m_width,m_height,GBUFFER_DIVISIBLES[BUFFER_TYPE_POSITION]);
            m_Buffers[BUFFER_TYPE_POSITION] = tbo;
			tbo = new TextureBuffer("BUFFER_LIGHTING",GBUFFER_TYPES[BUFFER_TYPE_LIGHTING],GBUFFER_PIXEL_TYPES[BUFFER_TYPE_LIGHTING],GBUFFER_FLOAT_TYPES[BUFFER_TYPE_LIGHTING],GBUFFER_ATTACHMENT_TYPES[BUFFER_TYPE_LIGHTING],m_width,m_height,GBUFFER_DIVISIBLES[BUFFER_TYPE_LIGHTING]);
            m_Buffers[BUFFER_TYPE_LIGHTING] = tbo;
			tbo = new TextureBuffer("BUFFER_FREE1",GBUFFER_TYPES[BUFFER_TYPE_FREE1],GBUFFER_PIXEL_TYPES[BUFFER_TYPE_FREE1],GBUFFER_FLOAT_TYPES[BUFFER_TYPE_FREE1],GBUFFER_ATTACHMENT_TYPES[BUFFER_TYPE_FREE1],m_width,m_height,GBUFFER_DIVISIBLES[BUFFER_TYPE_FREE1]);
            m_Buffers[BUFFER_TYPE_FREE1] = tbo;
			tbo = new TextureBuffer("BUFFER_DEPTH",GBUFFER_TYPES[BUFFER_TYPE_DEPTH],GBUFFER_PIXEL_TYPES[BUFFER_TYPE_DEPTH],GBUFFER_FLOAT_TYPES[BUFFER_TYPE_DEPTH],GBUFFER_ATTACHMENT_TYPES[BUFFER_TYPE_DEPTH],m_width,m_height,GBUFFER_DIVISIBLES[BUFFER_TYPE_DEPTH]);
            m_Buffers[BUFFER_TYPE_DEPTH] = tbo;

            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            glGenFramebuffers(1, &m_fbo_bloom);
            glGenRenderbuffers(1, &m_depth_fake);
            glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_bloom);
            glBindRenderbuffer(GL_RENDERBUFFER, m_depth_fake);// Bind the depth buffer
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_width, m_height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth_fake);

			tbo = new TextureBuffer("BUFFER_TYPE_BLOOM",GBUFFER_TYPES[BUFFER_TYPE_BLOOM],GBUFFER_PIXEL_TYPES[BUFFER_TYPE_BLOOM],GBUFFER_FLOAT_TYPES[BUFFER_TYPE_BLOOM],GBUFFER_ATTACHMENT_TYPES[BUFFER_TYPE_BLOOM],m_width,m_height,GBUFFER_DIVISIBLES[BUFFER_TYPE_BLOOM]);
            m_Buffers[BUFFER_TYPE_BLOOM] = tbo;
			tbo = new TextureBuffer("BUFFER_TYPE_FREE2",GBUFFER_TYPES[BUFFER_TYPE_FREE2],GBUFFER_PIXEL_TYPES[BUFFER_TYPE_FREE2],GBUFFER_FLOAT_TYPES[BUFFER_TYPE_FREE2],GBUFFER_ATTACHMENT_TYPES[BUFFER_TYPE_FREE2],m_width,m_height,GBUFFER_DIVISIBLES[BUFFER_TYPE_FREE2]);
            m_Buffers[BUFFER_TYPE_FREE2] = tbo;
			tbo = new TextureBuffer("BUFFER_TYPE_GODSRAYS",GBUFFER_TYPES[BUFFER_TYPE_GODSRAYS],GBUFFER_PIXEL_TYPES[BUFFER_TYPE_GODSRAYS],GBUFFER_FLOAT_TYPES[BUFFER_TYPE_GODSRAYS],GBUFFER_ATTACHMENT_TYPES[BUFFER_TYPE_GODSRAYS],m_width,m_height,GBUFFER_DIVISIBLES[BUFFER_TYPE_GODSRAYS]);
            m_Buffers[BUFFER_TYPE_GODSRAYS] = tbo;

			glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        void _destruct(){
            glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
            glDeleteRenderbuffers(1, &m_depth);
            glDeleteFramebuffers(1, &m_fbo);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_bloom);
            glDeleteRenderbuffers(1, &m_depth_fake);
            glDeleteFramebuffers(1, &m_fbo_bloom);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        void _resizeBaseBuffer(uint w,uint h){
            m_width  = w; m_height = h;
            glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
            glBindRenderbuffer(GL_RENDERBUFFER, m_depth);// Bind the depth buffer
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_bloom);
            glBindRenderbuffer(GL_RENDERBUFFER, m_depth_fake);// Bind the depth buffer
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth_fake);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        void _start(std::vector<uint>& types,std::string& channels,bool first_fbo){
			if(first_fbo){
				glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);      // Bind our FBO and set the viewport to the proper size
			}
			else{
				glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_bloom);// Bind our FBO and set the viewport to the proper size
			}
            bool r,g,b,a;
            if(channels.find("R") != std::string::npos) r=true; else r=false;
            if(channels.find("G") != std::string::npos) g=true; else g=false;
            if(channels.find("B") != std::string::npos) b=true; else b=false;
            if(channels.find("A") != std::string::npos) a=true; else a=false;
            glColorMask(r,g,b,a);
			glEnable(GL_TEXTURE_2D);
            glActiveTexture(GL_TEXTURE0);

            glDrawBuffers(types.size(), &types[0]);        // Specify what to render an start acquiring
            glClear(GL_COLOR_BUFFER_BIT);                  // Clear the render targets
        }
        void _start(uint type,std::string& channels,bool first_fbo){
            std::vector<uint> types;
            types.push_back(m_Buffers[type]->attatchment());
            _start(types,channels,first_fbo);
        }
        void _start(uint type,uint type1,std::string& channels,bool first_fbo){
            std::vector<uint> types;
            types.push_back(m_Buffers[type]->attatchment());
            types.push_back(m_Buffers[type1]->attatchment());
            _start(types,channels,first_fbo);
        }
        void _start(uint type,uint type1,uint type2,std::string& channels,bool first_fbo){
            std::vector<uint> types;
            types.push_back(m_Buffers[type]->attatchment());
            types.push_back(m_Buffers[type1]->attatchment());
            types.push_back(m_Buffers[type2]->attatchment());
            _start(types,channels,first_fbo);
        }
        void _start(uint type,uint type1,uint type2,uint type3,std::string& channels,bool first_fbo){
            std::vector<uint> types;
            types.push_back(m_Buffers[type]->attatchment());
            types.push_back(m_Buffers[type1]->attatchment());
            types.push_back(m_Buffers[type2]->attatchment());
            types.push_back(m_Buffers[type3]->attatchment());
            _start(types,channels,first_fbo);
        }
        void _start(uint type,uint type1,uint type2,uint type3,uint type4,std::string& channels,bool first_fbo){
            std::vector<uint> types;
            types.push_back(m_Buffers[type]->attatchment());
            types.push_back(m_Buffers[type1]->attatchment());
            types.push_back(m_Buffers[type2]->attatchment());
            types.push_back(m_Buffers[type3]->attatchment());
            types.push_back(m_Buffers[type4]->attatchment());
            _start(types,channels,first_fbo);
        }
        void _start(uint type,uint type1,uint type2,uint type3,uint type4,uint type5,std::string& channels,bool first_fbo){
            std::vector<uint> types;
            types.push_back(m_Buffers[type]->attatchment());
            types.push_back(m_Buffers[type1]->attatchment());
            types.push_back(m_Buffers[type2]->attatchment());
            types.push_back(m_Buffers[type3]->attatchment());
            types.push_back(m_Buffers[type4]->attatchment());
            types.push_back(m_Buffers[type5]->attatchment());
            _start(types,channels,first_fbo);
        }
        void _stop(){
            // Stop acquiring and unbind the FBO
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glColorMask(1,1,1,1);
            // Clear the render targets
            glClear(GL_COLOR_BUFFER_BIT);
        }
};
TextureBuffer::TextureBuffer(std::string name,int internalformat, int format, int type, int attatchment,uint width,uint height,float sizeScalar):Texture(name,width,height),m_i(new impl()){
    m_i->_init(internalformat,format,type,attatchment,width,height,sizeScalar,this);
}
TextureBuffer::~TextureBuffer(){
}
void TextureBuffer::resize(uint width,uint height){
    m_i->_resize(width,height,this);
}
float TextureBuffer::sizeScalar() const{
	return m_i->m_SizeScalar;
}
int TextureBuffer::attatchment() const{
    return m_i->m_BufferAttatchment;
}

//Create the FBO render texture initializing all the stuff that we need
GBuffer::GBuffer(uint width,uint height):m_i(new impl()){	
    m_i->_init(width,height);
}
GBuffer::~GBuffer(){
    m_i->_destruct();
}
void GBuffer::resizeBaseBuffer(uint width,uint height){
    m_i->_resizeBaseBuffer(width,height);
}
void GBuffer::resizeBuffer(uint buffer,uint width,uint height){ 
    m_i->m_Buffers[buffer]->resize(width,height); 
}
void GBuffer::start(std::vector<uint>& types,std::string channels,bool first_fbo){
    m_i->_start(types,channels,first_fbo);
}
void GBuffer::start(uint type,std::string channels,bool first_fbo){
    m_i->_start(type,channels,first_fbo);
}
void GBuffer::start(uint type,uint type1,std::string channels,bool first_fbo){
    m_i->_start(type,type1,channels,first_fbo);
}
void GBuffer::start(uint type,uint type1,uint type2,std::string channels,bool first_fbo){
    m_i->_start(type,type1,type2,channels,first_fbo);
}
void GBuffer::start(uint type,uint type1,uint type2,uint type3,std::string channels,bool first_fbo){
    m_i->_start(type,type1,type2,type3,channels,first_fbo);
}
void GBuffer::start(uint type,uint type1,uint type2,uint type3,uint type4,std::string channels,bool first_fbo){
    m_i->_start(type,type1,type2,type3,type4,channels,first_fbo);
}
void GBuffer::start(uint type,uint type1,uint type2,uint type3,uint type4,uint type5,std::string channels,bool first_fbo){
    m_i->_start(type,type1,type2,type3,type4,type5,channels,first_fbo);
}
void GBuffer::stop(){	
    m_i->_stop();
}
std::unordered_map<uint,TextureBuffer*> GBuffer::getBuffers(){ 
    return m_i->m_Buffers; 
}
Texture* GBuffer::getTexture(uint type){ 
	return m_i->m_Buffers[type];
}