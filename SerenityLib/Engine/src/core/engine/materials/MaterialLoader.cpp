#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/materials/MaterialLoader.h>
#include <core/engine/materials/MaterialComponent.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/materials/Material.h>
#include <core/engine/system/Engine.h>
#include <core/engine/events/Event.h>

void Engine::priv::MaterialLoader::Init(Material& material, Handle diffuse_ptr, Handle normal_ptr, Handle glow_ptr, Handle specular_ptr, Handle ao_ptr, Handle metalness_ptr, Handle smoothness_ptr) {
    InitBase(material);
    if (!diffuse_ptr.null())     material.internal_add_component_generic(MaterialComponentType::Diffuse,     diffuse_ptr);
    if (!normal_ptr.null())      material.internal_add_component_generic(MaterialComponentType::Normal,      normal_ptr);
    if (!glow_ptr.null())        material.internal_add_component_generic(MaterialComponentType::Glow,        glow_ptr);
    if (!specular_ptr.null())    material.internal_add_component_generic(MaterialComponentType::Specular,    specular_ptr);
    if (!ao_ptr.null())          material.internal_add_component_generic(MaterialComponentType::AO,          ao_ptr);
    if (!metalness_ptr.null())   material.internal_add_component_generic(MaterialComponentType::Metalness,   metalness_ptr);
    if (!smoothness_ptr.null())  material.internal_add_component_generic(MaterialComponentType::Smoothness,  smoothness_ptr);
}
void Engine::priv::MaterialLoader::InitBase(Material& material) {
    material.m_Components.reserve(MAX_MATERIAL_COMPONENTS);
    material.internal_update_global_material_pool(true);
}

std::pair<Texture*, Handle> Engine::priv::MaterialLoader::internal_load_texture(const std::string& file, bool mipmapped, ImageInternalFormat format, TextureType textureType) {
    std::pair<Texture*, Handle> texture = std::make_pair(nullptr, Handle{});
    if (!file.empty()) {
        texture = Engine::Resources::getResource<Texture>(file);
        if (!texture.first) {
            texture.second = Engine::Resources::addResource<Texture>(file, mipmapped, format, textureType);
            texture.first  = texture.second.get<Texture>();
        }
    }
    return texture;
}
std::pair<Texture*, Handle> Engine::priv::MaterialLoader::LoadTextureDiffuse(const std::string& file) {
    return internal_load_texture(file, ENGINE_MIPMAP_DEFAULT, ImageInternalFormat::SRGB8_ALPHA8, TextureType::Texture2D);
}
std::pair<Texture*, Handle> Engine::priv::MaterialLoader::LoadTextureNormal(const std::string& file) {
    return internal_load_texture(file, ENGINE_MIPMAP_DEFAULT, ImageInternalFormat::RGB8, TextureType::Texture2D);
}
std::pair<Texture*, Handle> Engine::priv::MaterialLoader::LoadTextureGlow(const std::string& file) {
    return internal_load_texture(file, ENGINE_MIPMAP_DEFAULT, ImageInternalFormat::R8, TextureType::Texture2D);
}
std::pair<Texture*, Handle> Engine::priv::MaterialLoader::LoadTextureSpecular(const std::string& file) {
    return internal_load_texture(file, ENGINE_MIPMAP_DEFAULT, ImageInternalFormat::R8, TextureType::Texture2D);
}
std::pair<Texture*, Handle> Engine::priv::MaterialLoader::LoadTextureAO(const std::string& file) {
    return internal_load_texture(file, ENGINE_MIPMAP_DEFAULT, ImageInternalFormat::R8, TextureType::Texture2D);
}
std::pair<Texture*, Handle> Engine::priv::MaterialLoader::LoadTextureMetalness(const std::string& file) {
    return internal_load_texture(file, ENGINE_MIPMAP_DEFAULT, ImageInternalFormat::R8, TextureType::Texture2D);
}
std::pair<Texture*, Handle> Engine::priv::MaterialLoader::LoadTextureSmoothness(const std::string& file) {
    return internal_load_texture(file, ENGINE_MIPMAP_DEFAULT, ImageInternalFormat::R8, TextureType::Texture2D);
}
std::pair<Texture*, Handle> Engine::priv::MaterialLoader::LoadTextureMask(const std::string& file) {
    return internal_load_texture(file, ENGINE_MIPMAP_DEFAULT, ImageInternalFormat::R8, TextureType::Texture2D);
}
std::pair<Texture*, Handle> Engine::priv::MaterialLoader::LoadTextureCubemap(const std::string& file) {
    return internal_load_texture(file, ENGINE_MIPMAP_DEFAULT, ImageInternalFormat::SRGB8_ALPHA8, TextureType::CubeMap);
}
void Engine::priv::InternalMaterialPublicInterface::LoadCPU(Material& material) {

}
void Engine::priv::InternalMaterialPublicInterface::LoadGPU(Material& material) {
    //TODO: move this to somewhere with more user control
    for (auto& component : material.m_Components) {
        auto* texture = component.layer().getTexture().get<Texture>();
        if (texture) {
            texture->setAnisotropicFiltering(2.0f);
        }
    }
    material.Resource::load();
}
void Engine::priv::InternalMaterialPublicInterface::UnloadCPU(Material& material) {
    material.Resource::unload();
}
void Engine::priv::InternalMaterialPublicInterface::UnloadGPU(Material& material) {

}
void Engine::priv::InternalMaterialPublicInterface::Load(Material& material) {
    if (!material.isLoaded()) {
        Engine::priv::InternalMaterialPublicInterface::LoadCPU(material);
        Engine::priv::InternalMaterialPublicInterface::LoadGPU(material);
    }
}
void Engine::priv::InternalMaterialPublicInterface::Unload(Material& material) {
    if (material.isLoaded()) {
        Engine::priv::InternalMaterialPublicInterface::UnloadGPU(material);
        Engine::priv::InternalMaterialPublicInterface::UnloadCPU(material);
    }
}