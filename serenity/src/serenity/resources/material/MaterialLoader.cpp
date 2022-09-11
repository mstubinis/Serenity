
#include <serenity/resources/material/MaterialLoader.h>
#include <serenity/resources/material/MaterialComponent.h>
#include <serenity/resources/texture/Texture.h>
#include <serenity/resources/texture/TextureCubemap.h>
#include <serenity/resources/material/Material.h>
#include <serenity/resources/Engine_Resources.h>

namespace {
    LoadedResource<Texture> internal_load_texture(std::string_view file, bool mipmapped, ImageInternalFormat format, TextureType textureType) {
        LoadedResource<Texture> texture;
        if (!file.empty()) {
            texture = Engine::Resources::getResource<Texture>(file);
            if (!texture.m_Resource) {

                TextureConstructorInfo ci;
                ci.filename       = file;
                ci.internalFormat = format;
                ci.mipmapped      = mipmapped;
                ci.type           = textureType;

                texture = Engine::Resources::addResource<Texture>(ci);
            }
        }
        return texture;
    }
    LoadedResource<TextureCubemap> internal_load_texture_cubemap(std::string_view file, bool mipmapped, ImageInternalFormat format) {
        LoadedResource<TextureCubemap> cubemap;
        if (!file.empty()) {
            cubemap = Engine::Resources::getResource<TextureCubemap>(file);
            if (!cubemap.m_Resource) {
                cubemap = Engine::Resources::addResource<TextureCubemap>(file, mipmapped, format);
            }
        }
        return cubemap;
    }
}

void Engine::priv::MaterialLoader::Init(Material& material, Handle diffuse, Handle normal, Handle glow, Handle specular, Handle ao, Handle metal, Handle smooth) {
    InitBase(material);
    if (diffuse)    material.internal_add_component_generic(MaterialComponentType::Diffuse,    diffuse);
    if (normal)     material.internal_add_component_generic(MaterialComponentType::Normal,     normal);
    if (glow)       material.internal_add_component_generic(MaterialComponentType::Glow,       glow);
    if (specular)   material.internal_add_component_generic(MaterialComponentType::Specular,   specular);
    if (metal)      material.internal_add_component_generic(MaterialComponentType::Metalness,  metal);
    if (smooth)     material.internal_add_component_generic(MaterialComponentType::Smoothness, smooth);
    if (ao)         material.internal_add_component_generic(MaterialComponentType::AO,         ao);
}
void Engine::priv::MaterialLoader::InitBase(Material& material) {
    material.m_Components.resize(MAX_MATERIAL_COMPONENTS);
    material.internal_update_global_material_pool(true);
}

LoadedResource<Texture> Engine::priv::MaterialLoader::LoadTextureDiffuse(std::string_view file) {
    return internal_load_texture(file, ENGINE_MIPMAP_DEFAULT, ImageInternalFormat::SRGB8_ALPHA8, TextureType::Texture2D);
}
LoadedResource<Texture> Engine::priv::MaterialLoader::LoadTextureNormal(std::string_view file) {
    return internal_load_texture(file, ENGINE_MIPMAP_DEFAULT, ImageInternalFormat::RGB8, TextureType::Texture2D);
}
LoadedResource<Texture> Engine::priv::MaterialLoader::LoadTextureGlow(std::string_view file) {
    return internal_load_texture(file, ENGINE_MIPMAP_DEFAULT, ImageInternalFormat::R8, TextureType::Texture2D);
}
LoadedResource<Texture> Engine::priv::MaterialLoader::LoadTextureSpecular(std::string_view file) {
    return internal_load_texture(file, ENGINE_MIPMAP_DEFAULT, ImageInternalFormat::R8, TextureType::Texture2D);
}
LoadedResource<Texture> Engine::priv::MaterialLoader::LoadTextureAO(std::string_view file) {
    return internal_load_texture(file, ENGINE_MIPMAP_DEFAULT, ImageInternalFormat::R8, TextureType::Texture2D);
}
LoadedResource<Texture> Engine::priv::MaterialLoader::LoadTextureMetalness(std::string_view file) {
    return internal_load_texture(file, ENGINE_MIPMAP_DEFAULT, ImageInternalFormat::R8, TextureType::Texture2D);
}
LoadedResource<Texture> Engine::priv::MaterialLoader::LoadTextureSmoothness(std::string_view file) {
    return internal_load_texture(file, ENGINE_MIPMAP_DEFAULT, ImageInternalFormat::R8, TextureType::Texture2D);
}
LoadedResource<Texture> Engine::priv::MaterialLoader::LoadTextureMask(std::string_view file) {
    return internal_load_texture(file, ENGINE_MIPMAP_DEFAULT, ImageInternalFormat::R8, TextureType::Texture2D);
}
LoadedResource<TextureCubemap> Engine::priv::MaterialLoader::LoadTextureCubemap(std::string_view file) {
    return internal_load_texture_cubemap(file, ENGINE_MIPMAP_DEFAULT, ImageInternalFormat::SRGB8_ALPHA8);
}
void Engine::priv::PublicMaterial::LoadGPU(Material& material) {
    material.Resource::load();
}
void Engine::priv::PublicMaterial::UnloadCPU(Material& material) {
    material.Resource::unload();
}
void Engine::priv::PublicMaterial::Load(Material& material) {
    if (!material.isLoaded()) {
        Engine::priv::PublicMaterial::LoadGPU(material);
    }
}
void Engine::priv::PublicMaterial::Unload(Material& material) {
    if (material.isLoaded()) {
        Engine::priv::PublicMaterial::UnloadCPU(material);
    }
}