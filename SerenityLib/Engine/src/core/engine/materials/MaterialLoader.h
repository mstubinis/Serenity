#pragma once
#ifndef ENGINE_MATERIALS_MATERIAL_LOADER_H
#define ENGINE_MATERIALS_MATERIAL_LOADER_H

class Material;
class Texture;

#include <string>
#include <core/engine/materials/MaterialEnums.h>

namespace Engine::priv {
    struct MaterialLoader final {
        static Texture* LoadTextureDiffuse(const std::string& file);
        static Texture* LoadTextureNormal(const std::string& file);
        static Texture* LoadTextureGlow(const std::string& file);
        static Texture* LoadTextureSpecular(const std::string& file);
        static Texture* LoadTextureAO(const std::string& file);
        static Texture* LoadTextureMetalness(const std::string& file);
        static Texture* LoadTextureSmoothness(const std::string& file);
        static Texture* LoadTextureMask(const std::string& file);
        static Texture* LoadTextureCubemap(const std::string& file);

        static void InternalInit(Material&, Texture* diffuse, Texture* normal, Texture* glow, Texture* specular, Texture* ao, Texture* metalness, Texture* smoothness);
        static void InternalInitBase(Material&);
    };
    struct InternalMaterialPublicInterface final {
        static void LoadCPU(Material&);
        static void LoadGPU(Material&);
        static void UnloadCPU(Material&);
        static void UnloadGPU(Material&);
        static void Load(Material&);
        static void Unload(Material&);
    };
};

#endif