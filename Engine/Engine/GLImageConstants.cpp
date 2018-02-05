#include "GLImageConstants.h"

using namespace std;

unordered_map<uint,GLuint> ImageInternalFormat::IMAGE_INTERNAL_FORMAT_MAP = [](){
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
    m.emplace(uint(ImageInternalFormat::RG16F), GL_RG16F);
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
}();
unordered_map<uint,GLuint> ImagePixelFormat::IMAGE_PIXEL_FORMAT_MAP = [](){
    unordered_map<uint,GLuint> m;

    m.emplace(uint(ImagePixelFormat::RED),GL_RED);
    m.emplace(uint(ImagePixelFormat::RG),GL_RG);
    m.emplace(uint(ImagePixelFormat::RGB),GL_RGB);
    m.emplace(uint(ImagePixelFormat::RGBA),GL_RGBA);
    m.emplace(uint(ImagePixelFormat::BGRA),GL_BGRA);
    m.emplace(uint(ImagePixelFormat::RED_INTEGER),GL_RED_INTEGER);
    m.emplace(uint(ImagePixelFormat::RG_INTEGER),GL_RG_INTEGER);
    m.emplace(uint(ImagePixelFormat::RGB_INTEGER),GL_RGB_INTEGER);
    m.emplace(uint(ImagePixelFormat::BGR_INTEGER),GL_BGR_INTEGER);
    m.emplace(uint(ImagePixelFormat::RGBA_INTEGER),GL_RGBA_INTEGER);
    m.emplace(uint(ImagePixelFormat::BGRA_INTEGER),GL_BGRA_INTEGER);
    m.emplace(uint(ImagePixelFormat::STENCIL_INDEX),GL_STENCIL_INDEX);
    m.emplace(uint(ImagePixelFormat::DEPTH_COMPONENT),GL_DEPTH_COMPONENT);
    m.emplace(uint(ImagePixelFormat::DEPTH_STENCIL),GL_DEPTH_STENCIL);

    return m;
}();
unordered_map<uint,GLuint> ImagePixelType::IMAGE_PIXEL_TYPE_MAP = [](){
    unordered_map<uint,GLuint> m;

    m.emplace(uint(ImagePixelType::UNSIGNED_BYTE),GL_UNSIGNED_BYTE);
    m.emplace(uint(ImagePixelType::BYTE),GL_BYTE);
    m.emplace(uint(ImagePixelType::UNSIGNED_SHORT),GL_UNSIGNED_SHORT);
    m.emplace(uint(ImagePixelType::SHORT),GL_SHORT);
    m.emplace(uint(ImagePixelType::UNSIGNED_INT),GL_UNSIGNED_INT);
    m.emplace(uint(ImagePixelType::INT),GL_INT);
    m.emplace(uint(ImagePixelType::FLOAT),GL_FLOAT);
    m.emplace(uint(ImagePixelType::UNSIGNED_BYTE_3_3_2),GL_UNSIGNED_BYTE_3_3_2);
    m.emplace(uint(ImagePixelType::UNSIGNED_BYTE_2_3_3_REV),GL_UNSIGNED_BYTE_2_3_3_REV);
    m.emplace(uint(ImagePixelType::UNSIGNED_SHORT_5_6_5),GL_UNSIGNED_SHORT_5_6_5);
    m.emplace(uint(ImagePixelType::UNSIGNED_SHORT_5_6_5_REV),GL_UNSIGNED_SHORT_5_6_5_REV);
    m.emplace(uint(ImagePixelType::UNSIGNED_SHORT_4_4_4_4),GL_UNSIGNED_SHORT_4_4_4_4);
    m.emplace(uint(ImagePixelType::UNSIGNED_SHORT_4_4_4_4_REV),GL_UNSIGNED_SHORT_4_4_4_4_REV);
    m.emplace(uint(ImagePixelType::UNSIGNED_SHORT_5_5_5_1),GL_UNSIGNED_SHORT_5_5_5_1);
    m.emplace(uint(ImagePixelType::UNSIGNED_SHORT_1_5_5_5_REV),GL_UNSIGNED_SHORT_1_5_5_5_REV);
    m.emplace(uint(ImagePixelType::UNSIGNED_INT_8_8_8_8),GL_UNSIGNED_INT_8_8_8_8);
    m.emplace(uint(ImagePixelType::UNSIGNED_INT_8_8_8_8_REV),GL_UNSIGNED_INT_8_8_8_8_REV);
    m.emplace(uint(ImagePixelType::UNSIGNED_INT_10_10_10_2),GL_UNSIGNED_INT_10_10_10_2);
    m.emplace(uint(ImagePixelType::UNSIGNED_INT_2_10_10_10_REV),GL_UNSIGNED_INT_2_10_10_10_REV);
	m.emplace(uint(ImagePixelType::UNSIGNED_INT_24_8),GL_UNSIGNED_INT_24_8);

    return m;
}();
unordered_map<uint,GLuint> FramebufferAttatchment::FRAMEBUFFER_ATTATCHMENT_FORMAT_MAP = [](){
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
}();
