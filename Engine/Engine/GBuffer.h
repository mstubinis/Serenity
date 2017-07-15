#pragma once
#ifndef ENGINE_GBUFFER_H
#define ENGINE_GBUFFER_H

#include "Texture.h"
#include <GL/glew.h>
#include <GL/GL.h>
#include <unordered_map>

typedef unsigned int uint;

//framebuffer pixel size = window size * this value here
const float GBUFFER_DIVISIBLES[] = {1.0f, // (diffuse.rgb)
                                    1.0f, // (normals.rgb)
                                    1.0f, // Glow & SpecularMap(Greyscale) & MaterialID THEN HDR
                                    1.0f, // (lighting.rgb)
                                    0.5f, // bloom, & ssao as alpha
                                    0.5f, // gods rays
                                    0.5f, // free2 buffer
                                    1.0f};// depth

const int GBUFFER_TYPES[] =            {GL_RGB8,               // (diffuse.rgb)
                                        GL_RGB16F,             // (normals.rgb)
                                        GL_RGB8,               // Glow & SpecularMap(Greyscale) & MaterialID THEN HDR
                                        GL_RGB16F,             // (lighting.rgb)
                                        GL_RGBA8,              // bloom, & ssao as alpha
                                        GL_RGB8,               // gods rays
                                        GL_RGBA8,              // free2 buffer
                                        GL_DEPTH_COMPONENT16}; // depth

const int GBUFFER_PIXEL_TYPES[] =      {GL_RGB,                // (diffuse.rgb)
                                        GL_RGB,                // (normals.rgb)
                                        GL_RGB,                // Glow & SpecularMap(Greyscale) & MaterialID THEN HDR
                                        GL_RGB,                // (lighting.rgb)
                                        GL_RGBA,               // bloom, & ssao as alpha
                                        GL_RGB,                // gods rays
                                        GL_RGBA,               // free2 buffer
                                        GL_DEPTH_COMPONENT};   // depth

const int GBUFFER_FLOAT_TYPES[] =      {GL_UNSIGNED_BYTE,
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
                                        GL_COLOR_ATTACHMENT0,
                                        GL_COLOR_ATTACHMENT4,
                                        GL_COLOR_ATTACHMENT1,
                                        GL_DEPTH_ATTACHMENT};

enum BUFFER_TYPES {BUFFER_TYPE_DIFFUSE,
                   BUFFER_TYPE_NORMAL,
                   BUFFER_TYPE_MISC,
                   BUFFER_TYPE_LIGHTING,
                   BUFFER_TYPE_BLOOM,
                   BUFFER_TYPE_GODSRAYS,
                   BUFFER_TYPE_FREE2,
                   BUFFER_TYPE_DEPTH,
                   BUFFER_TYPE_NUMBER};


class TextureBuffer final: public Texture{
    private:
        class impl;
        std::unique_ptr<impl> m_i;
    public:
        TextureBuffer(std::string name,int,int,int,int,uint,uint,float divisor);
        ~TextureBuffer();

        const float divisor() const;
        const int attatchment() const;
};

class GBuffer final{
    private:
        class impl;
        std::unique_ptr<impl> m_i;
    public:
        GBuffer(uint w,uint h);
        ~GBuffer();

        void resize(uint w,uint h);

        void start(std::vector<uint>&,std::string = "RGBA",bool = true);
        void start(uint,std::string = "RGBA",bool = true);
        void start(uint,uint,std::string = "RGBA",bool = true);
        void start(uint,uint,uint,std::string = "RGBA",bool = true);
        void start(uint,uint,uint,uint,std::string = "RGBA",bool = true);
        void start(uint,uint,uint,uint,uint,std::string = "RGBA",bool = true);
        void start(uint,uint,uint,uint,uint,uint,std::string = "RGBA",bool = true);
        void stop();

        const std::unordered_map<uint,boost::weak_ptr<TextureBuffer>>& getBuffers() const;
        Texture* getTexture(uint);

        GLuint& getMainFBO();
        GLuint& getSmallFBO();
};
#endif