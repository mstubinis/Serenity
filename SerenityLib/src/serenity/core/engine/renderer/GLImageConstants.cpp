
#include <serenity/core/engine/renderer/GLImageConstants.h>

ImageInternalFormat::operator ImagePixelFormat() const noexcept {
    switch (m_Type) {
        case ImageInternalFormat::COMPRESSED_RED: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RED;
        }case ImageInternalFormat::COMPRESSED_RED_RGTC1: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RED; //recheck this
        }case ImageInternalFormat::COMPRESSED_RG: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RG;
        }case ImageInternalFormat::COMPRESSED_RGB: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGB;
        }case ImageInternalFormat::COMPRESSED_RGBA: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGBA;
        }case ImageInternalFormat::COMPRESSED_RGBA_S3TC_DXT1_EXT: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGBA;
        }case ImageInternalFormat::COMPRESSED_RGBA_S3TC_DXT3_EXT: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGBA;
        }case ImageInternalFormat::COMPRESSED_RGBA_S3TC_DXT5_EXT: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGBA;
        }case ImageInternalFormat::COMPRESSED_RGB_S3TC_DXT1_EXT: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGB;
        }case ImageInternalFormat::COMPRESSED_RG_RGTC2: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RG; //recheck this
        }case ImageInternalFormat::COMPRESSED_SIGNED_RED_RGTC1: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RED; //recheck this 
        }case ImageInternalFormat::COMPRESSED_SIGNED_RG_RGTC2: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RG; //recheck this
        }case ImageInternalFormat::COMPRESSED_SRGB: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGB;
        }case ImageInternalFormat::COMPRESSED_SRGB_ALPHA: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGBA;
        }case ImageInternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGBA;
        }case ImageInternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGBA;
        }case ImageInternalFormat::COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGBA;
        }case ImageInternalFormat::COMPRESSED_SRGB_S3TC_DXT1_EXT: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGB;
        }case ImageInternalFormat::Depth16: {
            return (ImagePixelFormat)ImagePixelFormat::Type::DEPTH_COMPONENT;
        }case ImageInternalFormat::Depth24: {
            return (ImagePixelFormat)ImagePixelFormat::Type::DEPTH_COMPONENT;
        }case ImageInternalFormat::Depth32: {
            return (ImagePixelFormat)ImagePixelFormat::Type::DEPTH_COMPONENT;
        }case ImageInternalFormat::Depth32F: {
            return (ImagePixelFormat)ImagePixelFormat::Type::DEPTH_COMPONENT;
        }case ImageInternalFormat::Depth24Stencil8: {
            return (ImagePixelFormat)ImagePixelFormat::Type::DEPTH_STENCIL;
        }case ImageInternalFormat::Depth32FStencil8: {
            return (ImagePixelFormat)ImagePixelFormat::Type::DEPTH_STENCIL;
        }case ImageInternalFormat::DEPTH_COMPONENT: {
            return (ImagePixelFormat)ImagePixelFormat::Type::DEPTH_COMPONENT;
        }case ImageInternalFormat::DEPTH_STENCIL: {
            return (ImagePixelFormat)ImagePixelFormat::Type::DEPTH_STENCIL;
        }case ImageInternalFormat::R11F_G11F_B10F: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGB;
        }case ImageInternalFormat::R16: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RED;
        }case ImageInternalFormat::R16F: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RED;
        }case ImageInternalFormat::R16I: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RED_INTEGER;
        }case ImageInternalFormat::R16UI: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RED_INTEGER;
        }case ImageInternalFormat::R16_SNORM: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RED;
        }case ImageInternalFormat::R32F: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RED;
        }case ImageInternalFormat::R32I: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RED_INTEGER;
        }case ImageInternalFormat::R32UI: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RED_INTEGER;
        }case ImageInternalFormat::R3_G3_B2: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGB;
        }case ImageInternalFormat::R8: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RED;
        }case ImageInternalFormat::R8I: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RED_INTEGER;
        }case ImageInternalFormat::R8UI: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RED_INTEGER;
        }case ImageInternalFormat::R8_SNORM: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RED;
        }case ImageInternalFormat::RED: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RED;
        }case ImageInternalFormat::RG: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RG;
        }case ImageInternalFormat::RG16: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RG;
        }case ImageInternalFormat::RG16F: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RG;
        }case ImageInternalFormat::RG16I: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RG_INTEGER;
        }case ImageInternalFormat::RG16UI: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RG_INTEGER;
        }case ImageInternalFormat::RG16_SNORM: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RG;
        }case ImageInternalFormat::RG32F: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RG;
        }case ImageInternalFormat::RG32I: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RG_INTEGER;
        }case ImageInternalFormat::RG32UI: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RG_INTEGER;
        }case ImageInternalFormat::RG8: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RG;
        }case ImageInternalFormat::RG8I: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RG_INTEGER;
        }case ImageInternalFormat::RG8UI: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RG_INTEGER;
        }case ImageInternalFormat::RG8_SNORM: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RG;
        }case ImageInternalFormat::RGB: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGB;
        }case ImageInternalFormat::RGB10: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGB;
        }case ImageInternalFormat::RGB10_A2: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGBA;
        }case ImageInternalFormat::RGB12: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGB;
        }case ImageInternalFormat::RGB16F: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGB;
        }case ImageInternalFormat::RGB16I: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGB_INTEGER;
        }case ImageInternalFormat::RGB16UI: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGB_INTEGER;
        }case ImageInternalFormat::RGB16_SNORM: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGB;
        }case ImageInternalFormat::RGB32F: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGB;
        }case ImageInternalFormat::RGB32I: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGB_INTEGER;
        }case ImageInternalFormat::RGB32UI: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGB_INTEGER;
        }case ImageInternalFormat::RGB4: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGB;
        }case ImageInternalFormat::RGB5: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGB;
        }case ImageInternalFormat::RGB5_A1: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGBA;
        }case ImageInternalFormat::RGB8: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGB;
        }case ImageInternalFormat::RGB8I: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGB_INTEGER;
        }case ImageInternalFormat::RGB8UI: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGB_INTEGER;
        }case ImageInternalFormat::RGB8_SNORM: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGB;
        }case ImageInternalFormat::RGB9_E5: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGB; //recheck this 
        }case ImageInternalFormat::RGBA: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGBA;
        }case ImageInternalFormat::RGBA12: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGBA;
        }case ImageInternalFormat::RGBA16: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGBA;
        }case ImageInternalFormat::RGBA16F: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGBA;
        }case ImageInternalFormat::RGBA16I: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGBA_INTEGER;
        }case ImageInternalFormat::RGBA16UI: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGBA_INTEGER;
        }case ImageInternalFormat::RGBA2: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGBA;
        }case ImageInternalFormat::RGBA32F: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGBA;
        }case ImageInternalFormat::RGBA32I: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGBA_INTEGER;
        }case ImageInternalFormat::RGBA32UI: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGBA_INTEGER;
        }case ImageInternalFormat::RGBA4: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGBA;
        }case ImageInternalFormat::RGBA8: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGBA;
        }case ImageInternalFormat::RGBA8I: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGBA_INTEGER;
        }case ImageInternalFormat::RGBA8UI: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGBA_INTEGER;
        }case ImageInternalFormat::RGBA8_SNORM: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGBA;
        }case ImageInternalFormat::SRGB8: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGB;
        }case ImageInternalFormat::SRGB8_ALPHA8: {
            return (ImagePixelFormat)ImagePixelFormat::Type::RGBA;
        }case ImageInternalFormat::StencilIndex8: {
            return (ImagePixelFormat)ImagePixelFormat::Type::STENCIL_INDEX;
        }case ImageInternalFormat::STENCIL_INDEX: {
            return (ImagePixelFormat)ImagePixelFormat::Type::STENCIL_INDEX;
        }
    }
    return (ImagePixelFormat)ImagePixelFormat::Type::Unknown;
}