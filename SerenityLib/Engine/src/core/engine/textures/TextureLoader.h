#pragma once
#ifndef ENGINE_TEXTURES_TEXTURE_LOADER_H
#define ENGINE_TEXTURES_TEXTURE_LOADER_H

#include <core/engine/textures/TextureIncludes.h>

namespace Engine::priv {
    struct TextureLoader final {
        friend class Texture;

        static bool LoadDDSFile(Texture& texture, const std::string& filename, priv::ImageLoadedStructure& image);

        static void LoadTexture2DIntoOpenGL(Texture& texture);
        static void LoadTextureFramebufferIntoOpenGL(Texture& texture);
        static void LoadTextureCubemapIntoOpenGL(Texture& texture);

        static void EnumWrapToGL(unsigned int& gl, TextureWrap wrap);
        static void EnumFilterToGL(unsigned int& gl, TextureFilter filter, bool min);
        static bool IsCompressedType(ImageInternalFormat);

        static void GenerateMipmapsOpenGL(Texture& texture, unsigned int addressIndex = 0);
        static void WithdrawPixelsFromOpenGLMemory(Texture& texture, unsigned int imageIndex = 0, unsigned int mipmapLevel = 0);
        static void ChoosePixelFormat(ImagePixelFormat& outPxlFormat, ImageInternalFormat inInternalFormat);

        static void GeneratePBRData(Texture&, unsigned int convoludeTextureSize, unsigned int preEnvFilterSize);
        static void ImportIntoOpengl(Texture&, const Engine::priv::ImageMipmap& mipmap, unsigned int openGLType);
        static void InitCommon(Texture&, unsigned int openglTextureType, bool toBeMipmapped);


        static void InitFramebuffer(Texture&, unsigned int w, unsigned int h, ImagePixelType pxlType, ImagePixelFormat pxlFormat, ImageInternalFormat _internal, float divisor);
        static void InitFromMemory(Texture&, const sf::Image& sfImage, const std::string& name, bool genMipMaps, ImageInternalFormat _internal, unsigned int openglTextureType);
        static void InitFromFile(Texture&, const std::string& filename, bool genMipMaps, ImageInternalFormat _internal, unsigned int openglTextureType);
        static void InitFromFilesCubemap(Texture&, const std::string files[], const std::string& name, bool genMipMaps, ImageInternalFormat _internal);
    };
    struct InternalTexturePublicInterface final {
        static void LoadCPU(Texture&);
        static void LoadGPU(Texture&);
        static void UnloadCPU(Texture&);
        static void UnloadGPU(Texture&);
        static void Load(Texture&);
        static void Unload(Texture&);

        static void Resize(Texture& texture, Engine::priv::FramebufferObject&, unsigned int width, unsigned int height);
    };
};

#endif