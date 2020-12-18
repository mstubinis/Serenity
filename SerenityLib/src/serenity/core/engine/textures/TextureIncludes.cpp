
#include <serenity/core/engine/textures/TextureIncludes.h>
#include <SFML/Graphics/Image.hpp>

using namespace Engine::priv;

void ImageData::load(int width, int height, ImagePixelType pixelType, ImagePixelFormat pixelFormat, ImageInternalFormat internalFormat) {
    m_PixelFormat              = pixelFormat;
    m_PixelType                = pixelType;
    m_InternalFormat           = internalFormat;
    for (auto& mip : m_Mipmaps) {
        if (mip.null()) {
            mip.compressedSize = 0;
            mip.width          = width;
            mip.height         = height;
            break;
        }
    }
}
void ImageData::load(const sf::Image& sfImage, const std::string& filename) {
    m_Filename                 = filename;
    const auto imgSize         = sfImage.getSize();
    for (auto& mip : m_Mipmaps) {
        if (mip.null()) {
            mip.compressedSize = 0;
            mip.width          = imgSize.x;
            mip.height         = imgSize.y;
            auto* pxls         = sfImage.getPixelsPtr();
            mip.pixels.assign(pxls, pxls + mip.width * mip.height * 4);
            break;
        }
    }
}
void ImageData::setInternalFormat(ImageInternalFormat intFmt) {
    m_InternalFormat = intFmt;
    m_PixelFormat    = m_InternalFormat;
}