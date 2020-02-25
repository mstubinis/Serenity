#include <core/engine/materials/MaterialLoader.h>
#include <core/engine/materials/MaterialComponent.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/materials/Material.h>
#include <core/engine/system/Engine.h>
#include <core/engine/events/Engine_EventObject.h>

#include <iostream>

using namespace Engine;
using namespace std;

namespace Engine::priv {
    struct DefaultMaterialBindFunctor { void operator()(Material* material_ptr) const {
        auto& material             = *material_ptr;
        const size_t numComponents = material.m_Components.size();
        size_t textureUnit         = 0;

        for (size_t i = 0; i < numComponents; ++i) {
            if (material.m_Components[i]) {
                const auto& c = *material.m_Components[i];
                c.bind(i, textureUnit);
            }
        }

        Engine::Renderer::sendUniform1Safe("numComponents", int(numComponents));
        Engine::Renderer::sendUniform1Safe("Shadeless", static_cast<int>(material.m_Shadeless));
        Engine::Renderer::sendUniform4Safe("Material_F0AndID", 
            material.m_F0Color.r(), 
            material.m_F0Color.g(), 
            material.m_F0Color.b(), 
            static_cast<float>(material.m_ID)
        );
        Engine::Renderer::sendUniform4Safe("MaterialBasePropertiesOne", 
            static_cast<float>(material.m_BaseGlow) * 0.003921568627451f, 
            static_cast<float>(material.m_BaseAO) * 0.003921568627451f,
            static_cast<float>(material.m_BaseMetalness) * 0.003921568627451f,
            static_cast<float>(material.m_BaseSmoothness) * 0.003921568627451f
        );
        Engine::Renderer::sendUniform4Safe("MaterialBasePropertiesTwo", 
            static_cast<float>(material.m_BaseAlpha) * 0.003921568627451f,
            static_cast<float>(material.m_DiffuseModel), 
            static_cast<float>(material.m_SpecularModel), 
            0.0f
        );
    }};
};

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

    material.setCustomBindFunctor(priv::DefaultMaterialBindFunctor());
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
    material.EngineResource::load();

    Event e(EventType::MaterialLoaded);
    e.eventMaterialLoaded = EventMaterialLoaded(&material);
    priv::Core::m_Engine->m_EventManager.m_EventDispatcher.dispatchEvent(e);
}
void priv::InternalMaterialPublicInterface::UnloadCPU(Material& material) {
    material.EngineResource::unload();
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