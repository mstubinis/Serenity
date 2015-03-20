#include "TextureBuffer.h"

TextureBuffer::TextureBuffer(int internalformat, int format, int type, int attatchment, unsigned int width, unsigned int height){
	m_BufferInternalFormat = internalformat;
	m_BufferFormat = format;
	m_BufferType = type;
	m_BufferAttatchment = attatchment;

	m_width = width;
	m_height = height;

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
GLuint TextureBuffer::Texture()const { return m_Texture; }
int TextureBuffer::Attatchment()const { return m_BufferAttatchment; }
