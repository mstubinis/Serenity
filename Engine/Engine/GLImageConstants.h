#pragma once
#ifndef ENGINE_GL_IMAGE_CONSTANTS_H
#define ENGINE_GL_IMAGE_CONSTANTS_H

#include <vector>
#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

/*
Specifies the data type of the pixel data.
The following symbolic values are accepted: GL_UNSIGNED_BYTE, GL_BYTE, GL_UNSIGNED_SHORT, GL_SHORT,
GL_UNSIGNED_INT, GL_INT, GL_FLOAT, GL_UNSIGNED_BYTE_3_3_2, GL_UNSIGNED_BYTE_2_3_3_REV, GL_UNSIGNED_SHORT_5_6_5,
GL_UNSIGNED_SHORT_5_6_5_REV, GL_UNSIGNED_SHORT_4_4_4_4, GL_UNSIGNED_SHORT_4_4_4_4_REV, GL_UNSIGNED_SHORT_5_5_5_1,
GL_UNSIGNED_SHORT_1_5_5_5_REV, GL_UNSIGNED_INT_8_8_8_8, GL_UNSIGNED_INT_8_8_8_8_REV, GL_UNSIGNED_INT_10_10_10_2,
and GL_UNSIGNED_INT_2_10_10_10_REV.
*/
class ImagePixelType{
    public: enum Type{
        #pragma region ImagePixelTypes
        UNSIGNED_BYTE,
        BYTE,
        UNSIGNED_SHORT,
        SHORT,
        UNSIGNED_INT,
        INT,
        FLOAT,
        UNSIGNED_BYTE_3_3_2,
        UNSIGNED_BYTE_2_3_3_REV,
        UNSIGNED_SHORT_5_6_5,
        UNSIGNED_SHORT_5_6_5_REV,
        UNSIGNED_SHORT_4_4_4_4,
        UNSIGNED_SHORT_4_4_4_4_REV,
        UNSIGNED_SHORT_5_5_5_1,
        UNSIGNED_SHORT_1_5_5_5_REV,
        UNSIGNED_INT_8_8_8_8,
        UNSIGNED_INT_8_8_8_8_REV,
        UNSIGNED_INT_10_10_10_2,
        UNSIGNED_INT_2_10_10_10_REV,
        UNSIGNED_INT_24_8,
        FLOAT_32_UNSIGNED_INT_24_8_REV,
        #pragma endregion
    _TOTAL};
    static std::vector<GLuint> IMAGE_PIXEL_TYPES;
    static inline GLuint at(ImagePixelType::Type t){ return IMAGE_PIXEL_TYPES[t]; }
};
/*
Specifies the format of the pixel data.
For transfers of depth, stencil, or depth/stencil data, you must use GL_DEPTH_COMPONENT, GL_STENCIL_INDEX, or GL_DEPTH_STENCIL, where appropriate.
For transfers of normalized integer or floating-point color image data, you must use one of the following: GL_RED, GL_GREEN, GL_BLUE, GL_RG, GL_RGB, GL_BGR, GL_RGBA, and GL_BGRA.
For transfers of non-normalized integer data, you must use one of the following: GL_RED_INTEGER, GL_GREEN_INTEGER, GL_BLUE_INTEGER, GL_RG_INTEGER, GL_RGB_INTEGER, GL_BGR_INTEGER, GL_RGBA_INTEGER, and GL_BGRA_INTEGER.
*/
class ImagePixelFormat{
    public: enum Format{
        #pragma region ImagePixelFormats
        RED,
        RG,
        RGB,
        BGR,
        RGBA,
        BGRA,
        RED_INTEGER,
        RG_INTEGER,
        RGB_INTEGER,
        BGR_INTEGER,
        RGBA_INTEGER,
        BGRA_INTEGER,
        STENCIL_INDEX,
        DEPTH_COMPONENT,
        DEPTH_STENCIL,
        #pragma endregion
    _TOTAL};
    static std::vector<GLuint> IMAGE_PIXEL_FORMATS;
    static inline GLuint at(ImagePixelFormat::Format f){ return IMAGE_PIXEL_FORMATS[f]; }
};
/*
Specifies the number of color components in the texture.
Must be one of base internal formats given in Table 1,
one of the sized internal formats given in Table 2,
or one of the compressed internal formats given in Table 3, below.
https://www.khronos.org/opengl/wiki/GLAPI/glTexImage2D
*/
class ImageInternalFormat{
    public: enum Format{
        #pragma region ImageInternalFormats
        //Base Internal Formats - recheck these
        DEPTH_COMPONENT,
        DEPTH_STENCIL,
        STENCIL_INDEX,
        RED,
        RG,
        RGB,
        RGBA,

        //Sized Internal Formats
        R8,
        R8_SNORM,
        R16,
        R16_SNORM,
        RG8,
        RG8_SNORM,
        RG16,
        RG16_SNORM,
        R3_G3_B2,
        RGB4,
        RGB5,
        RGB8,
        RGB8_SNORM,
        RGB10,
        RGB12,
        RGB16_SNORM,
        RGBA2,
        RGBA4,
        RGB5_A1,
        RGBA8,
        RGBA8_SNORM,
        RGB10_A2,
        RGB10_A2UI,
        RGBA12,
        RGBA16,
        SRGB8,
        SRGB8_ALPHA8,
        R16F,
        RG16F,
        RGB16F,
        RGBA16F,
        R32F,
        RG32F,
        RGB32F,
        RGBA32F,
        R11F_G11F_B10F,
        RGB9_E5,
        R8I,
        R8UI,
        R16I,
        R16UI,
        R32I,
        R32UI,
        RG8I,
        RG8UI,
        RG16I,
        RG16UI,
        RG32I,
        RG32UI,
        RGB8I,
        RGB8UI,
        RGB16I,
        RGB16UI,
        RGB32I,
        RGB32UI,
        RGBA8I,
        RGBA8UI,
        RGBA16I,
        RGBA16UI,
        RGBA32I,
        RGBA32UI,

        //Sized Depth and Stencil Internal Formats
        Depth16,
        Depth24,
        Depth32,
        Depth32F,
        Depth24Stencil8,
        Depth32FStencil8,
        StencilIndex8,

        //Compressed Internal Formats
        COMPRESSED_RED,
        COMPRESSED_RG,
        COMPRESSED_RGB,
        COMPRESSED_RGBA,
        COMPRESSED_SRGB,
        COMPRESSED_SRGB_ALPHA,
        COMPRESSED_RED_RGTC1,
        COMPRESSED_SIGNED_RED_RGTC1,
        COMPRESSED_RG_RGTC2,
        COMPRESSED_SIGNED_RG_RGTC2,
        //COMPRESSED_RGBA_BPTC_UNORM,
        //COMPRESSED_SRGB_ALPHA_BPTC_UNORM,
        //COMPRESSED_RGB_BPTC_SIGNED_FLOAT,
        //COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT,
                //S3TC formats
        COMPRESSED_RGB_S3TC_DXT1_EXT,
        COMPRESSED_SRGB_S3TC_DXT1_EXT,
        COMPRESSED_RGBA_S3TC_DXT1_EXT,
        COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT,
        COMPRESSED_RGBA_S3TC_DXT3_EXT,
        COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT,
        COMPRESSED_RGBA_S3TC_DXT5_EXT,
        COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT,
        #pragma endregion
    _TOTAL};
    static std::vector<GLuint> IMAGE_INTERNAL_FORMATS;
    static inline GLuint at(ImageInternalFormat::Format f){ return IMAGE_INTERNAL_FORMATS[f]; }
};
class FramebufferAttatchment{
    public: enum Attatchment{
        #pragma region FramebufferAttatchments
        Color_0,
        Color_1,
        Color_2,
        Color_3,
        Color_4,
        Color_5,
        Color_6,
        Color_7,
        Color_8,
        Color_9,
        Color_10,
        Color_11,
        Color_12,
        Color_13,
        Color_14,
        Color_15,
        Depth,
        Stencil,
        DepthAndStencil,
        #pragma endregion
    _TOTAL};
    static std::vector<GLuint> FRAMEBUFFER_ATTATCHMENT_FORMATS;
    static inline GLuint at(FramebufferAttatchment::Attatchment a){ return FRAMEBUFFER_ATTATCHMENT_FORMATS[a]; }
};
#endif
