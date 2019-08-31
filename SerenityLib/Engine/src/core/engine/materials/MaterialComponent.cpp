#include <core/engine/materials/MaterialComponent.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/Engine.h>
#include <core/engine/scene/Scene.h>
#include <core/engine/scene/Skybox.h>
#include <core/engine/textures/Texture.h>

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <boost/tuple/tuple.hpp>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;

MaterialComponent::MaterialComponent(const MaterialComponentType::Type& type, Texture* texture, Texture* mask, Texture* cubemap) {
    m_ComponentType = type;
    m_NumLayers = 0;

    addLayer(texture, mask, cubemap);
}
MaterialComponent::~MaterialComponent() {
}
MaterialLayer* MaterialComponent::addLayer(const string& textureFile, const string& maskFile, const string& cubemapFile) {
    if (m_NumLayers == MAX_MATERIAL_LAYERS_PER_COMPONENT)
        return nullptr;
    Texture* texture, * mask, * cubemap;
    texture = mask = cubemap = nullptr;
    texture = Core::m_Engine->m_ResourceManager._hasTexture(textureFile);
    mask = Core::m_Engine->m_ResourceManager._hasTexture(maskFile);
    cubemap = Core::m_Engine->m_ResourceManager._hasTexture(cubemapFile);
    if (!texture) {
        texture = new Texture(textureFile);
        Core::m_Engine->m_ResourceManager._addTexture(texture);
    }
    if (!mask) {
        mask = new Texture(maskFile, false, ImageInternalFormat::R8);
        Core::m_Engine->m_ResourceManager._addTexture(mask);
    }
    if (!cubemap) {
        cubemap = new Texture(cubemapFile, false, ImageInternalFormat::SRGB8_ALPHA8, GL_TEXTURE_CUBE_MAP);
        Core::m_Engine->m_ResourceManager._addTexture(cubemap);
    }
    return addLayer(texture, mask, cubemap);
}
MaterialLayer* MaterialComponent::addLayer(Texture* texture, Texture* mask, Texture* cubemap) {
    if (m_NumLayers == MAX_MATERIAL_LAYERS_PER_COMPONENT)
        return nullptr;
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
        }
        default: {
            break;
        }
    }

    ++m_NumLayers;
    return &layer;
}

Texture* MaterialComponent::texture(const uint& index) const {
    return m_Layers[index].getTexture();
}
Texture* MaterialComponent::mask(const uint& index) const {
    return m_Layers[index].getMask();
}
Texture* MaterialComponent::cubemap(const uint& index) const {
    return m_Layers[index].getCubemap();
}
MaterialLayer& MaterialComponent::layer(const uint& index) {
    return m_Layers[index];
}
const MaterialComponentType::Type& MaterialComponent::type() const {
    return m_ComponentType;
}

void MaterialComponent::bind(const uint& component_index, uint& textureUnit) {
    const string wholeString = "components[" + to_string(component_index) + "].";
    Renderer::sendUniform1Safe((wholeString + "numLayers").c_str(), static_cast<int>(m_NumLayers));
    Renderer::sendUniform1Safe((wholeString + "componentType").c_str(), static_cast<int>(m_ComponentType));
    for (uint layerIndex = 0; layerIndex < m_NumLayers; ++layerIndex) {
        m_Layers[layerIndex].sendDataToGPU(wholeString, component_index, layerIndex, textureUnit);
    }
}
void MaterialComponent::unbind() {
}
void MaterialComponent::update(const double& dt) {
    for (unsigned int i = 0; i < m_NumLayers; ++i) {
        m_Layers[i].update(dt);
    }
}