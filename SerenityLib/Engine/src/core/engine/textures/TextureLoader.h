#pragma once
#ifndef ENGINE_TEXTURES_TEXTURE_LOADER_H
#define ENGINE_TEXTURES_TEXTURE_LOADER_H

#include <string>
#include <core/engine/textures/TextureIncludes.h>

namespace Engine::priv {
    struct TextureLoader final {
        friend class Texture;

        static bool LoadDDSFile(Texture& texture, const std::string& filename, priv::ImageLoadedStructure& image);

        static void LoadTexture2DIntoOpenGL(Texture& texture);
        static void LoadTextureFramebufferIntoOpenGL(Texture& texture);
        static void LoadTextureCubemapIntoOpenGL(Texture& texture);

        static void EnumWrapToGL(unsigned int& gl, const TextureWrap::Wrap wrap);
        static void EnumFilterToGL(unsigned int& gl, const TextureFilter::Filter filter, const bool min);
        static const bool IsCompressedType(const ImageInternalFormat::Format);

        static void GenerateMipmapsOpenGL(Texture& texture, const unsigned int addressIndex = 0);
        static void WithdrawPixelsFromOpenGLMemory(Texture& texture, const unsigned int imageIndex = 0, const unsigned int mipmapLevel = 0);
        static void ChoosePixelFormat(ImagePixelFormat::Format& outPxlFormat, const ImageInternalFormat::Format inInternalFormat);

        static void GeneratePBRData(Texture&, const unsigned int convoludeTextureSize, const unsigned int preEnvFilterSize);
        static void ImportIntoOpengl(Texture&, const Engine::priv::ImageMipmap& mipmap, const unsigned int openGLType);
        static void InitCommon(Texture&, const unsigned int openglTextureType, const bool toBeMipmapped);


        static void InitFramebuffer(Texture&, const unsigned int w, const unsigned int h, const ImagePixelType::Type pxlType, const ImagePixelFormat::Format pxlFormat, const ImageInternalFormat::Format _internal, const float divisor);
        static void InitFromMemory(Texture&, const sf::Image& sfImage, const std::string& name, const bool genMipMaps, const ImageInternalFormat::Format _internal, const unsigned int openglTextureType);
        static void InitFromFile(Texture&, const std::string& filename, const bool genMipMaps, const ImageInternalFormat::Format _internal, const unsigned int openglTextureType);
        static void InitFromFilesCubemap(Texture&, const std::string files[], const std::string& name, const bool genMipMaps, const ImageInternalFormat::Format _internal);
    };
    struct InternalTexturePublicInterface final {
        static void LoadCPU(Texture&);
        static void LoadGPU(Texture&);
        static void UnloadCPU(Texture&);
        static void UnloadGPU(Texture&);
        static void Load(Texture&);
        static void Unload(Texture&);

        static void Resize(Texture& texture, Engine::priv::FramebufferObject&, const unsigned int width, const unsigned int height);
    };
};

#endif