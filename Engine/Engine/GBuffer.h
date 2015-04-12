#ifndef GBUFFER_H
#define GBUFFER_H
#include "TextureBuffer.h"
#include <unordered_map>

const int GBUFFER_TYPES[] =		       {GL_RGBA8,		      //diffuse
								        GL_RGB8,		      //normals
								        GL_RGB32F,		      //lighting
								        GL_RGBA16F,           //bloom + lightmap (1 channel)
									    GL_R8,                //ssao
										GL_RGBA16F,           //free1
										GL_DEPTH_COMPONENT32F};//depth

const int GBUFFER_PIXEL_TYPES[] =      {GL_RGBA,			  //diffuse
							            GL_RGB,			      //normals
								        GL_RGB,			      //lighting
								        GL_RGBA,		      //bloom + lightmap (1 channel)
								        GL_RED,               //ssao
								        GL_RGBA,			  //free1
								        GL_DEPTH_COMPONENT};  //depth

const int GBUFFER_FLOAT_TYPES[] =      {GL_UNSIGNED_BYTE,     //diffuse
						                GL_UNSIGNED_BYTE,     //normals
								        GL_UNSIGNED_BYTE,     //lighting
								        GL_UNSIGNED_BYTE,     //bloom + lightmap (1 channel)
								        GL_UNSIGNED_BYTE,     //ssao
								        GL_UNSIGNED_BYTE,     //free1
								        GL_FLOAT};		      //depth

const int GBUFFER_ATTACHMENT_TYPES[] = {GL_COLOR_ATTACHMENT0, //diffuse
							            GL_COLOR_ATTACHMENT1, //normals
										GL_COLOR_ATTACHMENT2, //lighting
										GL_COLOR_ATTACHMENT3, //bloom + lightmap (1 channel)
										GL_COLOR_ATTACHMENT4, //ssao
										GL_COLOR_ATTACHMENT5, //free1
										GL_DEPTH_ATTACHMENT}; //depth

enum BUFFER_TYPES {BUFFER_TYPE_DIFFUSE,BUFFER_TYPE_NORMAL,BUFFER_TYPE_LIGHTING,BUFFER_TYPE_BLOOM,BUFFER_TYPE_SSAO,
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

		void Start(unsigned int);
		void Start(unsigned int,unsigned int);
		void Start(unsigned int,unsigned int,unsigned int);
		void Stop();

		std::unordered_map<unsigned int,TextureBuffer*> Buffers();
		GLuint Texture(unsigned int);
};
#endif