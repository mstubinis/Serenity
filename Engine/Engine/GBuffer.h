#ifndef ENGINE_GBUFFER_H
#define ENGINE_GBUFFER_H

#include <unordered_map>
#include <GL/glew.h>
#include <GL/GL.h>

const int GBUFFER_TYPES[] =		       {GL_RGBA16F,		       // (diffuse.rgba)
									    GL_RGBA16F,			   // (diffuse forward rendering.rgba)
								        GL_RGB16F,		       // (normals.rgb)
										GL_RGB,			       // Glow & SSAO & Specular
										GL_RGB32F,			   // World Position
								        GL_RGB,		           // (lighting.rgb)
										GL_RGBA,               // bloom
										GL_RGBA16F,            // free buffer
										GL_DEPTH_COMPONENT16}; // depth

const int GBUFFER_PIXEL_TYPES[] =      {GL_RGBA,			   // (diffuse.rgba)
										GL_RGBA,			   // (diffuse forward rendering.rgba)
							            GL_RGB,			       // (normals.rgb)
										GL_RGB,			       // Glow & SSAO & Specular
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
								        GL_UNSIGNED_BYTE,
								        GL_FLOAT};

const int GBUFFER_ATTACHMENT_TYPES[] = {GL_COLOR_ATTACHMENT0,
							            GL_COLOR_ATTACHMENT1,
										GL_COLOR_ATTACHMENT2,
										GL_COLOR_ATTACHMENT3,
										GL_COLOR_ATTACHMENT4,
										GL_COLOR_ATTACHMENT5,
										GL_COLOR_ATTACHMENT6,
										GL_COLOR_ATTACHMENT7,
										GL_DEPTH_ATTACHMENT};

enum BUFFER_TYPES {BUFFER_TYPE_DIFFUSE,
				   BUFFER_TYPE_DIFFUSE_FORWARD,
	               BUFFER_TYPE_NORMAL,
				   BUFFER_TYPE_GLOW,
				   BUFFER_TYPE_POSITION,
				   BUFFER_TYPE_LIGHTING,
				   BUFFER_TYPE_BLOOM,
				   BUFFER_TYPE_FREE1,
				   BUFFER_TYPE_DEPTH,
				   BUFFER_TYPE_NUMBER};


class TextureBuffer;
class GBuffer final{
	private:
		GLuint m_fbo;
		GLuint m_depth;

		std::unordered_map<unsigned int,TextureBuffer*> m_Buffers;

		unsigned int m_width;
		unsigned int m_height;

	public:
		GBuffer(int width, int height);
		~GBuffer();

		void resizeBaseBuffer(unsigned int w, unsigned int h);
		void resizeBuffer(unsigned int, unsigned int w, unsigned int h);
		void clearBuffer(unsigned int);
		void clearBuffers();

		void start(std::vector<unsigned int>&,std::string = "RGBA");
		void start(unsigned int,std::string = "RGBA");
		void start(unsigned int,unsigned int,std::string = "RGBA");
		void start(unsigned int,unsigned int,unsigned int,std::string = "RGBA");
		void start(unsigned int,unsigned int,unsigned int,unsigned int,std::string = "RGBA");
		void stop(std::string = "RGBA");

		std::unordered_map<unsigned int,TextureBuffer*> getBuffers();
		GLuint getTexture(unsigned int);
};
#endif