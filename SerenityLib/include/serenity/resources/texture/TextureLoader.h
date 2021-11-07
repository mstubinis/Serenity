#pragma once
#ifndef ENGINE_TEXTURES_TEXTURE_LOADER_H
#define ENGINE_TEXTURES_TEXTURE_LOADER_H

struct TextureRequest;
class  Texture;
namespace Engine::priv {
    struct TextureCPUData;
}
#include <serenity/resources/texture/TextureIncludes.h>

namespace Engine::priv {
    class TextureLoader final {
        friend class ::Texture;
        public:
            static bool LoadDDSFile(TextureCPUData&, priv::ImageData&);

            static void LoadTexture2DIntoOpenGL(Texture&);
            static void LoadTextureFramebufferIntoOpenGL(Texture&);
            static void LoadTextureCubemapIntoOpenGL(Texture&);

            static bool GenerateMipmapsOpenGL(Texture&);
            static void WithdrawPixelsFromOpenGLMemory(Texture&, uint32_t imageIndex = 0, uint32_t mipmapLevel = 0);

            static void ImportIntoOpengl(Texture&, const Engine::priv::ImageMipmap&, TextureType);
            static void Resize(Texture&, Engine::priv::FramebufferObject&, int width, int height);

            static void LoadCPU(TextureCPUData&, Handle);

            static void LoadGPU(Handle);
            static void LoadGPU(Texture&);
            static void UnloadGPU(Texture&);

            static void Load(Texture&);
            static void Unload(Texture&);
    };
};

#endif