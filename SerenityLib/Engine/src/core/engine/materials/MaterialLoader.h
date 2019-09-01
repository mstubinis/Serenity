#pragma once
#ifndef ENGINE_MATERIALS_MATERIAL_LOADER_H
#define ENGINE_MATERIALS_MATERIAL_LOADER_H

#include <string>
#include <core/engine/materials/MaterialEnums.h>

class Material;
class Texture;
namespace Engine {
    namespace epriv {
        struct MaterialLoader final {
            static Texture* LoadTextureDiffuse(Material&, const std::string& file);
            static Texture* LoadTextureNormal(Material&, const std::string& file);
            static Texture* LoadTextureGlow(Material&, const std::string& file);
            static Texture* LoadTextureSpecular(Material&, const std::string& file);
            static Texture* LoadTextureAO(Material&, const std::string& file);
            static Texture* LoadTextureMetalness(Material&, const std::string& file);
            static Texture* LoadTextureSmoothness(Material&, const std::string& file);
            static Texture* LoadTextureMask(Material&, const std::string& file);
            static Texture* LoadTextureCubemap(Material&, const std::string& file);

            static void InternalInit(Material&, Texture* diffuse, Texture* normal, Texture* glow, Texture* specular);
            static void InternalInitBase(Material&);
        };
        struct InternalMaterialPublicInterface final {
            static void LoadCPU(Material&);
            static void LoadGPU(Material&);
            static void UnloadCPU(Material&);
            static void UnloadGPU(Material&);
        };
    };
};

#endif