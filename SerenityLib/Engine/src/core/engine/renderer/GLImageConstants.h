#pragma once
#ifndef ENGINE_GL_IMAGE_CONSTANTS_H
#define ENGINE_GL_IMAGE_CONSTANTS_H

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
enum class ImagePixelType : unsigned int {
    Unknown = 0,
    #pragma region ImagePixelTypes
    UNSIGNED_BYTE                  = GL_UNSIGNED_BYTE,
    BYTE                           = GL_BYTE,
    UNSIGNED_SHORT                 = GL_UNSIGNED_SHORT,
    SHORT                          = GL_SHORT,
    UNSIGNED_INT                   = GL_UNSIGNED_INT,
    INT                            = GL_INT,
    FLOAT                          = GL_FLOAT,
    UNSIGNED_BYTE_3_3_2            = GL_UNSIGNED_BYTE_3_3_2,
    UNSIGNED_BYTE_2_3_3_REV        = GL_UNSIGNED_BYTE_2_3_3_REV,
    UNSIGNED_SHORT_5_6_5           = GL_UNSIGNED_SHORT_5_6_5,
    UNSIGNED_SHORT_5_6_5_REV       = GL_UNSIGNED_SHORT_5_6_5_REV,
    UNSIGNED_SHORT_4_4_4_4         = GL_UNSIGNED_SHORT_4_4_4_4,
    UNSIGNED_SHORT_4_4_4_4_REV     = GL_UNSIGNED_SHORT_4_4_4_4_REV,
    UNSIGNED_SHORT_5_5_5_1         = GL_UNSIGNED_SHORT_5_5_5_1,
    UNSIGNED_SHORT_1_5_5_5_REV     = GL_UNSIGNED_SHORT_1_5_5_5_REV,
    UNSIGNED_INT_8_8_8_8           = GL_UNSIGNED_INT_8_8_8_8,
    UNSIGNED_INT_8_8_8_8_REV       = GL_UNSIGNED_INT_8_8_8_8_REV,
    UNSIGNED_INT_10_10_10_2        = GL_UNSIGNED_INT_10_10_10_2,
    UNSIGNED_INT_2_10_10_10_REV    = GL_UNSIGNED_INT_2_10_10_10_REV,
    UNSIGNED_INT_24_8              = GL_UNSIGNED_INT_24_8,
    FLOAT_32_UNSIGNED_INT_24_8_REV = GL_FLOAT_32_UNSIGNED_INT_24_8_REV,
    #pragma endregion
};
/*
Specifies the format of the pixel data.
For transfers of depth, stencil, or depth/stencil data, you must use GL_DEPTH_COMPONENT, GL_STENCIL_INDEX, or GL_DEPTH_STENCIL, where appropriate.
For transfers of normalized integer or floating-point color image data, you must use one of the following: GL_RED, GL_GREEN, GL_BLUE, GL_RG, GL_RGB, GL_BGR, GL_RGBA, and GL_BGRA.
For transfers of non-normalized integer data, you must use one of the following: GL_RED_INTEGER, GL_GREEN_INTEGER, GL_BLUE_INTEGER, GL_RG_INTEGER, GL_RGB_INTEGER, GL_BGR_INTEGER, GL_RGBA_INTEGER, and GL_BGRA_INTEGER.
*/
enum class ImagePixelFormat : unsigned int {
    Unknown = 0,
    #pragma region ImagePixelFormats
    RED             = GL_RED,
    RG              = GL_RG,
    RGB             = GL_RGB,
    BGR             = GL_BGR,
    RGBA            = GL_RGBA,
    BGRA            = GL_BGRA,
    RED_INTEGER     = GL_RED_INTEGER,
    RG_INTEGER      = GL_RG_INTEGER,
    RGB_INTEGER     = GL_RGB_INTEGER,
    BGR_INTEGER     = GL_BGR_INTEGER,
    RGBA_INTEGER    = GL_RGBA_INTEGER,
    BGRA_INTEGER    = GL_BGRA_INTEGER,
    STENCIL_INDEX   = GL_STENCIL_INDEX,
    DEPTH_COMPONENT = GL_DEPTH_COMPONENT,
    DEPTH_STENCIL   = GL_DEPTH_STENCIL,
    #pragma endregion
};
/*
Specifies the number of color components in the texture.
Must be one of base internal formats given in Table 1,
one of the sized internal formats given in Table 2,
or one of the compressed internal formats given in Table 3, below.
https://www.khronos.org/opengl/wiki/GLAPI/glTexImage2D
*/
enum class ImageInternalFormat : unsigned int {
    Unknown = 0,
    #pragma region ImageInternalFormats
    //Base Internal Formats - recheck these
    DEPTH_COMPONENT                      = GL_DEPTH_COMPONENT,
    DEPTH_STENCIL                        = GL_DEPTH_STENCIL,
    STENCIL_INDEX                        = GL_STENCIL_INDEX,
    RED                                  = GL_RED,
    RG                                   = GL_RG,
    RGB                                  = GL_RGB,
    RGBA                                 = GL_RGBA,

    //Sized Internal Formats
    R8                                   = GL_R8,
    R8_SNORM                             = GL_R8_SNORM,
    R16                                  = GL_R16,
    R16_SNORM                            = GL_R16_SNORM,
    RG8                                  = GL_RG8,
    RG8_SNORM                            = GL_RG8_SNORM,
    RG16                                 = GL_RG16,
    RG16_SNORM                           = GL_RG16_SNORM,
    R3_G3_B2                             = GL_R3_G3_B2,
    RGB4                                 = GL_RGB4,
    RGB5                                 = GL_RGB5,
    RGB8                                 = GL_RGB8,
    RGB8_SNORM                           = GL_RGB8_SNORM,
    RGB10                                = GL_RGB10,
    RGB12                                = GL_RGB12,
    RGB16_SNORM                          = GL_RGB16_SNORM,
    RGBA2                                = GL_RGBA2,
    RGBA4                                = GL_RGBA4,
    RGB5_A1                              = GL_RGB5_A1,
    RGBA8                                = GL_RGBA8,
    RGBA8_SNORM                          = GL_RGBA8_SNORM,
    RGB10_A2                             = GL_RGB10_A2,
    RGB10_A2UI                           = GL_RGB10_A2UI,
    RGBA12                               = GL_RGBA12,
    RGBA16                               = GL_RGBA16,
    SRGB8                                = GL_SRGB8,
    SRGB8_ALPHA8                         = GL_SRGB8_ALPHA8,
    R16F                                 = GL_R16F,
    RG16F                                = GL_RG16F,
    RGB16F                               = GL_RGB16F,
    RGBA16F                              = GL_RGBA16F,
    R32F                                 = GL_R32F,
    RG32F                                = GL_RG32F,
    RGB32F                               = GL_RGB32F,
    RGBA32F                              = GL_RGBA32F,
    R11F_G11F_B10F                       = GL_R11F_G11F_B10F,
    RGB9_E5                              = GL_RGB9_E5,
    R8I                                  = GL_R8I,
    R8UI                                 = GL_R8UI,
    R16I                                 = GL_R16I,
    R16UI                                = GL_R16UI,
    R32I                                 = GL_R32I,
    R32UI                                = GL_R32UI,
    RG8I                                 = GL_RG8I,
    RG8UI                                = GL_RG8UI,
    RG16I                                = GL_RG16I,
    RG16UI                               = GL_RG16UI,
    RG32I                                = GL_RG32I,
    RG32UI                               = GL_RG32UI,
    RGB8I                                = GL_RGB8I,
    RGB8UI                               = GL_RGB8UI,
    RGB16I                               = GL_RGB16I,
    RGB16UI                              = GL_RGB16UI,
    RGB32I                               = GL_RGB32I,
    RGB32UI                              = GL_RGB32UI,
    RGBA8I                               = GL_RGBA8I,
    RGBA8UI                              = GL_RGBA8UI,
    RGBA16I                              = GL_RGBA16I,
    RGBA16UI                             = GL_RGBA16UI,
    RGBA32I                              = GL_RGBA32I,
    RGBA32UI                             = GL_RGBA32UI,

    //Sized Depth and Stencil Internal Formats
    Depth16                              = GL_DEPTH_COMPONENT16,
    Depth24                              = GL_DEPTH_COMPONENT24,
    Depth32                              = GL_DEPTH_COMPONENT32,
    Depth32F                             = GL_DEPTH_COMPONENT32F,
    Depth24Stencil8                      = GL_DEPTH24_STENCIL8,
    Depth32FStencil8                     = GL_DEPTH32F_STENCIL8,
    StencilIndex8                        = GL_STENCIL_INDEX8,

    //Compressed Internal Formats
    COMPRESSED_RED                       = GL_COMPRESSED_RED,
    COMPRESSED_RG                        = GL_COMPRESSED_RG,
    COMPRESSED_RGB                       = GL_COMPRESSED_RGB,
    COMPRESSED_RGBA                      = GL_COMPRESSED_RGBA,
    COMPRESSED_SRGB                      = GL_COMPRESSED_SRGB,
    COMPRESSED_SRGB_ALPHA                = GL_COMPRESSED_SRGB_ALPHA,
    COMPRESSED_RED_RGTC1                 = GL_COMPRESSED_RED_RGTC1,
    COMPRESSED_SIGNED_RED_RGTC1          = GL_COMPRESSED_SIGNED_RED_RGTC1,
    COMPRESSED_RG_RGTC2                  = GL_COMPRESSED_RG_RGTC2,
    COMPRESSED_SIGNED_RG_RGTC2           = GL_COMPRESSED_SIGNED_RG_RGTC2,
    //COMPRESSED_RGBA_BPTC_UNORM         = GL_COMPRESSED_RGBA_BPTC_UNORM,
    //COMPRESSED_SRGB_ALPHA_BPTC_UNORM   = GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM,
    //COMPRESSED_RGB_BPTC_SIGNED_FLOAT   = GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT,
    //COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT = GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT,
            //S3TC formats
    COMPRESSED_RGB_S3TC_DXT1_EXT         = GL_COMPRESSED_RGB_S3TC_DXT1_EXT,
    COMPRESSED_SRGB_S3TC_DXT1_EXT        = GL_COMPRESSED_SRGB_S3TC_DXT1_EXT,
    COMPRESSED_RGBA_S3TC_DXT1_EXT        = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT,
    COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT  = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT,
    COMPRESSED_RGBA_S3TC_DXT3_EXT        = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT,
    COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT  = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT,
    COMPRESSED_RGBA_S3TC_DXT5_EXT        = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT,
    COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT  = GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT,
    #pragma endregion
};
enum class FramebufferAttatchment : unsigned int {
    Unknown = 0,
    #pragma region FramebufferAttatchments
    Color_0         = GL_COLOR_ATTACHMENT0,
    Color_1         = GL_COLOR_ATTACHMENT1,
    Color_2         = GL_COLOR_ATTACHMENT2,
    Color_3         = GL_COLOR_ATTACHMENT3,
    Color_4         = GL_COLOR_ATTACHMENT4,
    Color_5         = GL_COLOR_ATTACHMENT5,
    Color_6         = GL_COLOR_ATTACHMENT6,
    Color_7         = GL_COLOR_ATTACHMENT7,
    Color_8         = GL_COLOR_ATTACHMENT8,
    Color_9         = GL_COLOR_ATTACHMENT9,
    Color_10        = GL_COLOR_ATTACHMENT10,
    Color_11        = GL_COLOR_ATTACHMENT11,
    Color_12        = GL_COLOR_ATTACHMENT12,
    Color_13        = GL_COLOR_ATTACHMENT13,
    Color_14        = GL_COLOR_ATTACHMENT14,
    Color_15        = GL_COLOR_ATTACHMENT15,
    Depth           = GL_DEPTH_ATTACHMENT,
    Stencil         = GL_STENCIL_ATTACHMENT,
    DepthAndStencil = GL_DEPTH_STENCIL_ATTACHMENT,
    #pragma endregion
};
#endif
