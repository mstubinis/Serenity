#include <core/engine/textures/TextureIncludes.h>

#include <SFML/Graphics/Image.hpp>

using namespace Engine;
using namespace std;


epriv::ImageMipmap::ImageMipmap() {
    level = width = height = compressedSize = 0;
}
epriv::ImageMipmap::~ImageMipmap() {
    vector_clear(pixels);
}


epriv::ImageLoadedStructure::ImageLoadedStructure() {
    ImageMipmap baseImage;
    filename = "";
    baseImage.compressedSize = 0;
    mipmaps.push_back(baseImage);
}
epriv::ImageLoadedStructure::~ImageLoadedStructure() {
    vector_clear(mipmaps);
}
epriv::ImageLoadedStructure::ImageLoadedStructure(const uint _w, const uint _h, const ImagePixelType::Type _pxlType, const ImagePixelFormat::Format _pxlFormat, const ImageInternalFormat::Format _internFormat) {
    load(_w, _h, _pxlType, _pxlFormat, _internFormat);
}
epriv::ImageLoadedStructure::ImageLoadedStructure(const sf::Image& i, const string& _filename) {
    load(i, _filename);
}
void epriv::ImageLoadedStructure::load(const uint _width, const uint _height, const ImagePixelType::Type _pixelType, const ImagePixelFormat::Format _pixelFormat, const ImageInternalFormat::Format _internalFormat) {
    ImageMipmap* baseImage = nullptr;
    if (mipmaps.size() > 0) {
        baseImage = &(mipmaps[0]);
    }else{
        baseImage = NEW ImageMipmap();
    }
    filename                  = "";
    pixelFormat               = _pixelFormat;
    pixelType                 = _pixelType;
    internalFormat            = _internalFormat;
    baseImage->width          = _width;
    baseImage->height         = _height;
    baseImage->compressedSize = 0;
    if (mipmaps.size() == 0) {
        mipmaps.push_back(ImageMipmap(*baseImage));
        SAFE_DELETE(baseImage);
    }
}
void epriv::ImageLoadedStructure::load(const sf::Image& i, const string& _filename) {
    ImageMipmap* baseImage = nullptr;
    if (mipmaps.size() > 0) {
        baseImage = &(mipmaps[0]);
    }else{
        baseImage = NEW ImageMipmap();
    }
    filename = _filename;
    const auto imgSize = i.getSize();
    baseImage->width = imgSize.x;
    baseImage->height = imgSize.y;
    baseImage->compressedSize = 0;
    auto pxls = i.getPixelsPtr();
    baseImage->pixels.assign(pxls, pxls + baseImage->width * baseImage->height * 4);
    if (mipmaps.size() == 0) {
        mipmaps.push_back(ImageMipmap(*baseImage));
        SAFE_DELETE(baseImage);
    }
}