#include "GBuffer.h"

TextureBuffer::TextureBuffer(int internalformat, int format, int type, int attatchment, unsigned int width, unsigned int height){
	m_BufferInternalFormat = internalformat;
	m_BufferFormat = format;
	m_BufferType = type;
	m_BufferAttatchment = attatchment;

	m_width = width; m_height = height;

	glGenTextures(1, &m_Texture);
	glBindTexture(GL_TEXTURE_2D, m_Texture);

	glTexImage2D(GL_TEXTURE_2D, 0, internalformat, width, height, 0, format, type, 0);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glFramebufferTexture2D(GL_FRAMEBUFFER, attatchment, GL_TEXTURE_2D, m_Texture, 0);
}
TextureBuffer::~TextureBuffer(){
	glDeleteTextures(1, &m_Texture);
}

void TextureBuffer::resize(unsigned int width, unsigned int height){
	m_width = width; m_height = height;
	glBindTexture(GL_TEXTURE_2D, m_Texture);
	glTexImage2D(GL_TEXTURE_2D, 0, m_BufferInternalFormat, width, height, 0, m_BufferFormat, m_BufferType, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, m_BufferAttatchment, GL_TEXTURE_2D, m_Texture, 0);
}

//Create the FBO render texture initializing all the stuff that we need
GBuffer::GBuffer(int width, int height){	
	m_width  = width; m_height = height;

	glGenFramebuffers(1, &m_fbo);
	glGenRenderbuffers(1, &m_depth);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

	// Bind the depth buffer
	glBindRenderbuffer(GL_RENDERBUFFER, m_depth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_width, m_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth);

	for(unsigned int i = 0; i < BUFFER_TYPE_NUMBER; i++){
		TextureBuffer* tbo = new TextureBuffer(GBUFFER_TYPES[i],GBUFFER_PIXEL_TYPES[i],
											   GBUFFER_FLOAT_TYPES[i],GBUFFER_ATTACHMENT_TYPES[i],
											   m_width,m_height);
		m_Buffers[i] = tbo;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
GBuffer::~GBuffer(){
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	glDeleteRenderbuffers(1, &m_depth);
	glDeleteFramebuffers(1, &m_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	for(auto buffer:m_Buffers){
		delete(buffer.second);
	}
	m_Buffers.clear();
}
void GBuffer::resizeBaseBuffer(unsigned int width, unsigned int height){
	m_width  = width; m_height = height;
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

	// Bind the depth buffer
	glBindRenderbuffer(GL_RENDERBUFFER, m_depth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void GBuffer::resizeBuffer(unsigned int buffer, unsigned int width, unsigned int height){ m_Buffers[buffer]->resize(width,height); }
void GBuffer::start(std::vector<unsigned int>& types,std::string channels){
	unsigned int r,g,b,a;
	if(channels.find("R") != std::string::npos) r=1; else r=0;
	if(channels.find("G") != std::string::npos) g=1; else g=0;
	if(channels.find("B") != std::string::npos) b=1; else b=0;
	if(channels.find("A") != std::string::npos) a=1; else a=0;
	glColorMask(r,g,b,a);
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);

	// Bind our FBO and set the viewport to the proper size
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

	// Specify what to render an start acquiring
	glDrawBuffers(types.size(), &types[0]);

	// Clear the render targets
	glClear(GL_COLOR_BUFFER_BIT);
}
void GBuffer::start(unsigned int type,std::string channels){
	std::vector<unsigned int> types;
	types.push_back(m_Buffers[type]->getAttatchment());
	start(types,channels);
}
void GBuffer::start(unsigned int type,unsigned int type1,std::string channels){
	std::vector<unsigned int> types;
	types.push_back(m_Buffers[type]->getAttatchment());
	types.push_back(m_Buffers[type1]->getAttatchment());
	start(types,channels);
}
void GBuffer::start(unsigned int type,unsigned int type1,unsigned int type2,std::string channels){
	std::vector<unsigned int> types;
	types.push_back(m_Buffers[type]->getAttatchment());
	types.push_back(m_Buffers[type1]->getAttatchment());
	types.push_back(m_Buffers[type2]->getAttatchment());
	start(types,channels);
}
void GBuffer::start(unsigned int type,unsigned int type1,unsigned int type2,unsigned int type3,std::string channels){
	std::vector<unsigned int> types;
	types.push_back(m_Buffers[type]->getAttatchment());
	types.push_back(m_Buffers[type1]->getAttatchment());
	types.push_back(m_Buffers[type2]->getAttatchment());
	types.push_back(m_Buffers[type3]->getAttatchment());
	start(types,channels);
}
void GBuffer::stop(){	
	// Stop acquiring and unbind the FBO
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glColorMask(1,1,1,1);
	// Clear the render targets
	glClear(GL_COLOR_BUFFER_BIT);
}
std::unordered_map<unsigned int,TextureBuffer*> GBuffer::getBuffers(){ return m_Buffers; }
GLuint GBuffer::getTexture(unsigned int type){ return m_Buffers[type]->getTexture(); }