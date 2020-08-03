#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/materials/MaterialLoader.h>
#include <core/engine/materials/MaterialComponent.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/materials/Material.h>
#include <core/engine/system/Engine.h>
#include <core/engine/events/Event.h>

void Engine::priv::MaterialLoader::Init(Material& material, Texture* diffuse_ptr, Texture* normal_ptr, Texture* glow_ptr, Texture* specular_ptr, Texture* ao_ptr, Texture* metalness_ptr, Texture* smoothness_ptr) {
    InitBase(material);
    if (diffuse_ptr)     material.internal_add_component_generic(MaterialComponentType::Diffuse,  diffuse_ptr);
    if (normal_ptr)      material.internal_add_component_generic(MaterialComponentType::Normal,   normal_ptr);
    if (glow_ptr)        material.internal_add_component_generic(MaterialComponentType::Glow,     glow_ptr);
    if (specular_ptr)    material.internal_add_component_generic(MaterialComponentType::Specular, specular_ptr);
    if (ao_ptr)          material.internal_add_component_generic(MaterialComponentType::AO, ao_ptr);
    if (metalness_ptr)   material.internal_add_component_generic(MaterialComponentType::Metalness, metalness_ptr);
    if (smoothness_ptr)  material.internal_add_component_generic(MaterialComponentType::Smoothness, smoothness_ptr);
}
void Engine::priv::MaterialLoader::InitBase(Material& material) {
    material.m_Components.reserve(MAX_MATERIAL_COMPONENTS);
    material.internal_update_global_material_pool(true);
}

Texture* Engine::priv::MaterialLoader::internal_load_texture(const std::string& file, bool mipmapped, ImageInternalFormat format, unsigned int gl_type) {
    Texture* texture = nullptr;
    if (!file.empty()) {
        texture = Engine::priv::Core::m_Engine->m_ResourceManager.HasResource<Texture>(file);
        if (!texture) {
            texture = NEW Texture(file, mipmapped, format, gl_type);
            Engine::priv::Core::m_Engine->m_ResourceManager._addTexture(texture);
        }
    }
    return texture;
}
Texture* Engine::priv::MaterialLoader::LoadTextureDiffuse(const std::string& file) {
    return internal_load_texture(file, false, ImageInternalFormat::SRGB8_ALPHA8, GL_TEXTURE_2D);
}
Texture* Engine::priv::MaterialLoader::LoadTextureNormal(const std::string& file) {
    return internal_load_texture(file, false, ImageInternalFormat::RGB8, GL_TEXTURE_2D);
}
Texture* Engine::priv::MaterialLoader::LoadTextureGlow(const std::string& file) {
    return internal_load_texture(file, false, ImageInternalFormat::R8, GL_TEXTURE_2D);
}
Texture* Engine::priv::MaterialLoader::LoadTextureSpecular(const std::string& file) {
    return internal_load_texture(file, false, ImageInternalFormat::R8, GL_TEXTURE_2D);
}
Texture* Engine::priv::MaterialLoader::LoadTextureAO(const std::string& file) {
    return internal_load_texture(file, false, ImageInternalFormat::R8, GL_TEXTURE_2D);
}
Texture* Engine::priv::MaterialLoader::LoadTextureMetalness(const std::string& file) {
    return internal_load_texture(file, false, ImageInternalFormat::R8, GL_TEXTURE_2D);
}
Texture* Engine::priv::MaterialLoader::LoadTextureSmoothness(const std::string& file) {
    return internal_load_texture(file, false, ImageInternalFormat::R8, GL_TEXTURE_2D);
}
Texture* Engine::priv::MaterialLoader::LoadTextureMask(const std::string& file) {
    return internal_load_texture(file, false, ImageInternalFormat::R8, GL_TEXTURE_2D);
}
Texture* Engine::priv::MaterialLoader::LoadTextureCubemap(const std::string& file) {
    return internal_load_texture(file, false, ImageInternalFormat::SRGB8_ALPHA8, GL_TEXTURE_CUBE_MAP);
}
void Engine::priv::InternalMaterialPublicInterface::LoadCPU(Material& material) {

}
void Engine::priv::InternalMaterialPublicInterface::LoadGPU(Material& material) {
    //TODO: move this to somewhere with more user control
    for (auto& component : material.m_Components) {
        if (component) {
            auto* texture = component->layer().getTexture();
            if (texture) {
                texture->setAnisotropicFiltering(2.0f);
            }
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