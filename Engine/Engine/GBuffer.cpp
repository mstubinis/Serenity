#include <unordered_map>
#include <GL/glew.h>
#include <GL/GL.h>
#include "GBuffer.h"
#include "Engine_Resources.h"

using namespace Engine;

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

        std::unordered_map<uint,boost::weak_ptr<TextureBuffer>> m_Buffers;
        uint m_Width; uint m_Height;
        void _init(uint w,uint h){
            m_Width = w; m_Height = h;

            glGenFramebuffers(1, &m_fbo);
            glGenRenderbuffers(1, &m_depth);
            glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
            glBindRenderbuffer(GL_RENDERBUFFER, m_depth);// Bind the depth buffer
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_Width, m_Height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth);

            _constructFramebuffer("BUFFER_DIFFUSE",BUFFER_TYPE_DIFFUSE,m_Width,m_Height);
            _constructFramebuffer("BUFFER_NORMAL",BUFFER_TYPE_NORMAL,m_Width,m_Height);
            _constructFramebuffer("BUFFER_MISC",BUFFER_TYPE_MISC,m_Width,m_Height);
            _constructFramebuffer("BUFFER_POSITION",BUFFER_TYPE_POSITION,m_Width,m_Height);
            _constructFramebuffer("BUFFER_LIGHTING",BUFFER_TYPE_LIGHTING,m_Width,m_Height);
            _constructFramebuffer("BUFFER_FREE1",BUFFER_TYPE_FREE1,m_Width,m_Height);
            _constructFramebuffer("BUFFER_DEPTH",BUFFER_TYPE_DEPTH,m_Width,m_Height);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            glGenFramebuffers(1, &m_fbo_bloom);
            glGenRenderbuffers(1, &m_depth_fake);
            glBindFramebuffer(GL_FRAMEBUFFER, m_fbo_bloom);
            glBindRenderbuffer(GL_RENDERBUFFER, m_depth_fake);// Bind the depth buffer
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_Width, m_Height);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth_fake);

            _constructFramebuffer("BUFFER_BLOOM",BUFFER_TYPE_BLOOM,m_Width,m_Height);
            _constructFramebuffer("BUFFER_FREE2",BUFFER_TYPE_FREE2,m_Width,m_Height);
            _constructFramebuffer("BUFFER_GODSRAYS",BUFFER_TYPE_GODSRAYS,m_Width,m_Height);

            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
        void _constructFramebuffer(std::string n,BUFFER_TYPES t,uint w,uint h){
            TextureBuffer* tbo = nullptr;
            tbo = new TextureBuffer(n,GBUFFER_TYPES[t],GBUFFER_PIXEL_TYPES[t],GBUFFER_FLOAT_TYPES[t],GBUFFER_ATTACHMENT_TYPES[t],w,h,GBUFFER_DIVISIBLES[t]);
            boost::weak_ptr<TextureBuffer> ptr = boost::dynamic_pointer_cast<TextureBuffer>(Resources::getTexturePtr(tbo->name()));
            m_Buffers[t] = ptr;
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

            for(auto buffer:m_Buffers){
                buffer.second.reset();
            }
        }
        void _resizeBaseBuffer(uint w,uint h){
            m_Width  = w; m_Height = h;
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
            types.push_back(m_Buffers.at(type).lock().get()->attatchment());
            _start(types,channels,first_fbo);
        }
        void _start(uint type,uint type1,std::string& channels,bool first_fbo){
            std::vector<uint> types;
            types.push_back(m_Buffers.at(type).lock().get()->attatchment());
            types.push_back(m_Buffers.at(type1).lock().get()->attatchment());
            _start(types,channels,first_fbo);
        }
        void _start(uint type,uint type1,uint type2,std::string& channels,bool first_fbo){
            std::vector<uint> types;
            types.push_back(m_Buffers.at(type).lock().get()->attatchment());
            types.push_back(m_Buffers.at(type1).lock().get()->attatchment());
            types.push_back(m_Buffers.at(type2).lock().get()->attatchment());
            _start(types,channels,first_fbo);
        }
        void _start(uint type,uint type1,uint type2,uint type3,std::string& channels,bool first_fbo){
            std::vector<uint> types;
            types.push_back(m_Buffers.at(type).lock().get()->attatchment());
            types.push_back(m_Buffers.at(type1).lock().get()->attatchment());
            types.push_back(m_Buffers.at(type2).lock().get()->attatchment());
            types.push_back(m_Buffers.at(type3).lock().get()->attatchment());
            _start(types,channels,first_fbo);
        }
        void _start(uint type,uint type1,uint type2,uint type3,uint type4,std::string& channels,bool first_fbo){
            std::vector<uint> types;
            types.push_back(m_Buffers.at(type).lock().get()->attatchment());
            types.push_back(m_Buffers.at(type1).lock().get()->attatchment());
            types.push_back(m_Buffers.at(type2).lock().get()->attatchment());
            types.push_back(m_Buffers.at(type3).lock().get()->attatchment());
            types.push_back(m_Buffers.at(type4).lock().get()->attatchment());
            _start(types,channels,first_fbo);
        }
        void _start(uint type,uint type1,uint type2,uint type3,uint type4,uint type5,std::string& channels,bool first_fbo){
            std::vector<uint> types;
            types.push_back(m_Buffers.at(type).lock().get()->attatchment());
            types.push_back(m_Buffers.at(type1).lock().get()->attatchment());
            types.push_back(m_Buffers.at(type2).lock().get()->attatchment());
            types.push_back(m_Buffers.at(type3).lock().get()->attatchment());
            types.push_back(m_Buffers.at(type4).lock().get()->attatchment());
            types.push_back(m_Buffers.at(type5).lock().get()->attatchment());
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
    m_i->m_Buffers.at(buffer).lock().get()->resize(width,height); 
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
const std::unordered_map<uint,boost::weak_ptr<TextureBuffer>>& GBuffer::getBuffers() const{ 
    return m_i->m_Buffers; 
}
Texture* GBuffer::getTexture(uint type){ 
    return m_i->m_Buffers.at(type).lock().get();
}