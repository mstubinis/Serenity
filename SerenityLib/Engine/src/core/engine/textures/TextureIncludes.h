#pragma once
#ifndef ENGINE_TEXTURE_INCLUDES_INCLUDE_GUARD
#define ENGINE_TEXTURE_INCLUDES_INCLUDE_GUARD

#include <core/engine/renderer/GLImageConstants.h>
#include <core/engine/utils/Utils.h>

struct TextureWrap final {enum Wrap {
    Repeat, 
    RepeatMirrored, 
    ClampToEdge, 
    ClampToBorder,
    _TOTAL,
};};
struct TextureFilter final {enum Filter {
    Linear,
    Nearest,
    Nearest_Mipmap_Nearest,
    Nearest_Mipmap_Linear,
    Linear_Mipmap_Nearest,
    Linear_Mipmap_Linear,
    _TOTAL,
};};
struct TextureType final {enum Type {
    Texture1D,
    Texture2D,
    Texture3D,
    CubeMap,
    RenderTarget,
    _TOTAL,
};};

class Texture;
namespace sf { 
    class Image; 
}
namespace Engine::priv {
    class  FramebufferObject;
};

namespace Engine::priv {
    struct ImageMipmap final {
        std::uint32_t               width          = 0;
        std::uint32_t               height         = 0;
        std::uint32_t               compressedSize = 0;
        std::uint32_t               level          = 0;
        std::vector<std::uint8_t>   pixels;
    };
    struct ImageLoadedStructure final {
        ImageInternalFormat::Format           m_InternalFormat;
        ImagePixelFormat::Format              m_PixelFormat;
        ImagePixelType::Type                  m_PixelType;
        std::vector<priv::ImageMipmap>        m_Mipmaps         = { ImageMipmap() };
        std::string                           m_Filename        = "";

        ImageLoadedStructure() = default;
        ImageLoadedStructure(const unsigned int width, const unsigned int height, const ImagePixelType::Type pixelType, const ImagePixelFormat::Format pixelFormat, const ImageInternalFormat::Format internalFormat);
        ImageLoadedStructure(const sf::Image& sfImage, const std::string& filename = "");
        ~ImageLoadedStructure();

        ImageLoadedStructure(const ImageLoadedStructure&)                      = delete;
        ImageLoadedStructure& operator=(const ImageLoadedStructure&)           = delete;
        ImageLoadedStructure(ImageLoadedStructure&& other) noexcept;
        ImageLoadedStructure& operator=(ImageLoadedStructure&& other) noexcept;

        void load(const unsigned int width, const unsigned int height, const ImagePixelType::Type pixelType, const ImagePixelFormat::Format pixelFormat, const ImageInternalFormat::Format internalFormat);
        void load(const sf::Image& sfImage, const std::string& filename = "");
    };
};


#endif