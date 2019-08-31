#pragma once
#ifndef ENGINE_TEXTURES_TEXTURE_LOADER_H
#define ENGINE_TEXTURES_TEXTURE_LOADER_H

#include <string>
#include <core/engine/textures/TextureIncludes.h>

class Texture;

namespace Engine {
    namespace epriv {
        struct TextureLoader final {
            friend class Texture;

            static void LoadDDSFile(Texture& texture, const std::string& filename, epriv::ImageLoadedStructure& image);

            static void LoadTexture2DIntoOpenGL(Texture& texture);
            static void LoadTextureFramebufferIntoOpenGL(Texture& texture);
            static void LoadTextureCubemapIntoOpenGL(Texture& texture);

            static void EnumWrapToGL(uint& gl, const TextureWrap::Wrap& wrap);
            static void EnumFilterToGL(uint& gl, const TextureFilter::Filter& filter, const bool& min);
            static const bool IsCompressedType(const ImageInternalFormat::Format&);

            static void GenerateMipmapsOpenGL(Texture& texture);
            static void WithdrawPixelsFromOpenGLMemory(Texture& texture, const uint& imageIndex = 0, const uint & mipmapLevel = 0);
            static void ChoosePixelFormat(ImagePixelFormat::Format& outPxlFormat, const ImageInternalFormat::Format& inInternalFormat);

            static void GeneratePBRData(Texture&, const unsigned int& convoludeTextureSize, const unsigned int& preEnvFilterSize);
            static void ImportIntoOpengl(Texture&, const Engine::epriv::ImageMipmap& mipmap, const GLuint& openGLType);
        };
        struct InternalTexturePublicInterface final {
            static void LoadCPU(Texture&);
            static void LoadGPU(Texture&);
            static void UnloadCPU(Texture&);
            static void UnloadGPU(Texture&);
        };
    };
};

#endif