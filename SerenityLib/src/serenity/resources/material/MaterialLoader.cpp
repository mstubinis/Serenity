
#include <serenity/resources/material/MaterialLoader.h>
#include <serenity/resources/material/MaterialComponent.h>
#include <serenity/resources/texture/Texture.h>
#include <serenity/resources/material/Material.h>
#include <serenity/system/Engine.h>
#include <serenity/events/Event.h>

void Engine::priv::MaterialLoader::Init(Material& material, Handle diffuse, Handle normal, Handle glow, Handle specular, Handle ao, Handle metal, Handle smooth) {
    InitBase(material);
    if (!diffuse.null())     material.internal_add_component_generic(MaterialComponentType::Diffuse,    diffuse);
    if (!normal.null())      material.internal_add_component_generic(MaterialComponentType::Normal,     normal);
    if (!glow.null())        material.internal_add_component_generic(MaterialComponentType::Glow,       glow);
    if (!specular.null())    material.internal_add_component_generic(MaterialComponentType::Specular,   specular);
    if (!ao.null())          material.internal_add_component_generic(MaterialComponentType::AO,         ao);
    if (!metal.null())       material.internal_add_component_generic(MaterialComponentType::Metalness,  metal);
    if (!smooth.null())      material.internal_add_component_generic(MaterialComponentType::Smoothness, smooth);
}
void Engine::priv::MaterialLoader::InitBase(Material& material) {
    material.m_Components.reserve(MAX_MATERIAL_COMPONENTS);
    material.internal_update_global_material_pool(true);
}

LoadedResource<Texture> Engine::priv::MaterialLoader::internal_load_texture(std::string_view file, bool mipmapped, ImageInternalFormat format, TextureType textureType) {
    LoadedResource<Texture> texture;
    if (!file.empty()) {
        texture = Engine::Resources::getResource<Texture>(file);
        if (!texture.m_Resource) {
            texture.m_Handle    = Engine::Resources::addResource<Texture>(file, mipmapped, format, textureType);
            texture.m_Resource  = texture.m_Handle.get<Texture>();
        }
    }
    return texture;
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
LoadedResource<Texture> Engine::priv::MaterialLoader::LoadTextureCubemap(std::string_view file) {
    return internal_load_texture(file, ENGINE_MIPMAP_DEFAULT, ImageInternalFormat::SRGB8_ALPHA8, TextureType::CubeMap);
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