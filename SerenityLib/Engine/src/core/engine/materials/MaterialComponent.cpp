#include <core/engine/materials/MaterialComponent.h>
#include <core/engine/renderer/Engine_Renderer.h>
#include <core/engine/resources/Engine_Resources.h>
#include <core/engine/scene/Scene.h>
#include <core/engine/scene/Skybox.h>
#include <core/engine/textures/Texture.h>

#include <GL/glew.h>
#include <SFML/OpenGL.hpp>
#include <boost/tuple/tuple.hpp>

using namespace Engine;
using namespace std;


struct MaterialComponentTextureSlot { enum Slot {
    Diffuse,
    Normal,
    Glow,
    Specular,
    AO,
    Metalness,
    Smoothness,
    Reflection_CUBEMAP,
    Reflection_CUBEMAP_MAP,
    Refraction_CUBEMAP,
    Refraction_CUBEMAP_MAP,
    Heightmap,
    _TOTAL
};};
vector<vector<uint>> MATERIAL_TEXTURE_SLOTS_MAP = []() {
    vector<vector<uint>> m; m.resize(MaterialComponentType::_TOTAL);

    m[MaterialComponentType::Diffuse].push_back(MaterialComponentTextureSlot::Diffuse);
    m[MaterialComponentType::Normal].push_back(MaterialComponentTextureSlot::Normal);
    m[MaterialComponentType::Glow].push_back(MaterialComponentTextureSlot::Glow);
    m[MaterialComponentType::Specular].push_back(MaterialComponentTextureSlot::Specular);
    m[MaterialComponentType::AO].push_back(MaterialComponentTextureSlot::AO);
    m[MaterialComponentType::Metalness].push_back(MaterialComponentTextureSlot::Metalness);
    m[MaterialComponentType::Smoothness].push_back(MaterialComponentTextureSlot::Smoothness);
    m[MaterialComponentType::Reflection].push_back(MaterialComponentTextureSlot::Reflection_CUBEMAP);
    m[MaterialComponentType::Reflection].push_back(MaterialComponentTextureSlot::Reflection_CUBEMAP_MAP);
    m[MaterialComponentType::Refraction].push_back(MaterialComponentTextureSlot::Refraction_CUBEMAP);
    m[MaterialComponentType::Refraction].push_back(MaterialComponentTextureSlot::Refraction_CUBEMAP_MAP);
    m[MaterialComponentType::ParallaxOcclusion].push_back(MaterialComponentTextureSlot::Heightmap);

    return m;
}();
const GLchar* MATERIAL_COMPONENT_SHADER_TEXTURE_NAMES[MaterialComponentType::Type::_TOTAL] = {
    "DiffuseTexture",
    "NormalTexture",
    "GlowTexture",
    "SpecularTexture",
    "AOTexture",
    "MetalnessTexture",
    "SmoothnessTexture",
    "ReflectionTexture",
    "RefractionTexture",
    "HeightmapTexture",
};


MaterialComponent::MaterialComponent(const MaterialComponentType::Type& type, Texture* texture, Texture* mask, Texture* cubemap) {
    m_ComponentType = type;
    m_NumLayers = 0;

    auto& layer = m_Layers[m_NumLayers];
    layer.setTexture(texture);
    layer.setMask(mask);
    layer.setCubemap(cubemap);

    ++m_NumLayers;
}
MaterialComponent::~MaterialComponent() {
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

void MaterialComponent::bind(const uint& component_index) {
    const auto& slots             = MATERIAL_TEXTURE_SLOTS_MAP[m_ComponentType];
    const string& textureTypeName = MATERIAL_COMPONENT_SHADER_TEXTURE_NAMES[m_ComponentType];
    for (uint i = 0; i < slots.size(); ++i) {
        Renderer::sendTextureSafe(textureTypeName.c_str(), *m_Layers[0].getTexture(), slots[i]);
    }

    //new material system
    //const string wholeString = "components[" + to_string(component_index) + "].";
    //Renderer::sendUniform1Safe((wholeString + "numLayers").c_str(), static_cast<int>(m_NumLayers));
    //Renderer::sendUniform1Safe((wholeString + "componentType").c_str(), static_cast<int>(m_ComponentType));
    //for (unsigned int i = 0; i < m_NumLayers; ++i) {
        //m_Layers[i].sendDataToGPU(wholeString, component_index, i);
    //}
    /////////////////////
}
void MaterialComponent::unbind() {
}
void MaterialComponent::update(const float& dt) {
    for (unsigned int i = 0; i < m_NumLayers; ++i) {
        m_Layers[i].update(dt);
    }
}