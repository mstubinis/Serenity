
#include <serenity/resources/material/MaterialComponent.h>
#include <serenity/renderer/Renderer.h>
#include <serenity/resources/Engine_Resources.h>
#include <serenity/system/Engine.h>
#include <serenity/scene/Scene.h>
#include <serenity/scene/Skybox.h>
#include <serenity/resources/texture/Texture.h>

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>

using namespace Engine;
using namespace Engine::priv;

constexpr std::array<glm::vec4, (size_t)MaterialComponentType::_TOTAL> MISC_DATA_INFO { {
    {1.0f, 1.0f, 1.0f, 1.0f}, //Diffuse
    {1.0f, 1.0f, 1.0f, 1.0f}, //Normal
    {0.0f, 1.0f, 1.0f, 0.0f}, //Glow
    {0.0f, 1.0f, 1.0f, 0.0f}, //Specular
    {0.0f, 1.0f, 1.0f, 0.0f}, //AO
    {0.0f, 1.0f, 1.0f, 0.0f}, //Metalness
    {0.0f, 1.0f, 1.0f, 0.0f}, //Smoothness
    {1.0f, 1.0f, 1.0f, 1.0f}, //Reflection
    {1.0f, 1.0f, 1.0f, 1.0f}, //Refraction
    {1.0f, 1.0f, 1.0f, 1.0f}, //ParallaxOcclusion
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
    auto texture = Engine::Resources::getResource<Texture>(textureFile);
    auto mask    = Engine::Resources::getResource<Texture>(maskFile);
    auto cubemap = Engine::Resources::getResource<Texture>(cubemapFile);

    if (!texture.m_Resource && !textureFile.empty()) {
        //texture.m_Handle = Engine::Resources::loadTextureAsync(textureFile, ImageInternalFormat::SRGB8_ALPHA8, false);
        texture.m_Handle   = Engine::Resources::addResource<Texture>(textureFile, false, ImageInternalFormat::SRGB8_ALPHA8, TextureType::Texture2D);
        texture.m_Resource = texture.m_Handle.get<Texture>();
    }
    if (!mask.m_Resource && !maskFile.empty()) {
        //mask.m_Handle = Engine::Resources::loadTextureAsync(maskFile, ImageInternalFormat::R8, false);
        mask.m_Handle   = Engine::Resources::addResource<Texture>(maskFile, false, ImageInternalFormat::R8, TextureType::Texture2D);
        mask.m_Resource = mask.m_Handle.get<Texture>();
    }
    if (!cubemap.m_Resource && !cubemapFile.empty()) {
        //cubemap.m_Handle = Engine::Resources::loadTextureAsync(cubemapFile, ImageInternalFormat::SRGB8_ALPHA8, false);
        cubemap.m_Handle   = Engine::Resources::addResource<Texture>(cubemapFile, false, ImageInternalFormat::SRGB8_ALPHA8, TextureType::CubeMap);
        cubemap.m_Resource = cubemap.m_Handle.get<Texture>();
    }
    return addLayer(texture.m_Handle, mask.m_Handle, cubemap.m_Handle);
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
    layer.setMiscData(MISC_DATA_INFO[(size_t)m_ComponentType]);
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