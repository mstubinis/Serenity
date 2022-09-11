
#include <serenity/resources/texture/TextureIncludes.h>
#include <SFML/Graphics/Image.hpp>

using namespace Engine::priv;

namespace {
    void do_flags(sf::Image& sfImage, sfImageLoaderFlags flags) {
        if (!(flags & sfImageLoaderFlags::FlipVertically)) { //by default, sfml will flip images vertically. kind of annoying
            sfImage.flipVertically();
        }
        if (flags & sfImageLoaderFlags::FlipHorizontally) {
            sfImage.flipHorizontally();
        }
    }
    Engine::priv::ImageMipmap* get_next_mipmap(std::vector<Engine::priv::ImageMipmap>& mipmaps) {
        for (auto& mipmap : mipmaps) {
            if (mipmap.isNull()) {
                return &mipmap;
            }
        }
        return nullptr;
    }
}

void ImageData::load(int width, int height, ImagePixelType pixelType, ImagePixelFormat pixelFormat, ImageInternalFormat internalFormat) {
    m_PixelFormat     = pixelFormat;
    m_PixelType       = pixelType;
    m_InternalFormat  = internalFormat;
    auto mipmap       = get_next_mipmap(m_Mipmaps);
    if (mipmap) {
        mipmap->compressedSize = 0;
        mipmap->width          = width;
        mipmap->height         = height;
    }
}
void ImageData::load(const uint8_t* inPixels, int inWidth, int inHeight, const std::string& filename) {
    m_Filename  = filename;
    auto mipmap = get_next_mipmap(m_Mipmaps);
    if (mipmap) {
        mipmap->compressedSize = 0;
        mipmap->assignPixels(inPixels, inWidth, inHeight);
    }
}
void ImageData::setFormats(ImageInternalFormat intFmt) {
    m_InternalFormat = intFmt;
    m_PixelFormat    = m_InternalFormat; //does a nice conversion here
}



sfImageLoader::sfImageLoader(const char* filename, sfImageLoaderFlags flags) {
    loadFromFile(filename, flags);
}

void sfImageLoader::loadFromFile(const char* filename, sfImageLoaderFlags flags) {
    m_SFImage.loadFromFile(filename);
    do_flags(m_SFImage, flags);
}
void sfImageLoader::loadFromMemory(const void* data, size_t size, sfImageLoaderFlags flags) {
    m_SFImage.loadFromMemory(data, size);
    do_flags(m_SFImage, flags);
}