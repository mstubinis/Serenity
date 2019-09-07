#include <core/engine/materials/MaterialLoader.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/materials/Material.h>
#include <core/engine/Engine.h>

#include <iostream>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;

namespace Engine {
    namespace epriv {
        struct DefaultMaterialBindFunctor {
            void operator()(BindableResource* r) const {
                auto& material = *static_cast<Material*>(r);
                const int& numComponents = material.m_Components.size();
                uint textureUnit = 0;
                for (int i = 0; i < numComponents; ++i) {
                    if (material.m_Components[i]) {
                        auto& component = *material.m_Components[i];
                        component.bind(i, textureUnit);
                    }
                }
                Renderer::sendUniform1Safe("numComponents", numComponents);
                Renderer::sendUniform1Safe("Shadeless", static_cast<int>(material.m_Shadeless));
                Renderer::sendUniform4Safe("Material_F0AndID", material.m_F0Color.r, material.m_F0Color.g, material.m_F0Color.b, static_cast<float>(material.m_ID));
                Renderer::sendUniform4Safe("MaterialBasePropertiesOne", material.m_BaseGlow, material.m_BaseAO, material.m_BaseMetalness, material.m_BaseSmoothness);
                Renderer::sendUniform4Safe("MaterialBasePropertiesTwo", material.m_BaseAlpha, static_cast<float>(material.m_DiffuseModel), static_cast<float>(material.m_SpecularModel), 0.0f);
            }
        };
        struct DefaultMaterialUnbindFunctor {
            void operator()(BindableResource* r) const {
                //auto& material = *static_cast<Material*>(r);
            }
        };
    };
};

void MaterialLoader::InternalInit(Material& material, Texture* diffuse_ptr, Texture* normal_ptr, Texture* glow_ptr, Texture* specular_ptr) {
    InternalInitBase(material);
    if (diffuse_ptr)   material.internalAddComponentGeneric(MaterialComponentType::Diffuse,  diffuse_ptr);
    if (normal_ptr)    material.internalAddComponentGeneric(MaterialComponentType::Normal,   normal_ptr);
    if (glow_ptr)      material.internalAddComponentGeneric(MaterialComponentType::Glow,     glow_ptr);
    if (specular_ptr)  material.internalAddComponentGeneric(MaterialComponentType::Specular, specular_ptr);
}
void MaterialLoader::InternalInitBase(Material& material) {
    material.m_Components.reserve(MAX_MATERIAL_COMPONENTS);
    material.internalUpdateGlobalMaterialPool(true);
    material.setDiffuseModel(DiffuseModel::Lambert);
    material.setSpecularModel(SpecularModel::GGX);
    material.setShadeless(false);
    material.setGlow(0.0f);
    material.setSmoothness(0.25f);
    material.setAlpha(1.0f);
    material.setAO(1.0f);
    material.setMetalness(0.0f);
    material.setF0Color(0.04f, 0.04f, 0.04f);
    material.setCustomBindFunctor(epriv::DefaultMaterialBindFunctor());
    material.setCustomUnbindFunctor(epriv::DefaultMaterialUnbindFunctor());
}

Texture* MaterialLoader::LoadTextureDiffuse(const string& file) {
    Texture* texture = nullptr;
    if (!file.empty()) {
        texture = Core::m_Engine->m_ResourceManager._hasTexture(file);
        if (!texture) {
            texture = new Texture(file, true, ImageInternalFormat::SRGB8_ALPHA8, GL_TEXTURE_2D);
            Core::m_Engine->m_ResourceManager._addTexture(texture);
        }
    }
    return texture;
}
Texture* MaterialLoader::LoadTextureNormal(const string& file) {
    Texture* texture = nullptr;
    if (!file.empty()) {
        texture = Core::m_Engine->m_ResourceManager._hasTexture(file);
        if (!texture) {
            texture = new Texture(file, false, ImageInternalFormat::RGB8, GL_TEXTURE_2D);
            Core::m_Engine->m_ResourceManager._addTexture(texture);
        }
    }
    return texture;
}
Texture* MaterialLoader::LoadTextureGlow(const string& file) {
    Texture* texture = nullptr;
    if (!file.empty()) {
        texture = Core::m_Engine->m_ResourceManager._hasTexture(file);
        if (!texture) {
            texture = new Texture(file, false, ImageInternalFormat::R8, GL_TEXTURE_2D);
            Core::m_Engine->m_ResourceManager._addTexture(texture);
        }
    }
    return texture;
}
Texture* MaterialLoader::LoadTextureSpecular(const string& file) {
    Texture* texture = nullptr;
    if (!file.empty()) {
        texture = Core::m_Engine->m_ResourceManager._hasTexture(file);
        if (!texture) {
            texture = new Texture(file, false, ImageInternalFormat::R8, GL_TEXTURE_2D);
            Core::m_Engine->m_ResourceManager._addTexture(texture);
        }
    }
    return texture;
}
Texture* MaterialLoader::LoadTextureAO(const string& file) {
    Texture* texture = nullptr;
    if (!file.empty()) {
        texture = Core::m_Engine->m_ResourceManager._hasTexture(file);
        if (!texture) {
            texture = new Texture(file, false, ImageInternalFormat::R8, GL_TEXTURE_2D);
            Core::m_Engine->m_ResourceManager._addTexture(texture);
        }
    }
    return texture;
}
Texture* MaterialLoader::LoadTextureMetalness(const string& file) {
    Texture* texture = nullptr;
    if (!file.empty()) {
        texture = Core::m_Engine->m_ResourceManager._hasTexture(file);
        if (!texture) {
            texture = new Texture(file, false, ImageInternalFormat::R8, GL_TEXTURE_2D);
            Core::m_Engine->m_ResourceManager._addTexture(texture);
        }
    }
    return texture;
}
Texture* MaterialLoader::LoadTextureSmoothness(const string& file) {
    Texture* texture = nullptr;
    if (!file.empty()) {
        texture = Core::m_Engine->m_ResourceManager._hasTexture(file);
        if (!texture) {
            texture = new Texture(file, false, ImageInternalFormat::R8, GL_TEXTURE_2D);
            Core::m_Engine->m_ResourceManager._addTexture(texture);
        }
    }
    return texture;
}
Texture* MaterialLoader::LoadTextureMask(const string& file) {
    Texture* texture = nullptr;
    if (!file.empty()) {
        texture = Core::m_Engine->m_ResourceManager._hasTexture(file);
        if (!texture) {
            texture = new Texture(file, false, ImageInternalFormat::R8, GL_TEXTURE_2D);
            Core::m_Engine->m_ResourceManager._addTexture(texture);
        }
    }
    return texture;
}
Texture* MaterialLoader::LoadTextureCubemap(const string& file) {
    Texture* texture = nullptr;
    if (!file.empty()) {
        texture = Core::m_Engine->m_ResourceManager._hasTexture(file);
        if (!texture) {
            texture = new Texture(file, false, ImageInternalFormat::SRGB8_ALPHA8, GL_TEXTURE_CUBE_MAP);
            Core::m_Engine->m_ResourceManager._addTexture(texture);
        }
    }
    return texture;
}
void InternalMaterialPublicInterface::LoadCPU(Material& material) {

}
void InternalMaterialPublicInterface::LoadGPU(Material& material) {
    /*
    for (auto& component : m_Components) {
        if (component) {
            Texture& texture = *component->texture();
            texture.incrementUseCount();
            if (!texture.isLoaded() && texture.useCount() > 0) {
                texture.load();
            }
        }
    }
    */
    //TODO: move this to somewhere with more user control
    for (auto& component : material.m_Components) {
        if (component) {
            auto* texture = component->layer().getTexture();
            if (texture) {
                texture->setAnisotropicFiltering(2.0f);
            }
        }
    }
    cout << "(Material) ";
    material.EngineResource::load();
}
void InternalMaterialPublicInterface::UnloadCPU(Material& material) {
    cout << "(Material) ";
    material.EngineResource::unload();
}
void InternalMaterialPublicInterface::UnloadGPU(Material& material) {
    /*
    for (auto& component : m_Components) {
        if (component) {
            Texture& texture = *component->texture();
            texture.decrementUseCount();
            if (texture.useCount() == 0 && texture.isLoaded()) {
                texture.unload();
            }
        }
    }
    */
}
void InternalMaterialPublicInterface::Load(Material& material) {
    if (!material.isLoaded()) {
        InternalMaterialPublicInterface::LoadCPU(material);
        InternalMaterialPublicInterface::LoadGPU(material);
    }
}
void InternalMaterialPublicInterface::Unload(Material& material) {
    if (material.isLoaded()) {
        InternalMaterialPublicInterface::UnloadGPU(material);
        InternalMaterialPublicInterface::UnloadCPU(material);
    }
}