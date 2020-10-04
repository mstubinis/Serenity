#pragma once
#ifndef ENGINE_MATERIALS_MATERIAL_LOADER_H
#define ENGINE_MATERIALS_MATERIAL_LOADER_H

class Material;
class Texture;

#include <core/engine/materials/MaterialEnums.h>
#include <core/engine/renderer/GLImageConstants.h>
#include <core/engine/resources/Handle.h>
#include <core/engine/textures/TextureIncludes.h>


#ifdef ENGINE_FORCE_TEXTURE_MIPMAPPING
    #define ENGINE_MIPMAP_DEFAULT true
#else
    #define ENGINE_MIPMAP_DEFAULT false
#endif

namespace Engine::priv {
    class MaterialLoader final {
        private: 
            static std::pair<Texture*, Handle> internal_load_texture(const std::string& file, bool mipmapped, ImageInternalFormat, TextureType);
        public:
            static std::pair<Texture*, Handle> LoadTextureDiffuse(const std::string& file);
            static std::pair<Texture*, Handle> LoadTextureNormal(const std::string& file);
            static std::pair<Texture*, Handle> LoadTextureGlow(const std::string& file);
            static std::pair<Texture*, Handle> LoadTextureSpecular(const std::string& file);
            static std::pair<Texture*, Handle> LoadTextureAO(const std::string& file);
            static std::pair<Texture*, Handle> LoadTextureMetalness(const std::string& file);
            static std::pair<Texture*, Handle> LoadTextureSmoothness(const std::string& file);
            static std::pair<Texture*, Handle> LoadTextureMask(const std::string& file);
            static std::pair<Texture*, Handle> LoadTextureCubemap(const std::string& file);

            static void InitBase(Material&);
            static void Init(Material&, Handle diffuse, Handle normal, Handle glow, Handle specular, Handle ao, Handle metalness, Handle smoothness);
            
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