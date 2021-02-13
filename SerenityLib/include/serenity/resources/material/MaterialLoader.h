#pragma once
#ifndef ENGINE_MATERIALS_MATERIAL_LOADER_H
#define ENGINE_MATERIALS_MATERIAL_LOADER_H

class Material;
class Texture;

#include <serenity/resources/material/MaterialEnums.h>
#include <serenity/renderer/GLImageConstants.h>
#include <serenity/resources/Handle.h>
#include <serenity/resources/texture/TextureIncludes.h>


#ifdef ENGINE_FORCE_TEXTURE_MIPMAPPING
    #define ENGINE_MIPMAP_DEFAULT true
#else
    #define ENGINE_MIPMAP_DEFAULT false
#endif

namespace Engine::priv {
    class MaterialLoader final {
        private: 
            static LoadedResource<Texture> internal_load_texture(const std::string& file, bool mipmapped, ImageInternalFormat, TextureType);
        public:
            static LoadedResource<Texture> LoadTextureDiffuse(const std::string& file);
            static LoadedResource<Texture> LoadTextureNormal(const std::string& file);
            static LoadedResource<Texture> LoadTextureGlow(const std::string& file);
            static LoadedResource<Texture> LoadTextureSpecular(const std::string& file);
            static LoadedResource<Texture> LoadTextureAO(const std::string& file);
            static LoadedResource<Texture> LoadTextureMetalness(const std::string& file);
            static LoadedResource<Texture> LoadTextureSmoothness(const std::string& file);
            static LoadedResource<Texture> LoadTextureMask(const std::string& file);
            static LoadedResource<Texture> LoadTextureCubemap(const std::string& file);

            static void InitBase(Material&);
            static void Init(Material&, Handle diffuse, Handle normal, Handle glow, Handle specular, Handle ao, Handle metalness, Handle smoothness);
            
    };
    struct PublicMaterial final {
        static void LoadGPU(Material&);
        static void UnloadCPU(Material&);
        static void Load(Material&);
        static void Unload(Material&);
    };
};

#endif