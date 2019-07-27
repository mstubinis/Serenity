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


MaterialComponent::MaterialComponent(const uint& type, Texture* texture) {
    m_ComponentType = static_cast<MaterialComponentType::Type>(type);
    m_NumLayers = 0;

    m_Layers[m_NumLayers].setTexture(texture);
    ++m_NumLayers;
}
MaterialComponent::~MaterialComponent() {
}
Texture* MaterialComponent::texture(const uint& index) const {
    return m_Layers[index].getTexture();
}
const MaterialComponentType::Type& MaterialComponent::type() const {
    return m_ComponentType;
}

void MaterialComponent::bind(glm::vec4& data) {
    const auto& slots = MATERIAL_TEXTURE_SLOTS_MAP[m_ComponentType];
    const string& textureTypeName = MATERIAL_COMPONENT_SHADER_TEXTURE_NAMES[m_ComponentType];
    const auto& textureTypeNameC = textureTypeName.c_str();
    for (uint i = 0; i < slots.size(); ++i) {
        Renderer::sendTextureSafe(textureTypeNameC, *m_Layers[0].getTexture(), slots[i]);
    }
}
void MaterialComponent::unbind() {
}




MaterialComponentReflection::MaterialComponentReflection(const uint& type, Texture* cubemap, Texture* map, const float& mixFactor) :MaterialComponent(type, cubemap) {
    setMixFactor(mixFactor);

    m_Layers[m_NumLayers].setCubemap(cubemap);
    m_Layers[m_NumLayers].setMask(map);
    m_Layers[m_NumLayers].setTexture(map);
    ++m_NumLayers;
}
MaterialComponentReflection::~MaterialComponentReflection() {
    MaterialComponent::~MaterialComponent();
}
void MaterialComponentReflection::setMixFactor(const float& factor) {
    m_MixFactor = glm::clamp(factor, 0.0f, 1.0f);
}
void MaterialComponentReflection::bind(glm::vec4& data) {
    const auto& slots = MATERIAL_TEXTURE_SLOTS_MAP[m_ComponentType];
    const string& textureTypeName = MATERIAL_COMPONENT_SHADER_TEXTURE_NAMES[m_ComponentType];
    const auto& textureTypeNameC = textureTypeName.c_str();
    data.x = m_MixFactor;
    if (!m_Layers[0].getCubemap())
        Renderer::sendTextureSafe(textureTypeNameC, *Resources::getCurrentScene()->skybox()->texture(), slots[0]);
    else
        Renderer::sendTextureSafe(textureTypeNameC, *m_Layers[0].getCubemap(), slots[0]);
    Renderer::sendTextureSafe((textureTypeName + "Map").c_str(), *m_Layers[0].getMask(), slots[1]);
}
void MaterialComponentReflection::unbind() {
}
MaterialComponentRefraction::MaterialComponentRefraction(Texture* cubemap, Texture* map, const float& i, const float& mix) :MaterialComponentReflection(MaterialComponentType::Refraction, cubemap, map, mix) {
    m_RefractionIndex = i;
}
MaterialComponentRefraction::~MaterialComponentRefraction() {
    MaterialComponentReflection::~MaterialComponentReflection();
}
void MaterialComponentRefraction::setRefractionIndex(const float& refractionIndex) {
    m_RefractionIndex = refractionIndex;
}
void MaterialComponentRefraction::bind(glm::vec4& data) {
    const auto& slots = MATERIAL_TEXTURE_SLOTS_MAP[m_ComponentType];
    const string& textureTypeName = MATERIAL_COMPONENT_SHADER_TEXTURE_NAMES[m_ComponentType];
    const auto& textureTypeNameC = textureTypeName.c_str();
    data.x = m_MixFactor;
    data.y = m_RefractionIndex;
    if (!m_Layers[0].getCubemap())
        Renderer::sendTextureSafe(textureTypeNameC, *Resources::getCurrentScene()->skybox()->texture(), slots[0]);
    else
        Renderer::sendTextureSafe(textureTypeNameC, *m_Layers[0].getCubemap(), slots[0]);
    Renderer::sendTextureSafe((textureTypeName + "Map").c_str(), *m_Layers[0].getMask(), slots[1]);
}

MaterialComponentParallaxOcclusion::MaterialComponentParallaxOcclusion(Texture* heightmap, const float& heightScale) :MaterialComponent(MaterialComponentType::ParallaxOcclusion, heightmap) {
    setHeightScale(heightScale);
}
MaterialComponentParallaxOcclusion::~MaterialComponentParallaxOcclusion() {
    MaterialComponent::~MaterialComponent();
}
void MaterialComponentParallaxOcclusion::setHeightScale(const float& factor) {
    m_HeightScale = factor;
}
void MaterialComponentParallaxOcclusion::bind(glm::vec4& data) {
    const auto& slots = MATERIAL_TEXTURE_SLOTS_MAP[m_ComponentType];
    const string& textureTypeName = MATERIAL_COMPONENT_SHADER_TEXTURE_NAMES[m_ComponentType];

    data.z = m_HeightScale;

    Renderer::sendTextureSafe(textureTypeName.c_str(), *m_Layers[0].getTexture(), slots[0]);
}
void MaterialComponentParallaxOcclusion::unbind() {
}