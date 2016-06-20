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

        uint m_width; uint m_height;    
        GLuint m_Texture;

        void _init(int internalformat, int format, int type, int attatchment,uint width,uint height,float sizeScalar){
            m_BufferInternalFormat = internalformat;
            m_BufferFormat = format;
            m_BufferType = type;
            m_BufferAttatchment = attatchment;
			m_SizeScalar = sizeScalar;
            glGenTextures(1, &m_Texture);
			_resize(width,height);
        }
        void _destruct(){
            glDeleteTextures(1, &m_Texture);
        }
        void _resize(uint width,uint height){
            m_width = width; m_height = height;
            glBindTexture(GL_TEXTURE_2D, m_Texture);
            glTexImage2D(GL_TEXTURE_2D, 0, m_BufferInternalFormat, m_width*m_SizeScalar, m_height*m_SizeScalar, 0, m_BufferFormat, m_BufferType, 0);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glFramebufferTexture2D(GL_FRAMEBUFFER, m_BufferAttatchment, GL_TEXTURE_2D, m_Texture, 0);
        }
};
class GBuffer::impl final{
    public:
        GLuint m_fbo;
        GLuint m_depth;
        std::unordered_map<uint,TextureBuffer*> m_Buffers;
        uint m_width; uint m_height;
        void _init(uint width,uint height){
            m_width  = width; m_height = height;
            glGenFramebuffers(1, &m_fbo);
            glGenRenderbuffers(1, &m_depth);
            glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
            // Bind the depth buffer
            glBindRenderbuffer(GL_RENDERBUFFER, m_depth);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_width, m_height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth);

            for(uint i = 0; i < BUFFER_TYPE_NUMBER; i++){
                TextureBuffer* tbo = new TextureBuffer(
					GBUFFER_TYPES[i],
					GBUFFER_PIXEL_TYPES[i],
					GBUFFER_FLOAT_TYPES[i],
					GBUFFER_ATTACHMENT_TYPES[i],
					m_width,
					m_height,
					GBUFFER_DIVISIBLES[i]);
                m_Buffers[i] = tbo;
            }
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        void _destruct(){
            glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
            glDeleteRenderbuffers(1, &m_depth);
            glDeleteFramebuffers(1, &m_fbo);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            for(auto buffer:m_Buffers){
                delete(buffer.second);
            }
            m_Buffers.clear();
        }
        void _resizeBaseBuffer(uint w,uint h){
            m_width  = w; m_height = h;
            glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

            // Bind the depth buffer
            glBindRenderbuffer(GL_RENDERBUFFER, m_depth);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        void _start(std::vector<uint>& types,std::string& channels){
            bool r,g,b,a;
            if(channels.find("R") != std::string::npos) r=true; else r=false;
            if(channels.find("G") != std::string::npos) g=true; else g=false;
            if(channels.find("B") != std::string::npos) b=true; else b=false;
            if(channels.find("A") != std::string::npos) a=true; else a=false;
            glColorMask(r,g,b,a);
			glEnable(GL_TEXTURE_2D);
            glActiveTexture(GL_TEXTURE0);
            glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);// Bind our FBO and set the viewport to the proper size
            glDrawBuffers(types.size(), &types[0]);  // Specify what to render an start acquiring
            glClear(GL_COLOR_BUFFER_BIT);            // Clear the render targets
        }
        void _start(uint type,std::string& channels){
            std::vector<uint> types;
            types.push_back(m_Buffers[type]->attatchment());
            _start(types,channels);
        }
        void _start(uint type,uint type1,std::string& channels){
            std::vector<uint> types;
            types.push_back(m_Buffers[type]->attatchment());
            types.push_back(m_Buffers[type1]->attatchment());
            _start(types,channels);
        }
        void _start(uint type,uint type1,uint type2,std::string& channels){
            std::vector<uint> types;
            types.push_back(m_Buffers[type]->attatchment());
            types.push_back(m_Buffers[type1]->attatchment());
            types.push_back(m_Buffers[type2]->attatchment());
            _start(types,channels);
        }
        void _start(uint type,uint type1,uint type2,uint type3,std::string& channels){
            std::vector<uint> types;
            types.push_back(m_Buffers[type]->attatchment());
            types.push_back(m_Buffers[type1]->attatchment());
            types.push_back(m_Buffers[type2]->attatchment());
            types.push_back(m_Buffers[type3]->attatchment());
            _start(types,channels);
        }
        void _start(uint type,uint type1,uint type2,uint type3,uint type4,std::string& channels){
            std::vector<uint> types;
            types.push_back(m_Buffers[type]->attatchment());
            types.push_back(m_Buffers[type1]->attatchment());
            types.push_back(m_Buffers[type2]->attatchment());
            types.push_back(m_Buffers[type3]->attatchment());
            types.push_back(m_Buffers[type4]->attatchment());
            _start(types,channels);
        }
        void _start(uint type,uint type1,uint type2,uint type3,uint type4,uint type5,std::string& channels){
            std::vector<uint> types;
            types.push_back(m_Buffers[type]->attatchment());
            types.push_back(m_Buffers[type1]->attatchment());
            types.push_back(m_Buffers[type2]->attatchment());
            types.push_back(m_Buffers[type3]->attatchment());
            types.push_back(m_Buffers[type4]->attatchment());
            types.push_back(m_Buffers[type5]->attatchment());
            _start(types,channels);
        }
        void _stop(){
            // Stop acquiring and unbind the FBO
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            glColorMask(1,1,1,1);
            // Clear the render targets
            glClear(GL_COLOR_BUFFER_BIT);
        }
};
TextureBuffer::TextureBuffer(int internalformat, int format, int type, int attatchment,uint width,uint height,float sizeScalar):m_i(new impl()){
    m_i->_init(internalformat,format,type,attatchment,width,height,sizeScalar);
}
TextureBuffer::~TextureBuffer(){
    m_i->_destruct();
}
void TextureBuffer::resize(uint width,uint height){
    m_i->_resize(width,height);
}
GLuint TextureBuffer::texture() const{
    return m_i->m_Texture;
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
void GBuffer::start(std::vector<uint>& types,std::string channels){
    m_i->_start(types,channels);
}
void GBuffer::start(uint type,std::string channels){
    m_i->_start(type,channels);
}
void GBuffer::start(uint type,uint type1,std::string channels){
    m_i->_start(type,type1,channels);
}
void GBuffer::start(uint type,uint type1,uint type2,std::string channels){
    m_i->_start(type,type1,type2,channels);
}
void GBuffer::start(uint type,uint type1,uint type2,uint type3,std::string channels){
    m_i->_start(type,type1,type2,type3,channels);
}
void GBuffer::start(uint type,uint type1,uint type2,uint type3,uint type4,std::string channels){
    m_i->_start(type,type1,type2,type3,type4,channels);
}
void GBuffer::start(uint type,uint type1,uint type2,uint type3,uint type4,uint type5,std::string channels){
    m_i->_start(type,type1,type2,type3,type4,type5,channels);
}
void GBuffer::stop(){	
    m_i->_stop();
}
std::unordered_map<uint,TextureBuffer*> GBuffer::getBuffers(){ 
    return m_i->m_Buffers; 
}
GLuint GBuffer::getTexture(uint type){ 
    return m_i->m_Buffers[type]->texture(); 
}