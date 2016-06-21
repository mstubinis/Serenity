#pragma once
#ifndef ENGINE_GBUFFER_H
#define ENGINE_GBUFFER_H

#include <memory>

#define GL_RGB 0x1907
#define GL_RGBA 0x1908
#define GL_RGB8 0x8051
#define GL_RGB16F 0x881B
#define GL_RGB32F 0x8815
#define GL_RGBA8 0x8058
#define GL_DEPTH_COMPONENT 0x1902
#define GL_DEPTH_COMPONENT24 0x81A6
#define GL_DEPTH_COMPONENT16 0x81A5

#define GL_UNSIGNED_BYTE 0x1401
#define GL_FLOAT 0x1406
#define GL_DEPTH_ATTACHMENT 0x8D00

#define GL_COLOR_ATTACHMENT0 0x8CE0
#define GL_COLOR_ATTACHMENT1 0x8CE1
#define GL_COLOR_ATTACHMENT2 0x8CE2
#define GL_COLOR_ATTACHMENT3 0x8CE3
#define GL_COLOR_ATTACHMENT4 0x8CE4
#define GL_COLOR_ATTACHMENT5 0x8CE5
#define GL_COLOR_ATTACHMENT6 0x8CE6
#define GL_COLOR_ATTACHMENT7 0x8CE7
#define GL_COLOR_ATTACHMENT8 0x8CE8
#define GL_COLOR_ATTACHMENT9 0x8CE9
#define GL_COLOR_ATTACHMENT10 0x8CEA
#define GL_COLOR_ATTACHMENT11 0x8CEB
#define GL_COLOR_ATTACHMENT12 0x8CEC
#define GL_COLOR_ATTACHMENT13 0x8CED
#define GL_COLOR_ATTACHMENT14 0x8CEE
#define GL_COLOR_ATTACHMENT15 0x8CEF

typedef unsigned int uint;

//framebuffer pixel size = window size * this value here
const float GBUFFER_DIVISIBLES[] = {1.0f, // (diffuse.rgb)
	                                1.0f, // (normals.rgb)
								    1.0f, // Glow & NULL & Specular THEN HDR
								    1.0f, // World Position
								    1.0f, // (lighting.rgb)
								    0.5f, // bloom, & ssao as alpha
								    1.0f, // gods rays
								    1.0f, // free buffer
								    0.5f, // free2 buffer
								    1.0f};// depth

const int GBUFFER_TYPES[] =	           {GL_RGB8,               // (diffuse.rgb)
                                        GL_RGB16F,		       // (normals.rgb)
                                        GL_RGB8,			   // Glow & NULL & Specular THEN HDR
                                        GL_RGB32F,			   // World Position
                                        GL_RGB16F,		       // (lighting.rgb)
                                        GL_RGBA8,              // bloom, & ssao as alpha
                                        GL_RGB8,			   // gods rays
                                        GL_RGBA8,              // free buffer
                                        GL_RGBA8,              // free2 buffer
                                        GL_DEPTH_COMPONENT24}; // depth

const int GBUFFER_PIXEL_TYPES[] =      {GL_RGB,			       // (diffuse.rgb)
                                        GL_RGB,			       // (normals.rgb)
                                        GL_RGB,			       // Glow & NULL & Specular THEN HDR
                                        GL_RGB,				   // World Position
                                        GL_RGB,			       // (lighting.rgb)
                                        GL_RGBA,			   // bloom, & ssao as alpha
                                        GL_RGB,			       // gods rays
                                        GL_RGBA,			   // free buffer
                                        GL_RGBA,			   // free2 buffer
                                        GL_DEPTH_COMPONENT};   // depth

const int GBUFFER_FLOAT_TYPES[] =      {GL_UNSIGNED_BYTE,
                                        GL_UNSIGNED_BYTE,
                                        GL_UNSIGNED_BYTE,
                                        GL_UNSIGNED_BYTE,
                                        GL_FLOAT,
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
                                        GL_COLOR_ATTACHMENT0,
                                        GL_COLOR_ATTACHMENT5,
                                        GL_COLOR_ATTACHMENT6,
										GL_COLOR_ATTACHMENT1,
                                        GL_DEPTH_ATTACHMENT};

enum BUFFER_TYPES {BUFFER_TYPE_DIFFUSE,
                   BUFFER_TYPE_NORMAL,
                   BUFFER_TYPE_MISC,
                   BUFFER_TYPE_POSITION,
                   BUFFER_TYPE_LIGHTING,
                   BUFFER_TYPE_BLOOM,
                   BUFFER_TYPE_GODSRAYS,
                   BUFFER_TYPE_FREE1,
				   BUFFER_TYPE_FREE2,
                   BUFFER_TYPE_DEPTH,
                   BUFFER_TYPE_NUMBER};


class TextureBuffer final{
    private:
        class impl;
        std::unique_ptr<impl> m_i;
    public:
        TextureBuffer(int,int,int,int,uint,uint,float=1.0f);
        ~TextureBuffer();

		float sizeScalar() const;
        void resize(uint,uint);
        GLuint texture() const;
        int attatchment() const;
};

class GBuffer final{
    private:
        class impl;
        std::unique_ptr<impl> m_i;
    public:
        GBuffer(uint width,uint height);
        ~GBuffer();

        void resizeBaseBuffer(uint w,uint h);
        void resizeBuffer(uint,uint w,uint h);

        void start(std::vector<uint>&,std::string = "RGBA",bool=true);
        void start(uint,std::string = "RGBA",bool=true);
        void start(uint,uint,std::string = "RGBA",bool=true);
        void start(uint,uint,uint,std::string = "RGBA",bool=true);
        void start(uint,uint,uint,uint,std::string = "RGBA",bool=true);
        void start(uint,uint,uint,uint,uint,std::string = "RGBA",bool=true);
        void start(uint,uint,uint,uint,uint,uint,std::string = "RGBA",bool=true);
        void stop();

        std::unordered_map<uint,TextureBuffer*> getBuffers();
        GLuint getTexture(uint);
};
#endif