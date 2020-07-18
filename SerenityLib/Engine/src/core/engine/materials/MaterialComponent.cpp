#include "core/engine/utils/PrecompiledHeader.h"
#include <core/engine/materials/MaterialComponent.h>
#include <core/engine/renderer/Renderer.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/system/Engine.h>
#include <core/engine/scene/Scene.h>
#include <core/engine/scene/Skybox.h>
#include <core/engine/textures/Texture.h>

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

using namespace Engine;
using namespace Engine::priv;
using namespace std;

MaterialComponent::MaterialComponent(MaterialComponentType::Type type, Texture* texture, Texture* mask, Texture* cubemap) {
    m_ComponentType = type;
    addLayer(texture, mask, cubemap);
}
MaterialLayer* MaterialComponent::addLayer(const string& textureFile, const string& maskFile, const string& cubemapFile) {
    if (m_NumLayers == MAX_MATERIAL_LAYERS_PER_COMPONENT) {
        return nullptr;
    }
    Texture* texture, * mask, * cubemap;
    texture = mask = cubemap = nullptr;
    texture = Core::m_Engine->m_ResourceManager.HasResource<Texture>(textureFile);
    mask    = Core::m_Engine->m_ResourceManager.HasResource<Texture>(maskFile);
    cubemap = Core::m_Engine->m_ResourceManager.HasResource<Texture>(cubemapFile);
    if (!texture) {
        texture = NEW Texture(textureFile);
        Core::m_Engine->m_ResourceManager._addTexture(texture);
    }
    if (!mask) {
        mask = NEW Texture(maskFile, false, ImageInternalFormat::R8);
        Core::m_Engine->m_ResourceManager._addTexture(mask);
    }
    if (!cubemap) {
        cubemap = NEW Texture(cubemapFile, false, ImageInternalFormat::SRGB8_ALPHA8, GL_TEXTURE_CUBE_MAP);
        Core::m_Engine->m_ResourceManager._addTexture(cubemap);
    }
    return addLayer(texture, mask, cubemap);
}
MaterialLayer* MaterialComponent::addLayer(Texture* texture, Texture* mask, Texture* cubemap) {
    if (m_NumLayers == MAX_MATERIAL_LAYERS_PER_COMPONENT) {
        return nullptr;
    }
    auto& layer = m_Layers[m_NumLayers];
    layer.setTexture(texture);
    layer.setMask(mask);
    layer.setCubemap(cubemap);

    switch (m_ComponentType) {
        case MaterialComponentType::Diffuse: {
            layer.setData2(1.0f, 1.0f, 1.0f, 1.0f);
            break;
        }case MaterialComponentType::Normal: {
            layer.setData2(1.0f, 1.0f, 1.0f, 1.0f);
            break;
        }case MaterialComponentType::Glow: {
            layer.setData2(0.0f, 1.0f, 1.0f, 0.0f);
            break;
        }case MaterialComponentType::Specular: {
            layer.setData2(0.0f, 1.0f, 1.0f, 0.0f);
            break;
        }case MaterialComponentType::AO: {
            layer.setData2(0.0f, 1.0f, 1.0f, 0.0f);
            break;
        }case MaterialComponentType::Metalness: {
            layer.setData2(0.0f, 1.0f, 1.0f, 0.0f);
            break;
        }case MaterialComponentType::Smoothness: {
            layer.setData2(0.0f, 1.0f, 1.0f, 0.0f);
            break;
        }case MaterialComponentType::Reflection: {
            break;
        }case MaterialComponentType::Refraction: {
            break;
        }case MaterialComponentType::ParallaxOcclusion: {
            break;
        }default: {
            break;
        }
    }
    ++m_NumLayers;
    return &layer;
}

Texture* MaterialComponent::texture(size_t index) const {
    return m_Layers[index].getTexture();
}
Texture* MaterialComponent::mask(size_t index) const {
    return m_Layers[index].getMask();
}
Texture* MaterialComponent::cubemap(size_t index) const {
    return m_Layers[index].getCubemap();
}
MaterialLayer& MaterialComponent::layer(size_t index) {
    return m_Layers[index];
}

void MaterialComponent::bind(size_t component_index, size_t& inTextureUnit) const {
    const string wholeString = "components[" + to_string(component_index) + "].";
    Engine::Renderer::sendUniform2Safe((wholeString + "componentData").c_str(), static_cast<int>(m_NumLayers), static_cast<int>(m_ComponentType));
    for (unsigned int layer = 0; layer < m_NumLayers; ++layer) {
        m_Layers[layer].sendDataToGPU(wholeString, component_index, layer, inTextureUnit);
    }
}
void MaterialComponent::update(const float dt) {
    for (unsigned int i = 0; i < m_NumLayers; ++i) {
        m_Layers[i].update(dt);
    }
}