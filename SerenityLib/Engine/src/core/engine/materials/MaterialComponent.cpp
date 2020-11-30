#include <core/engine/utils/PrecompiledHeader.h>
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

MaterialComponent::MaterialComponent(MaterialComponentType type, Handle textureHandle, Handle maskHandle, Handle cubemapHandle)
    : m_ComponentType{ type }
{
    addLayer(textureHandle, maskHandle, cubemapHandle);
}
MaterialComponent::MaterialComponent(MaterialComponent&& other) noexcept 
    : m_Layers        { std::move(other.m_Layers) }
    , m_NumLayers     { std::move(other.m_NumLayers) }
    , m_ComponentType { std::move(other.m_ComponentType) }
{}
MaterialComponent& MaterialComponent::operator=(MaterialComponent&& other) noexcept {
    m_Layers        = std::move(other.m_Layers);
    m_NumLayers     = std::move(other.m_NumLayers);
    m_ComponentType = std::move(other.m_ComponentType);
    return *this;
}

MaterialLayer* MaterialComponent::addLayer(const std::string& textureFile, const std::string& maskFile, const std::string& cubemapFile) {
    if (m_NumLayers == MAX_MATERIAL_LAYERS_PER_COMPONENT) {
        ENGINE_PRODUCTION_LOG("MaterialComponent::addLayer(string...): m_NumLayers == MAX_MATERIAL_LAYERS_PER_COMPONENT!")
        return nullptr;
    }
    auto  texture = Engine::Resources::getResource<Texture>(textureFile);
    auto  mask    = Engine::Resources::getResource<Texture>(maskFile);
    auto  cubemap = Engine::Resources::getResource<Texture>(cubemapFile);

    if (!texture.first && !textureFile.empty()) {
        //texture.second = Engine::Resources::loadTextureAsync(textureFile, ImageInternalFormat::SRGB8_ALPHA8, false);
        texture.second = Engine::Resources::addResource<Texture>(textureFile, false, ImageInternalFormat::SRGB8_ALPHA8, TextureType::Texture2D);
        texture.first  = texture.second.get<Texture>();
    }
    if (!mask.first && !maskFile.empty()) {
        //mask.second = Engine::Resources::loadTextureAsync(maskFile, ImageInternalFormat::R8, false);
        mask.second = Engine::Resources::addResource<Texture>(maskFile, false, ImageInternalFormat::R8, TextureType::Texture2D);
        mask.first  = mask.second.get<Texture>();
    }
    if (!cubemap.first && !cubemapFile.empty()) {
        //cubemap.second = Engine::Resources::loadTextureAsync(cubemapFile, ImageInternalFormat::SRGB8_ALPHA8, false);
        cubemap.second = Engine::Resources::addResource<Texture>(cubemapFile, false, ImageInternalFormat::SRGB8_ALPHA8, TextureType::CubeMap);
        cubemap.first  = cubemap.second.get<Texture>();
    }
    return addLayer(texture.second, mask.second, cubemap.second);
}
MaterialLayer* MaterialComponent::addLayer(Handle textureHandle, Handle maskHandle, Handle cubemapHandle) {
    if (m_NumLayers == MAX_MATERIAL_LAYERS_PER_COMPONENT) {
        ENGINE_PRODUCTION_LOG("MaterialComponent::addLayer(Handle...): m_NumLayers == MAX_MATERIAL_LAYERS_PER_COMPONENT!")
        return nullptr;
    }
    auto& layer = m_Layers[m_NumLayers];
    layer.setTexture(textureHandle);
    layer.setMask(maskHandle);
    layer.setCubemap(cubemapHandle);
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
    }
    ++m_NumLayers;
    return &layer;
}

void MaterialComponent::bind(size_t component_index, size_t& inTextureUnit) const {
    const std::string wholeString = "components[" + std::to_string(component_index) + "].";
    Engine::Renderer::sendUniform2Safe((wholeString + "componentData").c_str(), (int)m_NumLayers, (int)m_ComponentType);
    for (uint32_t layerNumber = 0; layerNumber < m_NumLayers; ++layerNumber) {
        m_Layers[layerNumber].sendDataToGPU(wholeString, component_index, layerNumber, inTextureUnit);
    }
}
void MaterialComponent::update(const float dt) {
    std::for_each_n(std::begin(m_Layers), m_NumLayers, [dt](auto& materialLayer) {
        materialLayer.update(dt);
    });
}