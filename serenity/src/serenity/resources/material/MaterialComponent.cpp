
#include <serenity/resources/material/MaterialComponent.h>
#include <serenity/renderer/Renderer.h>
#include <serenity/resources/Engine_Resources.h>
#include <serenity/system/Engine.h>
#include <serenity/scene/Scene.h>
#include <serenity/scene/Skybox.h>
#include <serenity/resources/texture/Texture.h>
#include <serenity/resources/texture/TextureCubemap.h>

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

namespace {
    std::string WHOLE_STR_BUFFER;
}

constexpr std::array<glm::vec4, MaterialComponentType::_TOTAL> MISC_DATA_INFO { {
    { 1.0f, 1.0f, 1.0f, 1.0f }, // Diffuse
    { 1.0f, 1.0f, 1.0f, 1.0f }, // Normal
    { 0.0f, 1.0f, 1.0f, 0.0f }, // Glow
    { 0.0f, 1.0f, 1.0f, 0.0f }, // Specular
    { 0.0f, 1.0f, 1.0f, 0.0f }, // Metalness
    { 0.0f, 1.0f, 1.0f, 0.0f }, // Smoothness
    { 0.0f, 1.0f, 1.0f, 0.0f }, // AO
    { 1.0f, 1.0f, 1.0f, 1.0f }, // Reflection
    { 1.0f, 1.0f, 1.0f, 1.0f }, // Refraction
    { 1.0f, 1.0f, 1.0f, 1.0f }, // ParallaxOcclusion
    { 0.0f, 0.0f, 0.0f, 0.0f }, // Empty
} };

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
    if (this != &other) {
        m_Layers        = std::move(other.m_Layers);
        m_NumLayers     = std::move(other.m_NumLayers);
        m_ComponentType = std::move(other.m_ComponentType);
    }
    return *this;
}
MaterialLayer* MaterialComponent::addLayer(const std::string& textureFile, const std::string& maskFile, const std::string& cubemapFile) {
    if (m_NumLayers == MAX_MATERIAL_LAYERS_PER_COMPONENT) {
        ENGINE_PRODUCTION_LOG(__FUNCTION__ << "(string...): m_NumLayers == MAX_MATERIAL_LAYERS_PER_COMPONENT!")
        return nullptr;
    }
    auto texture = Engine::Resources::getResource<Texture>(textureFile);
    auto mask    = Engine::Resources::getResource<Texture>(maskFile);
    auto cubemap = Engine::Resources::getResource<TextureCubemap>(cubemapFile);

    if (!texture.m_Resource && !textureFile.empty()) {
        texture  = Engine::Resources::addResource<Texture>(textureFile, false, ImageInternalFormat::SRGB8_ALPHA8, TextureType::Texture2D, true);
    }
    if (!mask.m_Resource && !maskFile.empty()) {
        mask     = Engine::Resources::addResource<Texture>(maskFile, false, ImageInternalFormat::R8, TextureType::Texture2D, true);
    }
    if (!cubemap.m_Resource && !cubemapFile.empty()) {
        cubemap  = Engine::Resources::addResource<TextureCubemap>(cubemapFile, false, ImageInternalFormat::SRGB8_ALPHA8);
    }
    return addLayer(texture.m_Handle, mask.m_Handle, cubemap.m_Handle);
}
MaterialLayer* MaterialComponent::addLayer(Handle textureHandle, Handle maskHandle, Handle cubemapHandle) {
    if (m_NumLayers == MAX_MATERIAL_LAYERS_PER_COMPONENT) {
        ENGINE_PRODUCTION_LOG(__FUNCTION__ << "(Handle...): m_NumLayers == MAX_MATERIAL_LAYERS_PER_COMPONENT!")
        return nullptr;
    }
    auto& layer = m_Layers[m_NumLayers];
    layer.m_MaterialLayerBaseData.enabled = 1.0f;
    layer.setTexture(textureHandle);
    layer.setMask(maskHandle);
    layer.setCubemap(cubemapHandle);
    layer.setMiscData(MISC_DATA_INFO[m_ComponentType]);
    ++m_NumLayers;
    return &layer;
}
void MaterialComponent::bind(size_t component_index, int& inTextureUnit) const {
    WHOLE_STR_BUFFER.clear();
    WHOLE_STR_BUFFER += "components[" + std::to_string(component_index) + "].";
    Engine::Renderer::sendUniform2Safe((WHOLE_STR_BUFFER + "compDat").c_str(), int(m_NumLayers), int(bool(m_ComponentType != MaterialComponentType::Empty)));
    for (uint32_t layerNumber = 0; layerNumber < MAX_MATERIAL_LAYERS_PER_COMPONENT; ++layerNumber) {
        m_Layers[layerNumber].sendDataToGPU(WHOLE_STR_BUFFER, layerNumber, inTextureUnit);
    }
}
void MaterialComponent::update(const float dt) {
    std::for_each_n(std::begin(m_Layers), m_NumLayers, [dt](auto& materialLayer) {
        materialLayer.update(dt);
    });
}