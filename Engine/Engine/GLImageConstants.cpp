#include "GLImageConstants.h"

using namespace std;

unordered_map<uint,GLuint> _populateImageFormatMap(){
    unordered_map<uint,GLuint> m;
    
    m.emplace(uint(ImageInternalFormat::Depth16), GL_DEPTH_COMPONENT16);
    m.emplace(uint(ImageInternalFormat::Depth24), GL_DEPTH_COMPONENT24);
    m.emplace(uint(ImageInternalFormat::Depth32), GL_DEPTH_COMPONENT32);
    m.emplace(uint(ImageInternalFormat::Depth32F), GL_DEPTH_COMPONENT32F);
    m.emplace(uint(ImageInternalFormat::Depth24Stencil8), GL_DEPTH24_STENCIL8);
    m.emplace(uint(ImageInternalFormat::Depth32FStencil8), GL_DEPTH32F_STENCIL8);
    m.emplace(uint(ImageInternalFormat::RGB10_A2), GL_RGB10_A2);
    m.emplace(uint(ImageInternalFormat::RGB10_A2UI), GL_RGB10_A2UI);
    m.emplace(uint(ImageInternalFormat::R11F_G11F_B10F), GL_R11F_G11F_B10F);
    m.emplace(uint(ImageInternalFormat::SRGB8_ALPHA8), GL_SRGB8_ALPHA8);
    m.emplace(uint(ImageInternalFormat::StencilIndex8), GL_STENCIL_INDEX8);
    m.emplace(uint(ImageInternalFormat::SRGB8), GL_SRGB8);
    m.emplace(uint(ImageInternalFormat::COMPRESSED_RGB_S3TC_DXT1_EXT), GL_COMPRESSED_RGB_S3TC_DXT1_EXT);
    m.emplace(uint(ImageInternalFormat::COMPRESSED_RGBA_S3TC_DXT1_EXT), GL_COMPRESSED_RGBA_S3TC_DXT1_EXT);
    m.emplace(uint(ImageInternalFormat::COMPRESSED_RGBA_S3TC_DXT3_EXT), GL_COMPRESSED_RGBA_S3TC_DXT3_EXT);
    m.emplace(uint(ImageInternalFormat::COMPRESSED_RGBA_S3TC_DXT5_EXT), GL_COMPRESSED_RGBA_S3TC_DXT5_EXT);
    m.emplace(uint(ImageInternalFormat::COMPRESSED_SRGB_S3TC_DXT1_EXT), GL_COMPRESSED_SRGB_S3TC_DXT1_EXT);
    m.emplace(uint(ImageInternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT), GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT);
    m.emplace(uint(ImageInternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT), GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT);
    m.emplace(uint(ImageInternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT), GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT);
    m.emplace(uint(ImageInternalFormat::COMPRESSED_RED_RGTC1), GL_COMPRESSED_RED_RGTC1);
    m.emplace(uint(ImageInternalFormat::COMPRESSED_SIGNED_RED_RGTC1), GL_COMPRESSED_SIGNED_RED_RGTC1);
    m.emplace(uint(ImageInternalFormat::COMPRESSED_RG_RGTC2), GL_COMPRESSED_RG_RGTC2);
    m.emplace(uint(ImageInternalFormat::COMPRESSED_SIGNED_RG_RGTC2), GL_COMPRESSED_SIGNED_RG_RGTC2);
    //m.emplace(uint(ImageInternalFormat::COMPRESSED_RGBA_BPTC_UNORM), GL_COMPRESSED_RGBA_BPTC_UNORM);
    //m.emplace(uint(ImageInternalFormat::COMPRESSED_SRGB_ALPHA_BPTC_UNORM), GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM);
    //m.emplace(uint(ImageInternalFormat::COMPRESSED_RGB_BPTC_SIGNED_FLOAT), GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT);
    //m.emplace(uint(ImageInternalFormat::COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT), GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT);
    m.emplace(uint(ImageInternalFormat::R3_G3_B2), GL_R3_G3_B2);
    m.emplace(uint(ImageInternalFormat::RGB5_A1), GL_RGB5_A1);
    m.emplace(uint(ImageInternalFormat::RGB10_A2), GL_RGB10_A2);
    m.emplace(uint(ImageInternalFormat::RGB10_A2UI), GL_RGB10_A2UI);
    m.emplace(uint(ImageInternalFormat::R11F_G11F_B10F), GL_R11F_G11F_B10F);
    m.emplace(uint(ImageInternalFormat::RGB9_E5), GL_RGB9_E5);
    //m.emplace(uint(ImageInternalFormat::RGB565), GL_RGB565);
    m.emplace(uint(ImageInternalFormat::RGB8), GL_RGB8);
    m.emplace(uint(ImageInternalFormat::RGBA8), GL_RGBA8);
    m.emplace(uint(ImageInternalFormat::RGB16), GL_RGB16);
    m.emplace(uint(ImageInternalFormat::RGBA16), GL_RGBA16);
    m.emplace(uint(ImageInternalFormat::RGB16F), GL_RGB16F);
    m.emplace(uint(ImageInternalFormat::RGBA16F), GL_RGBA16F);
    m.emplace(uint(ImageInternalFormat::RGB32F), GL_RGB32F);
    m.emplace(uint(ImageInternalFormat::RGBA32F), GL_RGBA32F);
    m.emplace(uint(ImageInternalFormat::R8), GL_R8);
    m.emplace(uint(ImageInternalFormat::RG8), GL_RG8);
    
    return m;
}
unordered_map<uint,GLuint> _populateGLAttatchmentMap(){
    unordered_map<uint,GLuint> m;

    m.emplace(uint(FramebufferAttatchment::Color_0), GL_COLOR_ATTACHMENT0);
    m.emplace(uint(FramebufferAttatchment::Color_1), GL_COLOR_ATTACHMENT1);
    m.emplace(uint(FramebufferAttatchment::Color_2), GL_COLOR_ATTACHMENT2);
    m.emplace(uint(FramebufferAttatchment::Color_3), GL_COLOR_ATTACHMENT3);
    m.emplace(uint(FramebufferAttatchment::Color_4), GL_COLOR_ATTACHMENT4);
    m.emplace(uint(FramebufferAttatchment::Color_5), GL_COLOR_ATTACHMENT5);
    m.emplace(uint(FramebufferAttatchment::Color_6), GL_COLOR_ATTACHMENT6);
    m.emplace(uint(FramebufferAttatchment::Color_7), GL_COLOR_ATTACHMENT7);
    m.emplace(uint(FramebufferAttatchment::Color_8), GL_COLOR_ATTACHMENT8);
    m.emplace(uint(FramebufferAttatchment::Depth), GL_DEPTH_ATTACHMENT);
    m.emplace(uint(FramebufferAttatchment::Stencil), GL_STENCIL_ATTACHMENT);
    m.emplace(uint(FramebufferAttatchment::DepthAndStencil), GL_DEPTH_STENCIL_ATTACHMENT);

    return m;
}
unordered_map<uint,GLuint> ImageInternalFormat::IMAGE_INTERNAL_FORMAT_MAP = _populateImageFormatMap();
unordered_map<uint,GLuint> FramebufferAttatchment::FRAMEBUFFER_ATTATCHMENT_FORMAT_MAP = _populateGLAttatchmentMap();
