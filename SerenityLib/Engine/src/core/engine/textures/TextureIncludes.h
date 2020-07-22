#pragma once
#ifndef ENGINE_TEXTURE_INCLUDES_H
#define ENGINE_TEXTURE_INCLUDES_H

#include <core/engine/renderer/GLImageConstants.h>

enum class TextureWrap : unsigned int {
    Repeat, 
    RepeatMirrored, 
    ClampToEdge, 
    ClampToBorder,
};
enum class TextureFilter : unsigned int {
    Linear,
    Nearest,
    Nearest_Mipmap_Nearest,
    Nearest_Mipmap_Linear,
    Linear_Mipmap_Nearest,
    Linear_Mipmap_Linear,
};
enum class TextureType : unsigned int {
    Unknown,
    Texture1D,
    Texture2D,
    Texture3D,
    CubeMap,
    RenderTarget,
};

class Texture;
namespace sf { 
    class Image; 
}
namespace Engine::priv {
    class  FramebufferObject;
};

namespace Engine::priv {
    struct ImageMipmap final {
        std::uint32_t               width          = 0U;
        std::uint32_t               height         = 0U;
        std::uint32_t               compressedSize = 0U;
        std::uint32_t               level          = 0U;
        std::vector<std::uint8_t>   pixels;
    };
    struct ImageLoadedStructure final {
        ImageInternalFormat             m_InternalFormat  = ImageInternalFormat::Unknown;
        ImagePixelFormat                m_PixelFormat     = ImagePixelFormat::Unknown;
        ImagePixelType                  m_PixelType       = ImagePixelType::Unknown;
        std::vector<priv::ImageMipmap>  m_Mipmaps         = { ImageMipmap() };
        std::string                     m_Filename        = "";

        ImageLoadedStructure() = default;
        ImageLoadedStructure(unsigned int width, unsigned int height, ImagePixelType pixelType, ImagePixelFormat pixelFormat, ImageInternalFormat internalFormat);
        ImageLoadedStructure(const sf::Image& sfImage, const std::string& filename = "");
        ~ImageLoadedStructure() = default;

        ImageLoadedStructure(const ImageLoadedStructure&)                      = delete;
        ImageLoadedStructure& operator=(const ImageLoadedStructure&)           = delete;
        ImageLoadedStructure(ImageLoadedStructure&& other) noexcept;
        ImageLoadedStructure& operator=(ImageLoadedStructure&& other) noexcept;

        void load(unsigned int width, unsigned int height, ImagePixelType pixelType, ImagePixelFormat pixelFormat, ImageInternalFormat internalFormat);
        void load(const sf::Image& sfImage, const std::string& filename = "");
    };
};


#endif