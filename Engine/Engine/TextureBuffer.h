#ifndef TEXTURE_BUFFER_H
#define TEXTURE_BUFFER_H

#include <GL/glew.h>
#include <GL/GL.h>

class TextureBuffer{
	private:
		int m_BufferInternalFormat;
		int m_BufferFormat;
		int m_BufferType;
		int m_BufferAttatchment;

		unsigned int m_width;
		unsigned int m_height;
		
		GLuint m_Texture;

	public:
		TextureBuffer(int,int,int,int,unsigned int,unsigned int);
		~TextureBuffer();

		GLuint getTexture() const { return m_Texture; }
		int getAttatchment() const { return m_BufferAttatchment; }
};

#endif