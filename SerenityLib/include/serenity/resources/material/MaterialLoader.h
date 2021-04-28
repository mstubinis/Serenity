#pragma once
#ifndef ENGINE_MATERIALS_MATERIAL_LOADER_H
#define ENGINE_MATERIALS_MATERIAL_LOADER_H

class Material;
class Texture;
class TextureCubemap;

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
            static LoadedResource<Texture> internal_load_texture(std::string_view file, bool mipmapped, ImageInternalFormat, TextureType);
            static LoadedResource<TextureCubemap> internal_load_texture_cubemap(std::string_view file, bool mipmapped, ImageInternalFormat);
        public:
            static LoadedResource<Texture> LoadTextureDiffuse(std::string_view file);
            static LoadedResource<Texture> LoadTextureNormal(std::string_view file);
            static LoadedResource<Texture> LoadTextureGlow(std::string_view file);
            static LoadedResource<Texture> LoadTextureSpecular(std::string_view file);
            static LoadedResource<Texture> LoadTextureAO(std::string_view file);
            static LoadedResource<Texture> LoadTextureMetalness(std::string_view file);
            static LoadedResource<Texture> LoadTextureSmoothness(std::string_view file);
            static LoadedResource<Texture> LoadTextureMask(std::string_view file);
            static LoadedResource<TextureCubemap> LoadTextureCubemap(std::string_view file);

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