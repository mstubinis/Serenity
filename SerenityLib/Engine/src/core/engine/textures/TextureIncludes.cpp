#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/textures/TextureIncludes.h>
#include <SFML/Graphics/Image.hpp>

using namespace Engine::priv;


ImageMipmap::ImageMipmap(ImageMipmap&& other) noexcept 
    : pixels         { std::move(other.pixels) }
    , width          { std::move(other.width) }
    , height         { std::move(other.height) }
    , compressedSize { std::move(other.compressedSize) }
    , level          { std::move(other.level) }
{}
ImageMipmap& ImageMipmap::operator=(ImageMipmap&& other) noexcept {
    pixels         = std::move(other.pixels);
    width          = std::move(other.width);
    height         = std::move(other.height);
    compressedSize = std::move(other.compressedSize);
    level          = std::move(other.level);
    return *this;
}




ImageLoadedStructure::ImageLoadedStructure(int width, int height, ImagePixelType pxlType, ImagePixelFormat pxlFmt, ImageInternalFormat internalFmt)
    : ImageLoadedStructure{}
{
    load(width, height, pxlType, pxlFmt, internalFmt);
}
ImageLoadedStructure::ImageLoadedStructure(const sf::Image& sfImage, const std::string& filename) 
    : ImageLoadedStructure{}
{
    load(sfImage, filename);
}
ImageLoadedStructure::ImageLoadedStructure(ImageLoadedStructure&& other) noexcept 
    : m_Mipmaps        { std::move(other.m_Mipmaps) }
    , m_Filename       { std::move(other.m_Filename) }
    , m_InternalFormat { std::move(other.m_InternalFormat) }
    , m_PixelFormat    { std::move(other.m_PixelFormat) }
    , m_PixelType      { std::move(other.m_PixelType) }
{}
ImageLoadedStructure& ImageLoadedStructure::operator=(ImageLoadedStructure&& other) noexcept {
    m_Mipmaps        = std::move(other.m_Mipmaps);
    m_Filename       = std::move(other.m_Filename);
    m_InternalFormat = std::move(other.m_InternalFormat);
    m_PixelFormat    = std::move(other.m_PixelFormat);
    m_PixelType      = std::move(other.m_PixelType);
    return *this;
}


void ImageLoadedStructure::load(int width, int height, ImagePixelType pixelType, ImagePixelFormat pixelFormat, ImageInternalFormat internalFormat) {
    m_PixelFormat                 = pixelFormat;
    m_PixelType                   = pixelType;
    m_InternalFormat              = internalFormat;
    m_Mipmaps[0].width            = width;
    m_Mipmaps[0].height           = height;
    m_Mipmaps[0].compressedSize   = 0;
}
void ImageLoadedStructure::load(const sf::Image& sfImage, const std::string& filename) {
    m_Filename                    = filename;
    const auto imgSize            = sfImage.getSize();
    m_Mipmaps[0].width            = imgSize.x;
    m_Mipmaps[0].height           = imgSize.y;
    m_Mipmaps[0].compressedSize   = 0;
    auto* pxls = sfImage.getPixelsPtr();
    m_Mipmaps[0].pixels.assign(pxls, pxls + m_Mipmaps[0].width * m_Mipmaps[0].height * 4);
}