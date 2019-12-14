#pragma once
#ifndef ENGINE_TEXTURE_INCLUDES_INCLUDE_GUARD
#define ENGINE_TEXTURE_INCLUDES_INCLUDE_GUARD

#include <memory>
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
namespace Engine {
    namespace epriv {
        class  FramebufferObject;
        struct ImageMipmap final {
            uint32_t width;
            uint32_t height;
            uint32_t compressedSize;
            uint32_t level;
            std::vector<uint8_t> pixels;
            ImageMipmap();
            ~ImageMipmap();
        };
        struct ImageLoadedStructure final {
            ImageInternalFormat::Format           internalFormat;
            ImagePixelFormat::Format              pixelFormat;
            ImagePixelType::Type                  pixelType;
            std::vector<epriv::ImageMipmap>       mipmaps;
            std::string                           filename;

            ImageLoadedStructure();
            ImageLoadedStructure(const uint width, const uint height, const ImagePixelType::Type _pxlType, const ImagePixelFormat::Format _pxlFormat, const ImageInternalFormat::Format _internFormat);
            ImageLoadedStructure(const sf::Image& sfImage, const std::string& filename = "");

            ImageLoadedStructure(const ImageLoadedStructure&)                      = delete;
            ImageLoadedStructure& operator=(const ImageLoadedStructure&)           = delete;
            ImageLoadedStructure(ImageLoadedStructure&& other) noexcept            = default;
            ImageLoadedStructure& operator=(ImageLoadedStructure&& other) noexcept = default;


            ~ImageLoadedStructure();

            void load(const uint width, const uint height, const ImagePixelType::Type _pixelType, const ImagePixelFormat::Format _pixelFormat, const ImageInternalFormat::Format _internalFormat);
            void load(const sf::Image& sfImage, const std::string& filename = "");
        };
    };
};


#endif