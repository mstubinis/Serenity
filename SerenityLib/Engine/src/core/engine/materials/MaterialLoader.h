#pragma once
#ifndef ENGINE_MATERIALS_MATERIAL_LOADER_H
#define ENGINE_MATERIALS_MATERIAL_LOADER_H

class Material;
class Texture;

#include <core/engine/materials/MaterialEnums.h>
#include <core/engine/renderer/GLImageConstants.h>


#ifdef ENGINE_FORCE_TEXTURE_MIPMAPPING
    #define ENGINE_MIPMAP_DEFAULT true
#else
    #define ENGINE_MIPMAP_DEFAULT false
#endif

namespace Engine::priv {
    class MaterialLoader final {
        private: 
            static Texture* internal_load_texture(const std::string& file, bool mipmapped, ImageInternalFormat format, unsigned int gl_type);
        public:
            static Texture* LoadTextureDiffuse(const std::string& file);
            static Texture* LoadTextureNormal(const std::string& file);
            static Texture* LoadTextureGlow(const std::string& file);
            static Texture* LoadTextureSpecular(const std::string& file);
            static Texture* LoadTextureAO(const std::string& file);
            static Texture* LoadTextureMetalness(const std::string& file);
            static Texture* LoadTextureSmoothness(const std::string& file);
            static Texture* LoadTextureMask(const std::string& file);
            static Texture* LoadTextureCubemap(const std::string& file);

            static void InitBase(Material&);
            static void Init(Material&, Texture* diffuse, Texture* normal, Texture* glow, Texture* specular, Texture* ao, Texture* metalness, Texture* smoothness);
            
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