#include "GBuffer.h"
#include "TextureBuffer.h"

//Create the FBO render texture initializing all the stuff that we need
GBuffer::GBuffer(int width, int height){	
	m_width  = width; m_height = height;

	glGenFramebuffers(1, &m_fbo);
	glGenRenderbuffers(1, &m_depth);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0,0,0,0);

	// Bind the depth buffer
	glBindRenderbuffer(GL_RENDERBUFFER, m_depth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_width, m_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth);

	glClear(GL_DEPTH_BUFFER_BIT);
	glClearColor(0,0,0,0);

	for(unsigned int i = 0; i < BUFFER_TYPE_NUMBER; i++){
		TextureBuffer* tbo = new TextureBuffer(GBUFFER_TYPES[i],
											   GBUFFER_PIXEL_TYPES[i],
											   GBUFFER_FLOAT_TYPES[i],
											   GBUFFER_ATTACHMENT_TYPES[i],
											   m_width,
											   m_height);
		m_Buffers[i] = tbo;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
GBuffer::~GBuffer(){
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	clearBuffers();
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
void GBuffer::clearBuffer(unsigned int buffer){
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	m_Buffers[buffer]->clear(m_fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void GBuffer::clearBuffers(){
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	for(unsigned int i = 0; i < BUFFER_TYPE_NUMBER; i++) clearBuffer(i); 

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glPopAttrib();

	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0,0,0,0);
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
	glPushAttrib(GL_VIEWPORT_BIT);

	glViewport(0,0,m_width, m_height);

	// Specify what to render an start acquiring
	glDrawBuffers(types.size(), &types[0]);

	// Clear the render targets
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0,0,0,0);
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
void GBuffer::stop(std::string channels){	
	// Stop acquiring and unbind the FBO
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glPopAttrib();

	unsigned int r,g,b,a;
	if(channels.find("R") != std::string::npos) r=1; else r=0;
	if(channels.find("G") != std::string::npos) g=1; else g=0;
	if(channels.find("B") != std::string::npos) b=1; else b=0;
	if(channels.find("A") != std::string::npos) a=1; else a=0;
	glColorMask(r,g,b,a);
}
std::unordered_map<unsigned int,TextureBuffer*> GBuffer::getBuffers(){ return m_Buffers; }
GLuint GBuffer::getTexture(unsigned int type){ return m_Buffers[type]->getTexture(); }