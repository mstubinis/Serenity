#pragma once
#ifndef ENGINE_TEXTURE_INCLUDES_H
#define ENGINE_TEXTURE_INCLUDES_H

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
    Unknown   = 0,
    Texture1D = 1,
    Texture2D,
    Texture3D,
    CubeMap,
    RenderTarget,
_TOTAL};

class Texture;
namespace sf { 
    class Image; 
}
namespace Engine::priv {
    class  FramebufferObject;
};

#include <core/engine/renderer/GLImageConstants.h>

namespace Engine::priv {
    struct ImageMipmap final {
        std::vector<std::uint8_t>   pixels;
        int                         width          = 0U;
        int                         height         = 0U;
        std::uint32_t               compressedSize = 0U;
        int                         level          = 0U;
    };
    struct ImageLoadedStructure final {
        std::vector<priv::ImageMipmap>  m_Mipmaps         = { ImageMipmap() };
        std::string                     m_Filename        = "";
        ImageInternalFormat             m_InternalFormat  = ImageInternalFormat::Unknown;
        ImagePixelFormat                m_PixelFormat     = ImagePixelFormat::Unknown;
        ImagePixelType                  m_PixelType       = ImagePixelType::Unknown;

        ImageLoadedStructure() = default;
        ImageLoadedStructure(int width, int height, ImagePixelType pixelType, ImagePixelFormat pixelFormat, ImageInternalFormat internalFormat);
        ImageLoadedStructure(const sf::Image& sfImage, const std::string& filename = "");
        ~ImageLoadedStructure() = default;

        ImageLoadedStructure(const ImageLoadedStructure&)                      = delete;
        ImageLoadedStructure& operator=(const ImageLoadedStructure&)           = delete;
        ImageLoadedStructure(ImageLoadedStructure&& other) noexcept            = default;
        ImageLoadedStructure& operator=(ImageLoadedStructure&& other) noexcept = default;

        void load(int width, int height, ImagePixelType pixelType, ImagePixelFormat pixelFormat, ImageInternalFormat internalFormat);
        void load(const sf::Image& sfImage, const std::string& filename = "");
    };
};


#endif