#pragma once
#ifndef ENGINE_MATERIALS_MATERIAL_LOADER_H
#define ENGINE_MATERIALS_MATERIAL_LOADER_H

class Material;
class Texture;

#include <serenity/core/engine/materials/MaterialEnums.h>
#include <serenity/core/engine/renderer/GLImageConstants.h>
#include <serenity/core/engine/resources/Handle.h>
#include <serenity/core/engine/textures/TextureIncludes.h>


#ifdef ENGINE_FORCE_TEXTURE_MIPMAPPING
    #define ENGINE_MIPMAP_DEFAULT true
#else
    #define ENGINE_MIPMAP_DEFAULT false
#endif

namespace Engine::priv {
    class MaterialLoader final {
        private: 
            static std::pair<Engine::view_ptr<Texture>, Handle> internal_load_texture(const std::string& file, bool mipmapped, ImageInternalFormat, TextureType);
        public:
            static std::pair<Engine::view_ptr<Texture>, Handle> LoadTextureDiffuse(const std::string& file);
            static std::pair<Engine::view_ptr<Texture>, Handle> LoadTextureNormal(const std::string& file);
            static std::pair<Engine::view_ptr<Texture>, Handle> LoadTextureGlow(const std::string& file);
            static std::pair<Engine::view_ptr<Texture>, Handle> LoadTextureSpecular(const std::string& file);
            static std::pair<Engine::view_ptr<Texture>, Handle> LoadTextureAO(const std::string& file);
            static std::pair<Engine::view_ptr<Texture>, Handle> LoadTextureMetalness(const std::string& file);
            static std::pair<Engine::view_ptr<Texture>, Handle> LoadTextureSmoothness(const std::string& file);
            static std::pair<Engine::view_ptr<Texture>, Handle> LoadTextureMask(const std::string& file);
            static std::pair<Engine::view_ptr<Texture>, Handle> LoadTextureCubemap(const std::string& file);

            static void InitBase(Material&);
            static void Init(Material&, Handle diffuse, Handle normal, Handle glow, Handle specular, Handle ao, Handle metalness, Handle smoothness);
            
    };
    struct InternalMaterialPublicInterface final {
        static void LoadGPU(Material&);
        static void UnloadCPU(Material&);
        static void Load(Material&);
        static void Unload(Material&);
    };
};

#endif