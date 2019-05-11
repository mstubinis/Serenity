#include <core/engine/textures/TextureIncludes.h>
#include <core/engine/Engine_Utils.h>

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
epriv::ImageLoadedStructure::ImageLoadedStructure(uint _w, uint _h, ImagePixelType::Type _pxlType, ImagePixelFormat::Format _pxlFormat, ImageInternalFormat::Format _internFormat) {
    load(_w, _h, _pxlType, _pxlFormat, _internFormat);
}
epriv::ImageLoadedStructure::ImageLoadedStructure(const sf::Image& i, string _filename) {
    load(i, _filename);
}
void epriv::ImageLoadedStructure::load(uint _width, uint _height, ImagePixelType::Type _pixelType, ImagePixelFormat::Format _pixelFormat, ImageInternalFormat::Format _internalFormat) {
    ImageMipmap* baseImage = nullptr;
    if (mipmaps.size() > 0) {
        baseImage = &(mipmaps[0]);
    }else{
        baseImage = new ImageMipmap();
    }
    filename = "";
    pixelFormat = _pixelFormat;
    pixelType = _pixelType;
    internalFormat = _internalFormat;
    baseImage->width = _width;
    baseImage->height = _height;
    baseImage->compressedSize = 0;
    if (mipmaps.size() == 0) {
        mipmaps.push_back(ImageMipmap(*baseImage));
        delete baseImage;
    }
}
void epriv::ImageLoadedStructure::load(const sf::Image& i, string _filename) {
    ImageMipmap* baseImage = nullptr;
    if (mipmaps.size() > 0) {
        baseImage = &(mipmaps[0]);
    }else{
        baseImage = new ImageMipmap();
    }
    filename = _filename;
    baseImage->width = i.getSize().x;
    baseImage->height = i.getSize().y;
    baseImage->compressedSize = 0;
    baseImage->pixels.assign(i.getPixelsPtr(), i.getPixelsPtr() + baseImage->width * baseImage->height * 4);
    if (mipmaps.size() == 0) {
        mipmaps.push_back(ImageMipmap(*baseImage));
        delete baseImage;
    }
}