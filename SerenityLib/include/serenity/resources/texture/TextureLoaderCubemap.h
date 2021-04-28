#pragma once
#ifndef ENGINE_TEXTURES_TEXTURE_LOADER_CUBEMAP_H
#define ENGINE_TEXTURES_TEXTURE_LOADER_CUBEMAP_H

struct TextureRequest;
class  Texture;
class  TextureCubemap;
namespace Engine::priv {
    struct TextureCPUData;
    struct TextureCubemapCPUData;
}
#include <serenity/resources/texture/TextureIncludes.h>

namespace Engine::priv {
    class TextureLoaderCubemap final {
        friend class  TextureCubemap;
        friend struct Engine::priv::TextureCubemapCPUData;
        public:
            static bool LoadDDSFile(TextureCubemapCPUData&, priv::ImageData& image);

            static void LoadTexture2DIntoOpenGL(TextureCubemap&);
            static void LoadTextureFramebufferIntoOpenGL(TextureCubemap&);
            static void LoadTextureCubemapIntoOpenGL(TextureCubemap&);

            static bool GenerateMipmapsOpenGL(TextureCubemap&);
            static void WithdrawPixelsFromOpenGLMemory(TextureCubemap&, uint32_t imageIndex = 0, uint32_t mipmapLevel = 0);

            static void GeneratePBRData(TextureCubemap&, int convoludeTextureSize, int preEnvFilterSize);
            static void ImportIntoOpengl(TextureCubemap&, const Engine::priv::ImageMipmap& mipmap, TextureType textureType);

            static void LoadCPU(TextureCubemapCPUData&, Handle);

            static void LoadGPU(Handle);
            static void LoadGPU(TextureCubemap&);
            static void UnloadGPU(TextureCubemap&);

            static void Load(TextureCubemap&);
            static void Unload(TextureCubemap&);
    };
};

#endif