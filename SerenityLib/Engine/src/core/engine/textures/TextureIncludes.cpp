#include "core/engine/utils/PrecompiledHeader.h"
#include <core/engine/textures/TextureIncludes.h>
#include <SFML/Graphics/Image.hpp>

using namespace Engine::priv;
using namespace std;

ImageLoadedStructure::ImageLoadedStructure(const unsigned int width, const unsigned int height, const ImagePixelType::Type pixelType, const ImagePixelFormat::Format pixelFormat, const ImageInternalFormat::Format internalFormat){
    load(width, height, pixelType, pixelFormat, internalFormat);
}
ImageLoadedStructure::ImageLoadedStructure(const sf::Image& i, const string& filename){
    load(i, filename);
}
ImageLoadedStructure::~ImageLoadedStructure() {
}

ImageLoadedStructure::ImageLoadedStructure(ImageLoadedStructure&& other) noexcept {
    m_InternalFormat = std::move(other.m_InternalFormat);
    m_PixelFormat    = std::move(other.m_PixelFormat);
    m_PixelType      = std::move(other.m_PixelType);
    m_Mipmaps        = std::move(other.m_Mipmaps);
    m_Filename       = std::move(other.m_Filename);
}
ImageLoadedStructure& ImageLoadedStructure::operator=(ImageLoadedStructure&& other) noexcept {
    if (&other != this) {
        m_InternalFormat = std::move(other.m_InternalFormat);
        m_PixelFormat    = std::move(other.m_PixelFormat);
        m_PixelType      = std::move(other.m_PixelType);
        m_Mipmaps        = std::move(other.m_Mipmaps);
        m_Filename       = std::move(other.m_Filename);
    }
    return *this;
}
void ImageLoadedStructure::load(const unsigned int width, const unsigned int height, const ImagePixelType::Type pixelType, const ImagePixelFormat::Format pixelFormat, const ImageInternalFormat::Format internalFormat) {
    m_PixelFormat                 = pixelFormat;
    m_PixelType                   = pixelType;
    m_InternalFormat              = internalFormat;
    m_Mipmaps[0].width            = width;
    m_Mipmaps[0].height           = height;
    m_Mipmaps[0].compressedSize   = 0;
}
void ImageLoadedStructure::load(const sf::Image& sfImage, const string& filename) {
    m_Filename                    = filename;
    const auto imgSize            = sfImage.getSize();
    m_Mipmaps[0].width            = imgSize.x;
    m_Mipmaps[0].height           = imgSize.y;
    m_Mipmaps[0].compressedSize   = 0;
    auto* pxls = sfImage.getPixelsPtr();
    m_Mipmaps[0].pixels.assign(pxls, pxls + m_Mipmaps[0].width * m_Mipmaps[0].height * 4);
}