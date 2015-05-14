#ifndef GBUFFER_H
#define GBUFFER_H
#include "TextureBuffer.h"
#include <unordered_map>

const int GBUFFER_TYPES[] =		       {GL_RGBA8,		       // (diffuse.rgba)
								        GL_RGBA16F,		       // (normals.rgba)
										GL_RG8,			       // Glow & SSAO
										GL_RGB32F,			   // World Position
								        GL_RGB8,		       // (lighting.rgb)
										GL_RGBA8,              // bloom
										GL_RGBA8,              // free buffer
										GL_DEPTH_COMPONENT16}; // depth

const int GBUFFER_PIXEL_TYPES[] =      {GL_RGBA,			   // (diffuse.rgba)
							            GL_RGBA,			   // (normals.rgba)
										GL_RG,			       // Glow & SSAO
										GL_RGB,				   // World Position
								        GL_RGB,			       // (lighting.rgb)
								        GL_RGBA,			   // bloom
								        GL_RGBA,			   // free buffer
								        GL_DEPTH_COMPONENT};   // depth

const int GBUFFER_FLOAT_TYPES[] =      {GL_UNSIGNED_BYTE,
						                GL_UNSIGNED_BYTE,
										GL_UNSIGNED_BYTE,
										GL_UNSIGNED_BYTE,
								        GL_UNSIGNED_BYTE,
								        GL_UNSIGNED_BYTE,
								        GL_UNSIGNED_BYTE,
								        GL_FLOAT};

const int GBUFFER_ATTACHMENT_TYPES[] = {GL_COLOR_ATTACHMENT0,
							            GL_COLOR_ATTACHMENT1,
										GL_COLOR_ATTACHMENT2,
										GL_COLOR_ATTACHMENT3,
										GL_COLOR_ATTACHMENT4,
										GL_COLOR_ATTACHMENT5,
										GL_COLOR_ATTACHMENT6,
										GL_DEPTH_ATTACHMENT};

enum BUFFER_TYPES {BUFFER_TYPE_DIFFUSE,BUFFER_TYPE_NORMAL,BUFFER_TYPE_GLOW,BUFFER_TYPE_POSITION,BUFFER_TYPE_LIGHTING,BUFFER_TYPE_BLOOM,
				   BUFFER_TYPE_FREE1,
				   BUFFER_TYPE_DEPTH,
				   BUFFER_TYPE_NUMBER};


class GBuffer{
	private:
		GLuint m_fbo;
		GLuint m_depth;

		std::unordered_map<unsigned int,TextureBuffer*> m_Buffers;

		unsigned int m_width;
		unsigned int m_height;

	public:
		GBuffer(int width, int height);
		~GBuffer();

		void start(unsigned int,std::string = "RGBA");
		void start(unsigned int,unsigned int,std::string = "RGBA");
		void start(unsigned int,unsigned int,unsigned int,std::string = "RGBA");
		void start(unsigned int,unsigned int,unsigned int,unsigned int,std::string = "RGBA");
		void stop(std::string = "RGBA");

		std::unordered_map<unsigned int,TextureBuffer*> getBuffers();
		GLuint getTexture(unsigned int);
};
#endif