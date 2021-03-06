#pragma once
#ifndef ENGINE_TEXTURES_TEXTURE_LOADER_H
#define ENGINE_TEXTURES_TEXTURE_LOADER_H

struct TextureRequest;

#include <core/engine/textures/TextureIncludes.h>

namespace Engine::priv {
    struct TextureCPUData;

    struct TextureLoader final {
        friend class Texture;

        static bool LoadDDSFile(TextureCPUData& texture, priv::ImageData& image);

        static void LoadTexture2DIntoOpenGL(Texture& texture);
        static void LoadTextureFramebufferIntoOpenGL(Texture& texture);
        static void LoadTextureCubemapIntoOpenGL(Texture& texture);

        static void GenerateMipmapsOpenGL(Texture& texture);
        static void WithdrawPixelsFromOpenGLMemory(Texture& texture, unsigned int imageIndex = 0, unsigned int mipmapLevel = 0);

        static void GeneratePBRData(Texture&, int convoludeTextureSize, int preEnvFilterSize);
        static void ImportIntoOpengl(Texture&, const Engine::priv::ImageMipmap& mipmap, TextureType textureType);

        static void LoadGPU(Handle);
        static void LoadGPU(Texture&);

        static void UnloadGPU(Texture&);
        static void Load(Texture&);
        static void Unload(Texture&);

        static void LoadCPU(TextureCPUData&, Handle);

        static void Resize(Texture& texture, Engine::priv::FramebufferObject&, int width, int height);

    };
};

#endif