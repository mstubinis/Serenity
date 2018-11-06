#include "core/engine/GLImageConstants.h"

using namespace std;

void _add(vector<GLuint>& v,unsigned int key,GLuint value){ v[key] = value; }

vector<GLuint> ImageInternalFormat::IMAGE_INTERNAL_FORMATS = [](){
    vector<GLuint> m; m.resize(ImageInternalFormat::_TOTAL,0x0000);
    
    //Base Internal Formats - recheck these
    _add(m,ImageInternalFormat::DEPTH_COMPONENT,GL_DEPTH_COMPONENT);
    _add(m,ImageInternalFormat::DEPTH_STENCIL,GL_DEPTH_STENCIL);
    _add(m,ImageInternalFormat::STENCIL_INDEX,GL_STENCIL_INDEX);
    _add(m,ImageInternalFormat::RED,GL_RED);
    _add(m,ImageInternalFormat::RG,GL_RG);
    _add(m,ImageInternalFormat::RGB,GL_RGB);
    _add(m,ImageInternalFormat::RGBA,GL_RGBA);

    //Sized Internal Formats
    _add(m,ImageInternalFormat::R8,GL_R8);
    _add(m,ImageInternalFormat::R8_SNORM,GL_R8_SNORM);
    _add(m,ImageInternalFormat::R16,GL_R16);
    _add(m,ImageInternalFormat::R16_SNORM,GL_R16_SNORM);
    _add(m,ImageInternalFormat::RG8,GL_RG8);
    _add(m,ImageInternalFormat::RG8_SNORM,GL_RG8_SNORM);
    _add(m,ImageInternalFormat::RG16,GL_RG16);
    _add(m,ImageInternalFormat::RG16_SNORM,GL_RG16_SNORM);
    _add(m,ImageInternalFormat::R3_G3_B2,GL_R3_G3_B2);
    _add(m,ImageInternalFormat::RGB4,GL_RGB4);
    _add(m,ImageInternalFormat::RGB5,GL_RGB5);
    _add(m,ImageInternalFormat::RGB8,GL_RGB8);
    _add(m,ImageInternalFormat::RGB8_SNORM,GL_RGB8_SNORM);
    _add(m,ImageInternalFormat::RGB10,GL_RGB10);
    _add(m,ImageInternalFormat::RGB12,GL_RGB12);
    _add(m,ImageInternalFormat::RGB16_SNORM,GL_RGB16_SNORM);
    _add(m,ImageInternalFormat::RGBA2,GL_RGBA2);
    _add(m,ImageInternalFormat::RGBA4,GL_RGBA4);
    _add(m,ImageInternalFormat::RGB5_A1,GL_RGB5_A1);
    _add(m,ImageInternalFormat::RGBA8,GL_RGBA8);
    _add(m,ImageInternalFormat::RGBA8_SNORM,GL_RGBA8_SNORM);
    _add(m,ImageInternalFormat::RGB10_A2,GL_RGB10_A2);
    _add(m,ImageInternalFormat::RGB10_A2UI,GL_RGB10_A2UI);
    _add(m,ImageInternalFormat::RGBA12,GL_RGBA12);
    _add(m,ImageInternalFormat::RGBA16,GL_RGBA16);
    _add(m,ImageInternalFormat::SRGB8,GL_SRGB8);
    _add(m,ImageInternalFormat::SRGB8_ALPHA8,GL_SRGB8_ALPHA8);
    _add(m,ImageInternalFormat::R16F,GL_R16F);
    _add(m,ImageInternalFormat::RG16F,GL_RG16F);
    _add(m,ImageInternalFormat::RGB16F,GL_RGB16F);
    _add(m,ImageInternalFormat::RGBA16F,GL_RGBA16F);
    _add(m,ImageInternalFormat::R32F,GL_R32F);
    _add(m,ImageInternalFormat::RG32F,GL_RG32F);
    _add(m,ImageInternalFormat::RGB32F,GL_RGB32F);
    _add(m,ImageInternalFormat::RGBA32F,GL_RGBA32F);
    _add(m,ImageInternalFormat::R11F_G11F_B10F,GL_R11F_G11F_B10F);
    _add(m,ImageInternalFormat::RGB9_E5,GL_RGB9_E5);
    _add(m,ImageInternalFormat::R8I,GL_R8I);
    _add(m,ImageInternalFormat::R8UI,GL_R8UI);
    _add(m,ImageInternalFormat::R16I,GL_R16I);
    _add(m,ImageInternalFormat::R16UI,GL_R16UI);
    _add(m,ImageInternalFormat::R32I,GL_R32I);
    _add(m,ImageInternalFormat::R32UI,GL_R32UI);
    _add(m,ImageInternalFormat::RG8I,GL_RG8I);
    _add(m,ImageInternalFormat::RG8UI,GL_RG8UI);
    _add(m,ImageInternalFormat::RG16I,GL_RG16I);
    _add(m,ImageInternalFormat::RG16UI,GL_RG16UI);
    _add(m,ImageInternalFormat::RG32I,GL_RG32I);
    _add(m,ImageInternalFormat::RG32UI,GL_RG32UI);
    _add(m,ImageInternalFormat::RGB8I,GL_RGB8I);
    _add(m,ImageInternalFormat::RGB8UI,GL_RGB8UI);
    _add(m,ImageInternalFormat::RGB16I,GL_RGB16I);
    _add(m,ImageInternalFormat::RGB16UI,GL_RGB16UI);
    _add(m,ImageInternalFormat::RGB32I,GL_RGB32I);
    _add(m,ImageInternalFormat::RGB32UI,GL_RGB32UI);
    _add(m,ImageInternalFormat::RGBA8I,GL_RGBA8I);
    _add(m,ImageInternalFormat::RGBA8UI,GL_RGBA8UI);
    _add(m,ImageInternalFormat::RGBA16I,GL_RGBA16I);
    _add(m,ImageInternalFormat::RGBA16UI,GL_RGBA16UI);
    _add(m,ImageInternalFormat::RGBA32I,GL_RGBA32I);
    _add(m,ImageInternalFormat::RGBA32UI,GL_RGBA32UI);

    //Sized Depth and Stencil Internal Formats
    _add(m,ImageInternalFormat::Depth16, GL_DEPTH_COMPONENT16);
    _add(m,ImageInternalFormat::Depth24, GL_DEPTH_COMPONENT24);
    _add(m,ImageInternalFormat::Depth32, GL_DEPTH_COMPONENT32);
    _add(m,ImageInternalFormat::Depth32F, GL_DEPTH_COMPONENT32F);
    _add(m,ImageInternalFormat::Depth24Stencil8, GL_DEPTH24_STENCIL8);
    _add(m,ImageInternalFormat::Depth32FStencil8, GL_DEPTH32F_STENCIL8);
    _add(m,ImageInternalFormat::StencilIndex8, GL_STENCIL_INDEX8);


    //Compressed Internal Formats
    _add(m,ImageInternalFormat::COMPRESSED_RED,GL_COMPRESSED_RED);
    _add(m,ImageInternalFormat::COMPRESSED_RG,GL_COMPRESSED_RG);
    _add(m,ImageInternalFormat::COMPRESSED_RGB,GL_COMPRESSED_RGB);
    _add(m,ImageInternalFormat::COMPRESSED_RGBA,GL_COMPRESSED_RGBA);
    _add(m,ImageInternalFormat::COMPRESSED_SRGB,GL_COMPRESSED_SRGB);
    _add(m,ImageInternalFormat::COMPRESSED_SRGB_ALPHA,GL_COMPRESSED_SRGB_ALPHA);

    _add(m,ImageInternalFormat::COMPRESSED_RED_RGTC1,GL_COMPRESSED_RED_RGTC1);
    _add(m,ImageInternalFormat::COMPRESSED_SIGNED_RED_RGTC1,GL_COMPRESSED_SIGNED_RED_RGTC1);
    _add(m,ImageInternalFormat::COMPRESSED_RG_RGTC2,GL_COMPRESSED_RG_RGTC2);
    _add(m,ImageInternalFormat::COMPRESSED_SIGNED_RG_RGTC2,GL_COMPRESSED_SIGNED_RG_RGTC2);
    //_add(m,ImageInternalFormat::COMPRESSED_RGBA_BPTC_UNORM,GL_COMPRESSED_RGBA_BPTC_UNORM);
    //_add(m,ImageInternalFormat::COMPRESSED_SRGB_ALPHA_BPTC_UNORM,GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM);
    //_add(m,ImageInternalFormat::COMPRESSED_RGB_BPTC_SIGNED_FLOAT,GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT);
    //_add(m,ImageInternalFormat::COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT,GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT);
            //S3TC formats
    _add(m,ImageInternalFormat::COMPRESSED_RGB_S3TC_DXT1_EXT,GL_COMPRESSED_RGB_S3TC_DXT1_EXT);
    _add(m,ImageInternalFormat::COMPRESSED_SRGB_S3TC_DXT1_EXT,GL_COMPRESSED_SRGB_S3TC_DXT1_EXT);
    _add(m,ImageInternalFormat::COMPRESSED_RGBA_S3TC_DXT1_EXT,GL_COMPRESSED_RGBA_S3TC_DXT1_EXT);
    _add(m,ImageInternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT,GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT);
    _add(m,ImageInternalFormat::COMPRESSED_RGBA_S3TC_DXT3_EXT,GL_COMPRESSED_RGBA_S3TC_DXT3_EXT);
    _add(m,ImageInternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT,GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT);
    _add(m,ImageInternalFormat::COMPRESSED_RGBA_S3TC_DXT5_EXT,GL_COMPRESSED_RGBA_S3TC_DXT5_EXT);
    _add(m,ImageInternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT,GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT);

    return m;
}();
vector<GLuint> ImagePixelFormat::IMAGE_PIXEL_FORMATS = [](){
    vector<GLuint> m; m.resize(ImagePixelFormat::_TOTAL,0x0000);

    _add(m,ImagePixelFormat::RED,GL_RED);
    _add(m,ImagePixelFormat::RG,GL_RG);
    _add(m,ImagePixelFormat::RGB,GL_RGB);
    _add(m,ImagePixelFormat::RGBA,GL_RGBA);
    _add(m,ImagePixelFormat::BGRA,GL_BGRA);
    _add(m,ImagePixelFormat::RED_INTEGER,GL_RED_INTEGER);
    _add(m,ImagePixelFormat::RG_INTEGER,GL_RG_INTEGER);
    _add(m,ImagePixelFormat::RGB_INTEGER,GL_RGB_INTEGER);
    _add(m,ImagePixelFormat::BGR_INTEGER,GL_BGR_INTEGER);
    _add(m,ImagePixelFormat::RGBA_INTEGER,GL_RGBA_INTEGER);
    _add(m,ImagePixelFormat::BGRA_INTEGER,GL_BGRA_INTEGER);
    _add(m,ImagePixelFormat::STENCIL_INDEX,GL_STENCIL_INDEX);
    _add(m,ImagePixelFormat::DEPTH_COMPONENT,GL_DEPTH_COMPONENT);
    _add(m,ImagePixelFormat::DEPTH_STENCIL,GL_DEPTH_STENCIL);

    return m;
}();
vector<GLuint> ImagePixelType::IMAGE_PIXEL_TYPES = [](){
    vector<GLuint> m; m.resize(ImagePixelType::_TOTAL,0x0000);

    _add(m,ImagePixelType::UNSIGNED_BYTE,GL_UNSIGNED_BYTE);
    _add(m,ImagePixelType::BYTE,GL_BYTE);
    _add(m,ImagePixelType::UNSIGNED_SHORT,GL_UNSIGNED_SHORT);
    _add(m,ImagePixelType::SHORT,GL_SHORT);
    _add(m,ImagePixelType::UNSIGNED_INT,GL_UNSIGNED_INT);
    _add(m,ImagePixelType::INT,GL_INT);
    _add(m,ImagePixelType::FLOAT,GL_FLOAT);
    _add(m,ImagePixelType::UNSIGNED_BYTE_3_3_2,GL_UNSIGNED_BYTE_3_3_2);
    _add(m,ImagePixelType::UNSIGNED_BYTE_2_3_3_REV,GL_UNSIGNED_BYTE_2_3_3_REV);
    _add(m,ImagePixelType::UNSIGNED_SHORT_5_6_5,GL_UNSIGNED_SHORT_5_6_5);
    _add(m,ImagePixelType::UNSIGNED_SHORT_5_6_5_REV,GL_UNSIGNED_SHORT_5_6_5_REV);
    _add(m,ImagePixelType::UNSIGNED_SHORT_4_4_4_4,GL_UNSIGNED_SHORT_4_4_4_4);
    _add(m,ImagePixelType::UNSIGNED_SHORT_4_4_4_4_REV,GL_UNSIGNED_SHORT_4_4_4_4_REV);
    _add(m,ImagePixelType::UNSIGNED_SHORT_5_5_5_1,GL_UNSIGNED_SHORT_5_5_5_1);
    _add(m,ImagePixelType::UNSIGNED_SHORT_1_5_5_5_REV,GL_UNSIGNED_SHORT_1_5_5_5_REV);
    _add(m,ImagePixelType::UNSIGNED_INT_8_8_8_8,GL_UNSIGNED_INT_8_8_8_8);
    _add(m,ImagePixelType::UNSIGNED_INT_8_8_8_8_REV,GL_UNSIGNED_INT_8_8_8_8_REV);
    _add(m,ImagePixelType::UNSIGNED_INT_10_10_10_2,GL_UNSIGNED_INT_10_10_10_2);
    _add(m,ImagePixelType::UNSIGNED_INT_2_10_10_10_REV,GL_UNSIGNED_INT_2_10_10_10_REV);
    _add(m,ImagePixelType::UNSIGNED_INT_24_8,GL_UNSIGNED_INT_24_8);
    _add(m,ImagePixelType::FLOAT_32_UNSIGNED_INT_24_8_REV,GL_FLOAT_32_UNSIGNED_INT_24_8_REV);

    return m;
}();
vector<GLuint> FramebufferAttatchment::FRAMEBUFFER_ATTATCHMENT_FORMATS = [](){
    vector<GLuint> m; m.resize(FramebufferAttatchment::_TOTAL,0x0000);

    _add(m,FramebufferAttatchment::Color_0,  GL_COLOR_ATTACHMENT0);
    _add(m,FramebufferAttatchment::Color_1,  GL_COLOR_ATTACHMENT1);
    _add(m,FramebufferAttatchment::Color_2,  GL_COLOR_ATTACHMENT2);
    _add(m,FramebufferAttatchment::Color_3,  GL_COLOR_ATTACHMENT3);
    _add(m,FramebufferAttatchment::Color_4,  GL_COLOR_ATTACHMENT4);
    _add(m,FramebufferAttatchment::Color_5,  GL_COLOR_ATTACHMENT5);
    _add(m,FramebufferAttatchment::Color_6,  GL_COLOR_ATTACHMENT6);
    _add(m,FramebufferAttatchment::Color_7,  GL_COLOR_ATTACHMENT7);
    _add(m,FramebufferAttatchment::Color_8,  GL_COLOR_ATTACHMENT8);
    _add(m,FramebufferAttatchment::Color_9,  GL_COLOR_ATTACHMENT9);
    _add(m,FramebufferAttatchment::Color_10, GL_COLOR_ATTACHMENT10);
    _add(m,FramebufferAttatchment::Color_11, GL_COLOR_ATTACHMENT11);
    _add(m,FramebufferAttatchment::Color_12, GL_COLOR_ATTACHMENT12);
    _add(m,FramebufferAttatchment::Color_13, GL_COLOR_ATTACHMENT13);
    _add(m,FramebufferAttatchment::Color_14, GL_COLOR_ATTACHMENT14);
    _add(m,FramebufferAttatchment::Color_15, GL_COLOR_ATTACHMENT15);
    _add(m,FramebufferAttatchment::Depth, GL_DEPTH_ATTACHMENT);
    _add(m,FramebufferAttatchment::Stencil, GL_STENCIL_ATTACHMENT);
    _add(m,FramebufferAttatchment::DepthAndStencil, GL_DEPTH_STENCIL_ATTACHMENT);

    return m;
}();
