
#include <serenity/core/engine/materials/MaterialLoader.h>
#include <serenity/core/engine/materials/MaterialComponent.h>
#include <serenity/core/engine/textures/Texture.h>
#include <serenity/core/engine/materials/Material.h>
#include <serenity/core/engine/system/Engine.h>
#include <serenity/core/engine/events/Event.h>

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

std::pair<Engine::view_ptr<Texture>, Handle> Engine::priv::MaterialLoader::internal_load_texture(const std::string& file, bool mipmapped, ImageInternalFormat format, TextureType textureType) {
    std::pair<Engine::view_ptr<Texture>, Handle> texture = std::make_pair(nullptr, Handle{});
    if (!file.empty()) {
        texture = Engine::Resources::getResource<Texture>(file);
        if (!texture.first) {
            texture.second = Engine::Resources::addResource<Texture>(file, mipmapped, format, textureType);
            texture.first  = texture.second.get<Texture>();
        }
    }
    return texture;
}
std::pair<Engine::view_ptr<Texture>, Handle> Engine::priv::MaterialLoader::LoadTextureDiffuse(const std::string& file) {
    return internal_load_texture(file, ENGINE_MIPMAP_DEFAULT, ImageInternalFormat::SRGB8_ALPHA8, TextureType::Texture2D);
}
std::pair<Engine::view_ptr<Texture>, Handle> Engine::priv::MaterialLoader::LoadTextureNormal(const std::string& file) {
    return internal_load_texture(file, ENGINE_MIPMAP_DEFAULT, ImageInternalFormat::RGB8, TextureType::Texture2D);
}
std::pair<Engine::view_ptr<Texture>, Handle> Engine::priv::MaterialLoader::LoadTextureGlow(const std::string& file) {
    return internal_load_texture(file, ENGINE_MIPMAP_DEFAULT, ImageInternalFormat::R8, TextureType::Texture2D);
}
std::pair<Engine::view_ptr<Texture>, Handle> Engine::priv::MaterialLoader::LoadTextureSpecular(const std::string& file) {
    return internal_load_texture(file, ENGINE_MIPMAP_DEFAULT, ImageInternalFormat::R8, TextureType::Texture2D);
}
std::pair<Engine::view_ptr<Texture>, Handle> Engine::priv::MaterialLoader::LoadTextureAO(const std::string& file) {
    return internal_load_texture(file, ENGINE_MIPMAP_DEFAULT, ImageInternalFormat::R8, TextureType::Texture2D);
}
std::pair<Engine::view_ptr<Texture>, Handle> Engine::priv::MaterialLoader::LoadTextureMetalness(const std::string& file) {
    return internal_load_texture(file, ENGINE_MIPMAP_DEFAULT, ImageInternalFormat::R8, TextureType::Texture2D);
}
std::pair<Engine::view_ptr<Texture>, Handle> Engine::priv::MaterialLoader::LoadTextureSmoothness(const std::string& file) {
    return internal_load_texture(file, ENGINE_MIPMAP_DEFAULT, ImageInternalFormat::R8, TextureType::Texture2D);
}
std::pair<Engine::view_ptr<Texture>, Handle> Engine::priv::MaterialLoader::LoadTextureMask(const std::string& file) {
    return internal_load_texture(file, ENGINE_MIPMAP_DEFAULT, ImageInternalFormat::R8, TextureType::Texture2D);
}
std::pair<Engine::view_ptr<Texture>, Handle> Engine::priv::MaterialLoader::LoadTextureCubemap(const std::string& file) {
    return internal_load_texture(file, ENGINE_MIPMAP_DEFAULT, ImageInternalFormat::SRGB8_ALPHA8, TextureType::CubeMap);
}
void Engine::priv::InternalMaterialPublicInterface::LoadGPU(Material& material) {
    material.Resource::load();
}
void Engine::priv::InternalMaterialPublicInterface::UnloadCPU(Material& material) {
    material.Resource::unload();
}
void Engine::priv::InternalMaterialPublicInterface::Load(Material& material) {
    if (!material.isLoaded()) {
        Engine::priv::InternalMaterialPublicInterface::LoadGPU(material);
    }
}
void Engine::priv::InternalMaterialPublicInterface::Unload(Material& material) {
    if (material.isLoaded()) {
        Engine::priv::InternalMaterialPublicInterface::UnloadCPU(material);
    }
}