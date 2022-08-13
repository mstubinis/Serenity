#pragma once
#ifndef ENGINE_TEXTURES_TEXTURE_LOADER_H
#define ENGINE_TEXTURES_TEXTURE_LOADER_H

struct TextureRequest;
class  Texture;
namespace Engine::priv {
    class  FramebufferObject;
    struct TextureCPUData;
}
#include <serenity/resources/texture/TextureIncludes.h>

namespace Engine::priv {
    class TextureLoader final {
        friend class ::Texture;
        public:
            static void LoadTexture2DIntoOpenGL(Texture&);
            static void LoadTextureFramebufferIntoOpenGL(Texture&);
            static void LoadTextureCubemapIntoOpenGL(Texture&);

            static void Resize(Texture&, Engine::priv::FramebufferObject&, int width, int height);

            static void LoadCPU(TextureCPUData&, Handle);

            static void MirrorPixelsVertically(uint8_t* pixels, uint32_t width, uint32_t height, uint32_t colorsPerPixel = 4);

            static void LoadGPU(Handle, bool dispatchEventLoaded = true);
            static void LoadGPU(Texture&, bool dispatchEventLoaded = true);
            static void UnloadGPU(Texture&, bool dispatchEventUnloaded = true);
    };
};

#endif