#pragma once
#ifndef ENGINE_GL_IMAGE_CONSTANTS_H
#define ENGINE_GL_IMAGE_CONSTANTS_H

#include <unordered_map>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

typedef unsigned int uint;

class ImagePixelFormat{
	public: enum Format{
        RED,RG,RGB,BGR,RGBA,BGRA,RED_INTEGER,RG_INTEGER,RGB_INTEGER,BGR_INTEGER,RGBA_INTEGER,
        BGRA_INTEGER,STENCIL_INDEX,DEPTH_COMPONENT,DEPTH_STENCIL
    };
    public: static std::unordered_map<uint,GLuint> IMAGE_PIXEL_FORMAT_MAP;
            static GLuint at(ImagePixelFormat::Format format){ return IMAGE_PIXEL_FORMAT_MAP.at(uint(format)); }
};

class ImageInternalFormat{
    public: enum Format{//this is incomplete...
        Depth16,Depth24,Depth32,Depth32F,Depth24Stencil8,Depth32FStencil8,RGB10_A2,RGB10_A2UI,
        R11F_G11F_B10F, SRGB8_ALPHA8, StencilIndex8,SRGB8,COMPRESSED_RGB_S3TC_DXT1_EXT,COMPRESSED_RGBA_S3TC_DXT1_EXT,
        COMPRESSED_RGBA_S3TC_DXT3_EXT,COMPRESSED_RGBA_S3TC_DXT5_EXT,COMPRESSED_SRGB_S3TC_DXT1_EXT,COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT,
        COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT,COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT,COMPRESSED_RED_RGTC1,COMPRESSED_SIGNED_RED_RGTC1,COMPRESSED_RG_RGTC2,
        COMPRESSED_SIGNED_RG_RGTC2,COMPRESSED_RGBA_BPTC_UNORM,COMPRESSED_SRGB_ALPHA_BPTC_UNORM,COMPRESSED_RGB_BPTC_SIGNED_FLOAT,
        COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT,R3_G3_B2,RGB5_A1,RGB9_E5,RGB565,RGB8,RGBA8,RGB16,RGBA16,RGB16F,RGBA16F,RGB32F,RGBA32F,R8,RG8
    };
    public: static std::unordered_map<uint,GLuint> IMAGE_INTERNAL_FORMAT_MAP;
            static GLuint at(ImageInternalFormat::Format format){ return IMAGE_INTERNAL_FORMAT_MAP.at(uint(format)); }
};
class FramebufferAttatchment{
    public: enum Attatchment{
        Color_0,Color_1,Color_2,Color_3,Color_4,Color_5,Color_6,Color_7,Color_8,Depth,Stencil,DepthAndStencil
    };
    public: static std::unordered_map<uint,GLuint> FRAMEBUFFER_ATTATCHMENT_FORMAT_MAP;
            static GLuint at(FramebufferAttatchment::Attatchment attatchment){ return FRAMEBUFFER_ATTATCHMENT_FORMAT_MAP.at(uint(attatchment)); }
};
#endif
