#pragma once
#ifndef ENGINE_FRAMEBUFFER_OBJECT_H
#define ENGINE_FRAMEBUFFER_OBJECT_H

#include "BindableResource.h"

class Texture;
typedef unsigned int uint;
class FramebufferAttatchment{public: enum Attatchment{
    Color_0, Color_1, Color_2, Color_3, Color_4, Color_5,
    Color_6, Color_7, Color_8, Depth, Stencil, DepthAndStencil
};};
//this is incomplete...
class ImageInternalFormat{public: enum Format{
    Depth16, Depth24, Depth32, Depth32F, Depth24Stencil8, Depth32FStencil8,
    RGB10_A2, RGB10_A2UI, R11F_G11F_B10F, SRGB8_ALPHA8, StencilIndex8,
    SRGB8,COMPRESSED_RGB_S3TC_DXT1_EXT,COMPRESSED_RGBA_S3TC_DXT1_EXT,
    COMPRESSED_RGBA_S3TC_DXT3_EXT,COMPRESSED_RGBA_S3TC_DXT5_EXT,
    COMPRESSED_SRGB_S3TC_DXT1_EXT,COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT,
    COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT,COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT,
    COMPRESSED_RED_RGTC1,COMPRESSED_SIGNED_RED_RGTC1,COMPRESSED_RG_RGTC2,
    COMPRESSED_SIGNED_RG_RGTC2,COMPRESSED_RGBA_BPTC_UNORM,COMPRESSED_SRGB_ALPHA_BPTC_UNORM,
    COMPRESSED_RGB_BPTC_SIGNED_FLOAT,COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT,
    R3_G3_B2,RGB5_A1,RGB10_A2,RGB10_A2UI,R11F_G11F_B10F,RGB9_E5,RGB565,
    RGB8,RGBA8,RGB16,RGBA16,RGB16F,RGBA16F,RGB32F,RGBA32F,R8,RG8
};};

class FramebufferObjectAttatchment{
    private:
        class impl;
        std::unique_ptr<impl> m_i;
    public:
        FramebufferObjectAttatchment();
        ~FramebufferObjectAttatchment();
    
        uint width();
        uint height();
};

class RenderbufferObject{
    private:
        class impl;
        std::unique_ptr<impl> m_i;
    public:
        RenderbufferObject(uint width,uint height,GLuint internalFormat);
        ~RenderbufferObject();
};

class FramebufferObject: public BindableResource{
    private:
        class impl;
        std::unique_ptr<impl> m_i;
    public:
        FramebufferObject(std::string name,uint width,uint height);
        ~FramebufferObject();

        void resize(uint,uint);
    
        void attatchTexture(Texture*,FramebufferAttatchment::Attatchment);
        void attatchRenderBuffer(RenderbufferObject*,FramebufferAttatchment::Attatchment);
    
        uint width();
        uint height();
};
#endif
