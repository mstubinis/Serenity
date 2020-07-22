#include <core/engine/utils/PrecompiledHeader.h>
#include <core/engine/materials/MaterialLoader.h>
#include <core/engine/materials/MaterialComponent.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/materials/Material.h>
#include <core/engine/system/Engine.h>
#include <core/engine/events/Event.h>

using namespace Engine;
using namespace std;

void priv::MaterialLoader::InternalInit(Material& material, Texture* diffuse_ptr, Texture* normal_ptr, Texture* glow_ptr, Texture* specular_ptr, Texture* ao_ptr, Texture* metalness_ptr, Texture* smoothness_ptr) {
    priv::MaterialLoader::InternalInitBase(material);
    if (diffuse_ptr)     material.internalAddComponentGeneric(MaterialComponentType::Diffuse,  diffuse_ptr);
    if (normal_ptr)      material.internalAddComponentGeneric(MaterialComponentType::Normal,   normal_ptr);
    if (glow_ptr)        material.internalAddComponentGeneric(MaterialComponentType::Glow,     glow_ptr);
    if (specular_ptr)    material.internalAddComponentGeneric(MaterialComponentType::Specular, specular_ptr);
    if (ao_ptr)          material.internalAddComponentGeneric(MaterialComponentType::AO, ao_ptr);
    if (metalness_ptr)   material.internalAddComponentGeneric(MaterialComponentType::Metalness, metalness_ptr);
    if (smoothness_ptr)  material.internalAddComponentGeneric(MaterialComponentType::Smoothness, smoothness_ptr);
}
void priv::MaterialLoader::InternalInitBase(Material& material) {
    material.m_Components.reserve(MAX_MATERIAL_COMPONENTS);

    material.internalUpdateGlobalMaterialPool(true);
}

Texture* priv::MaterialLoader::LoadTextureDiffuse(const string& file) {
    Texture* texture = nullptr;
    if (!file.empty()) {
        texture = priv::Core::m_Engine->m_ResourceManager.HasResource<Texture>(file);
        if (!texture) {
            texture = NEW Texture(file, true, ImageInternalFormat::SRGB8_ALPHA8, GL_TEXTURE_2D);
            priv::Core::m_Engine->m_ResourceManager._addTexture(texture);
        }
    }
    return texture;
}
Texture* priv::MaterialLoader::LoadTextureNormal(const string& file) {
    Texture* texture = nullptr;
    if (!file.empty()) {
        texture = priv::Core::m_Engine->m_ResourceManager.HasResource<Texture>(file);
        if (!texture) {
            texture = NEW Texture(file, true, ImageInternalFormat::RGB8, GL_TEXTURE_2D);
            priv::Core::m_Engine->m_ResourceManager._addTexture(texture);
        }
    }
    return texture;
}
Texture* priv::MaterialLoader::LoadTextureGlow(const string& file) {
    Texture* texture = nullptr;
    if (!file.empty()) {
        texture = Core::m_Engine->m_ResourceManager.HasResource<Texture>(file);
        if (!texture) {
            texture = NEW Texture(file, true, ImageInternalFormat::R8, GL_TEXTURE_2D);
            priv::Core::m_Engine->m_ResourceManager._addTexture(texture);
        }
    }
    return texture;
}
Texture* priv::MaterialLoader::LoadTextureSpecular(const string& file) {
    Texture* texture = nullptr;
    if (!file.empty()) {
        texture = priv::Core::m_Engine->m_ResourceManager.HasResource<Texture>(file);
        if (!texture) {
            texture = NEW Texture(file, false, ImageInternalFormat::R8, GL_TEXTURE_2D);
            priv::Core::m_Engine->m_ResourceManager._addTexture(texture);
        }
    }
    return texture;
}
Texture* priv::MaterialLoader::LoadTextureAO(const string& file) {
    Texture* texture = nullptr;
    if (!file.empty()) {
        texture = priv::Core::m_Engine->m_ResourceManager.HasResource<Texture>(file);
        if (!texture) {
            texture = NEW Texture(file, false, ImageInternalFormat::R8, GL_TEXTURE_2D);
            priv::Core::m_Engine->m_ResourceManager._addTexture(texture);
        }
    }
    return texture;
}
Texture* priv::MaterialLoader::LoadTextureMetalness(const string& file) {
    Texture* texture = nullptr;
    if (!file.empty()) {
        texture = priv::Core::m_Engine->m_ResourceManager.HasResource<Texture>(file);
        if (!texture) {
            texture = NEW Texture(file, false, ImageInternalFormat::R8, GL_TEXTURE_2D);
            priv::Core::m_Engine->m_ResourceManager._addTexture(texture);
        }
    }
    return texture;
}
Texture* priv::MaterialLoader::LoadTextureSmoothness(const string& file) {
    Texture* texture = nullptr;
    if (!file.empty()) {
        texture = priv::Core::m_Engine->m_ResourceManager.HasResource<Texture>(file);
        if (!texture) {
            texture = NEW Texture(file, false, ImageInternalFormat::R8, GL_TEXTURE_2D);
            priv::Core::m_Engine->m_ResourceManager._addTexture(texture);
        }
    }
    return texture;
}
Texture* priv::MaterialLoader::LoadTextureMask(const string& file) {
    Texture* texture = nullptr;
    if (!file.empty()) {
        texture = priv::Core::m_Engine->m_ResourceManager.HasResource<Texture>(file);
        if (!texture) {
            texture = NEW Texture(file, false, ImageInternalFormat::R8, GL_TEXTURE_2D);
            priv::Core::m_Engine->m_ResourceManager._addTexture(texture);
        }
    }
    return texture;
}
Texture* priv::MaterialLoader::LoadTextureCubemap(const string& file) {
    Texture* texture = nullptr;
    if (!file.empty()) {
        texture = priv::Core::m_Engine->m_ResourceManager.HasResource<Texture>(file);
        if (!texture) {
            texture = NEW Texture(file, false, ImageInternalFormat::SRGB8_ALPHA8, GL_TEXTURE_CUBE_MAP);
            priv::Core::m_Engine->m_ResourceManager._addTexture(texture);
        }
    }
    return texture;
}
void priv::InternalMaterialPublicInterface::LoadCPU(Material& material) {

}
void priv::InternalMaterialPublicInterface::LoadGPU(Material& material) {
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
void priv::InternalMaterialPublicInterface::UnloadCPU(Material& material) {
    material.Resource::unload();
}
void priv::InternalMaterialPublicInterface::UnloadGPU(Material& material) {

}
void priv::InternalMaterialPublicInterface::Load(Material& material) {
    if (!material.isLoaded()) {
        priv::InternalMaterialPublicInterface::LoadCPU(material);
        priv::InternalMaterialPublicInterface::LoadGPU(material);
    }
}
void priv::InternalMaterialPublicInterface::Unload(Material& material) {
    if (material.isLoaded()) {
        priv::InternalMaterialPublicInterface::UnloadGPU(material);
        priv::InternalMaterialPublicInterface::UnloadCPU(material);
    }
}