#pragma once
#ifndef ENGINE_TEXTURES_TEXTURE_LOADER_CUBEMAP_H
#define ENGINE_TEXTURES_TEXTURE_LOADER_CUBEMAP_H

class  Texture;
class  TextureCubemap;
namespace Engine::priv {
    struct TextureCPUData;
    struct TextureCubemapCPUData;
}
#include <serenity/resources/texture/TextureIncludes.h>

namespace Engine::priv {
    class TextureLoaderCubemap final {
        friend class  ::TextureCubemap;
        friend struct Engine::priv::TextureCubemapCPUData;
        public:
            static void GeneratePBRData(TextureCubemap&, int convoludeTextureSize, int preEnvFilterSize);

            static void LoadCPU(TextureCubemapCPUData&, Handle);

            static void LoadGPU(Handle);
            static void LoadGPU(TextureCubemap&);
            static void UnloadGPU(TextureCubemap&);
    };
};

#endif