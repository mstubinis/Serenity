#pragma once
#ifndef ENGINE_TEXTURES_TEXTURE_LOADER_H
#define ENGINE_TEXTURES_TEXTURE_LOADER_H

#include <core/engine/textures/TextureIncludes.h>

namespace Engine::priv {
    struct TextureCPUData;

    struct TextureLoader final {
        friend class Texture;

        static bool LoadDDSFile(TextureCPUData& texture, const std::string& filename, priv::ImageLoadedStructure& image);

        static void LoadTexture2DIntoOpenGL(Texture& texture);
        static void LoadTextureFramebufferIntoOpenGL(Texture& texture);
        static void LoadTextureCubemapIntoOpenGL(Texture& texture);

        static void GenerateMipmapsOpenGL(Texture& texture);
        static void WithdrawPixelsFromOpenGLMemory(Texture& texture, unsigned int imageIndex = 0, unsigned int mipmapLevel = 0);

        static void GeneratePBRData(Texture&, int convoludeTextureSize, int preEnvFilterSize);
        static void ImportIntoOpengl(Texture&, const Engine::priv::ImageMipmap& mipmap, TextureType textureType);

        static void CPUInitCommon(TextureCPUData&, TextureType textureType, bool toBeMipmapped);
        static void CPUInitFramebuffer(TextureCPUData&, int w, int h, ImagePixelType pxlType, ImagePixelFormat pxlFormat, ImageInternalFormat intFormat, float divisor);
        static void CPUInitFromMemory(TextureCPUData&, const sf::Image& sfImage, const std::string& name, bool genMipMaps, ImageInternalFormat intFormat, TextureType textureType);
        static void CPUInitFromMemory(Handle textureHandle, const sf::Image& sfImage, const std::string& name, bool genMipMaps, ImageInternalFormat intFormat, TextureType textureType);
        static void CPUInitFromFile(TextureCPUData&, const std::string& filename, bool genMipMaps, ImageInternalFormat intFormat, TextureType textureType);

        static void CPUInitFromFile(Handle textureHandle, const std::string& filename, bool genMipMaps, ImageInternalFormat intFormat, TextureType textureType);

        static void CPUInitFromFilesCubemap(TextureCPUData&, const std::array<std::string, 6>& files, const std::string& name, bool genMipMaps, ImageInternalFormat intFormat);
    };
    struct InternalTexturePublicInterface final {
        static void LoadCPU(Handle);
        static void LoadGPU(Handle);
        static void UnloadCPU(Handle);
        static void UnloadGPU(Handle);
        static void Load(Handle);
        static void Unload(Handle);

        static void LoadCPU(Texture&);
        static void LoadGPU(Texture&);
        static void UnloadCPU(Texture&);
        static void UnloadGPU(Texture&);
        static void Load(Texture&);
        static void Unload(Texture&);

        static void Resize(Texture& texture, Engine::priv::FramebufferObject&, int width, int height);
    };
};

#endif