#include "GBuffer.h"

//Create the FBO render texture initializing all the stuff that we need
GBuffer::GBuffer(int width, int height){	
	m_width  = width;
	m_height = height;

	glGenFramebuffers(1, &m_fbo);
	glGenRenderbuffers(1, &m_depth);
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

	// Bind the depth buffer
	glBindRenderbuffer(GL_RENDERBUFFER, m_depth);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_width, m_height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, m_depth);

	for(unsigned int i = 0; i < BUFFER_TYPE_NUMBER; i++){
		TextureBuffer* tbo = new TextureBuffer(GBUFFER_TYPES[i],GBUFFER_PIXEL_TYPES[i],GBUFFER_FLOAT_TYPES[i],GBUFFER_ATTACHMENT_TYPES[i],m_width,m_height);
		m_Buffers[i] = tbo;
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

GBuffer::~GBuffer(){
	for(auto iterator:m_Buffers){
		delete iterator.second;
	}
	glDeleteFramebuffers(1, &m_fbo);
	glDeleteRenderbuffers(1, &m_depth);
}
void GBuffer::start(unsigned int type){
	// Bind our FBO and set the viewport to the proper size
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	glPushAttrib(GL_VIEWPORT_BIT);

	glViewport(0,0,m_width, m_height);

	// Specify what to render an start acquiring
	unsigned int buffers[] = { m_Buffers[type]->getAttatchment() };
	glDrawBuffers(sizeof(buffers)/sizeof(*buffers), buffers);

	// Clear the render targets
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glClearColor( 0, 0, 0, 1 );

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
}
void GBuffer::start(unsigned int type,unsigned int type1){
	// Bind our FBO and set the viewport to the proper size
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	glPushAttrib(GL_VIEWPORT_BIT);
	glViewport(0,0,m_width, m_height);

	// Specify what to render an start acquiring
	unsigned int buffers[] = { m_Buffers[type]->getAttatchment(),m_Buffers[type1]->getAttatchment() };
	glDrawBuffers(sizeof(buffers)/sizeof(*buffers), buffers);

	// Clear the render targets
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glClearColor( 0, 0, 0, 1 );

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
}
void GBuffer::start(unsigned int type,unsigned int type1,unsigned int type2){
	// Bind our FBO and set the viewport to the proper size
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	glPushAttrib(GL_VIEWPORT_BIT);
	glViewport(0,0,m_width, m_height);

	// Specify what to render an start acquiring
	unsigned int buffers[] = { m_Buffers[type]->getAttatchment(),m_Buffers[type1]->getAttatchment(),m_Buffers[type2]->getAttatchment() };
	glDrawBuffers(sizeof(buffers)/sizeof(*buffers), buffers);

	// Clear the render targets
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glClearColor( 0, 0, 0, 1 );

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
}
void GBuffer::start(unsigned int type,unsigned int type1,unsigned int type2,unsigned int type3){
	// Bind our FBO and set the viewport to the proper size
	glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
	glPushAttrib(GL_VIEWPORT_BIT);
	glViewport(0,0,m_width, m_height);

	// Specify what to render an start acquiring
	unsigned int buffers[] = { m_Buffers[type]->getAttatchment(),m_Buffers[type1]->getAttatchment(),m_Buffers[type2]->getAttatchment(),m_Buffers[type3]->getAttatchment() };
	glDrawBuffers(sizeof(buffers)/sizeof(*buffers), buffers);

	// Clear the render targets
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glClearColor( 0, 0, 0, 1 );

	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_2D);
}
void GBuffer::stop(){	
	// Stop acquiring and unbind the FBO
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glPopAttrib();
}
std::unordered_map<unsigned int,TextureBuffer*> GBuffer::getBuffers(){ return m_Buffers; }
GLuint GBuffer::getTexture(unsigned int type){ return m_Buffers[type]->getTexture(); }