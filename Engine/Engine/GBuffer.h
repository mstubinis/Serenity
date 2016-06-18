#pragma once
#ifndef ENGINE_GBUFFER_H
#define ENGINE_GBUFFER_H

#include <GL/glew.h>
#include <GL/GL.h>
#include <memory>

typedef unsigned int uint;

const int GBUFFER_TYPES[] =	           {GL_RGB8,               // (diffuse.rgb)
                                        GL_RGB16F,		       // (normals.rgb)
                                        GL_RGB8,			   // Glow & SSAO & Specular THEN HDR
                                        GL_RGB32F,			   // World Position
                                        GL_RGB16F,		       // (lighting.rgb)
                                        GL_RGB8,               // bloom
                                        GL_RGB8,			   // gods rays
                                        GL_RGBA8,              // free buffer
                                        GL_DEPTH_COMPONENT24}; // depth

const int GBUFFER_PIXEL_TYPES[] =      {GL_RGB,			       // (diffuse.rgb)
                                        GL_RGB,			       // (normals.rgb)
                                        GL_RGB,			       // Glow & SSAO & Specular THEN HDR
                                        GL_RGB,				   // World Position
                                        GL_RGB,			       // (lighting.rgb)
                                        GL_RGB,			       // bloom
                                        GL_RGB,			       // gods rays
                                        GL_RGBA,			   // free buffer
                                        GL_DEPTH_COMPONENT};   // depth

const int GBUFFER_FLOAT_TYPES[] =      {GL_UNSIGNED_BYTE,
                                        GL_UNSIGNED_BYTE,
                                        GL_UNSIGNED_BYTE,
                                        GL_UNSIGNED_BYTE,
                                        GL_FLOAT,
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
                   BUFFER_TYPE_NORMAL,
                   BUFFER_TYPE_MISC,
                   BUFFER_TYPE_POSITION,
                   BUFFER_TYPE_LIGHTING,
                   BUFFER_TYPE_BLOOM,
                   BUFFER_TYPE_GODSRAYS,
                   BUFFER_TYPE_FREE1,
                   BUFFER_TYPE_DEPTH,
                   BUFFER_TYPE_NUMBER};


class TextureBuffer final{
    private:
        class impl;
        std::unique_ptr<impl> m_i;
    public:
        TextureBuffer(int,int,int,int,uint,uint);
        ~TextureBuffer();

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

        void start(std::vector<uint>&,std::string = "RGBA");
        void start(uint,std::string = "RGBA");
        void start(uint,uint,std::string = "RGBA");
        void start(uint,uint,uint,std::string = "RGBA");
        void start(uint,uint,uint,uint,std::string = "RGBA");
        void start(uint,uint,uint,uint,uint,std::string = "RGBA");
        void start(uint,uint,uint,uint,uint,uint,std::string = "RGBA");
        void stop();

        std::unordered_map<uint,TextureBuffer*> getBuffers();
        GLuint getTexture(uint);
};
#endif