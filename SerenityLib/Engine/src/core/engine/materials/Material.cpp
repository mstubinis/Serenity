#include <core/engine/materials/Material.h>
#include <core/engine/materials/MaterialLoader.h>
#include <core/engine/materials/MaterialComponent.h>
#include <core/engine/system/Engine.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/shaders/ShaderProgram.h>
#include <core/engine/scene/Scene.h>
#include <core/engine/scene/Skybox.h>

#include <algorithm>
#include <iostream>

using namespace Engine;
using namespace Engine::priv;
using namespace std;

vector<glm::vec4> Material::m_MaterialProperities;
Material* Material::Checkers       = nullptr;
Material* Material::WhiteShadeless = nullptr;

vector<tuple<unsigned char, unsigned char, unsigned char, unsigned char, unsigned char>> MATERIAL_PROPERTIES = []() {
    vector<tuple<unsigned char, unsigned char, unsigned char, unsigned char, unsigned char>> m;
    m.resize(MaterialPhysics::_TOTAL);

    m[MaterialPhysics::Aluminium]            = make_tuple(233_uc, 235_uc, 235_uc, 191_uc, 255_uc);
    m[MaterialPhysics::Copper]               = make_tuple(243_uc, 162_uc, 137_uc, 229_uc, 255_uc);
    m[MaterialPhysics::Diamond]              = make_tuple(44_uc, 44_uc, 44_uc, 250_uc, 0_uc);
    m[MaterialPhysics::Glass_Or_Ruby_High]   = make_tuple(20_uc, 20_uc, 20_uc, 250_uc, 0_uc);
    m[MaterialPhysics::Gold]                 = make_tuple(237_uc, 177_uc, 0_uc, 229_uc, 255_uc);
    m[MaterialPhysics::Iron]                 = make_tuple(143_uc, 145_uc, 148_uc, 128_uc, 255_uc);
    m[MaterialPhysics::Plastic_High]         = make_tuple(13_uc, 13_uc, 13_uc, 234_uc, 0_uc);
    m[MaterialPhysics::Plastic_Or_Glass_Low] = make_tuple(8_uc, 8_uc, 8_uc, 246_uc, 0_uc);
    m[MaterialPhysics::Silver]               = make_tuple(242_uc, 237_uc, 224_uc, 240_uc, 255_uc);
    m[MaterialPhysics::Water]                = make_tuple(5_uc, 5_uc, 5_uc, 128_uc, 0_uc);
    m[MaterialPhysics::Skin]                 = make_tuple(7_uc, 7_uc, 7_uc, 25_uc, 0_uc);
    m[MaterialPhysics::Quartz]               = make_tuple(11_uc, 11_uc, 11_uc, 204_uc, 0_uc);
    m[MaterialPhysics::Crystal]              = make_tuple(28_uc, 28_uc, 28_uc, 229_uc, 0_uc);
    m[MaterialPhysics::Alcohol]              = make_tuple(5_uc, 5_uc, 5_uc, 204_uc, 0_uc);
    m[MaterialPhysics::Milk]                 = make_tuple(6_uc, 6_uc, 6_uc, 153_uc, 0_uc);
    m[MaterialPhysics::Glass]                = make_tuple(10_uc, 10_uc, 10_uc, 247_uc, 0_uc);
    m[MaterialPhysics::Titanium]             = make_tuple(138_uc, 126_uc, 114_uc, 232_uc, 255_uc);
    m[MaterialPhysics::Platinum]             = make_tuple(171_uc, 162_uc, 150_uc, 232_uc, 255_uc);
    m[MaterialPhysics::Nickel]               = make_tuple(168_uc, 155_uc, 134_uc, 242_uc, 255_uc);
    m[MaterialPhysics::Black_Leather]        = make_tuple(1_uc, 1_uc, 2_uc, 115_uc, 0_uc);
    m[MaterialPhysics::Yellow_Paint_MERL]    = make_tuple(81_uc, 56_uc, 13_uc, 81_uc, 0_uc);
    m[MaterialPhysics::Chromium]             = make_tuple(140_uc, 141_uc, 141_uc, 204_uc, 255_uc);
    m[MaterialPhysics::Red_Plastic_MERL]     = make_tuple(66_uc, 13_uc, 2_uc, 234_uc, 0_uc);
    m[MaterialPhysics::Blue_Rubber_MERL]     = make_tuple(13_uc, 20_uc, 43_uc, 89_uc, 0_uc);
    m[MaterialPhysics::Zinc]                 = make_tuple(169_uc, 210_uc, 217_uc, 229_uc, 255_uc);
    m[MaterialPhysics::Car_Paint_Orange]     = make_tuple(255_uc, 51_uc, 0_uc, 229_uc, 128_uc);
    return m;
}();


#pragma region Material

Material::Material(const string& name, const string& diffuse, const string& normal, const string& glow, const string& specular, const string& ao, const string& metalness, const string& smoothness) : EngineResource(ResourceType::Material, name){
    Texture* d  = MaterialLoader::LoadTextureDiffuse(diffuse);
    Texture* n  = MaterialLoader::LoadTextureNormal(normal);
    Texture* g  = MaterialLoader::LoadTextureGlow(glow);
    Texture* s  = MaterialLoader::LoadTextureSpecular(specular);
    Texture* a  = MaterialLoader::LoadTextureAO(ao);
    Texture* m  = MaterialLoader::LoadTextureMetalness(metalness);
    Texture* sm = MaterialLoader::LoadTextureSmoothness(smoothness);

    MaterialLoader::InternalInit(*this, d, n, g, s, a, m, sm);
    InternalMaterialPublicInterface::Load(*this);
}
Material::Material() : EngineResource(ResourceType::Material) {
    MaterialLoader::InternalInitBase(*this);
}
Material::Material(const string& name,Texture* diffuse,Texture* normal,Texture* glow,Texture* specular, Texture* ao, Texture* metalness, Texture* smoothness) : EngineResource(ResourceType::Material, name){
    MaterialLoader::InternalInit(*this, diffuse, normal, glow, specular, ao, metalness, smoothness);
    InternalMaterialPublicInterface::Load(*this);
}
Material::~Material(){
    InternalMaterialPublicInterface::Unload(*this);
    SAFE_DELETE_VECTOR(m_Components);
}
MaterialComponent* Material::internalAddComponentGeneric(const MaterialComponentType::Type& type, Texture* texture, Texture* mask, Texture* cubemap) {
    MaterialComponent* newMaterialComponent = NEW MaterialComponent(type, texture, mask, cubemap);
    m_Components.push_back(newMaterialComponent);
    return newMaterialComponent;
}
void Material::internalUpdateGlobalMaterialPool(const bool& addToDatabase) {
    //this data is kept around to be deferred to the lighting pass
    glm::vec4* data = nullptr;
    if (!addToDatabase) {
        data = &Material::m_MaterialProperities[m_ID];
    }else{
        m_ID = static_cast<std::uint32_t>(Material::m_MaterialProperities.size());
        data = NEW glm::vec4(0.0f);
    }
    data->r = Math::pack3FloatsInto1FloatUnsigned(m_F0Color.r(), m_F0Color.g(), m_F0Color.b());
    data->g = static_cast<float>(m_BaseAlpha) * 0.003921568627451f;
    data->b = static_cast<float>(m_SpecularModel);
    data->a = static_cast<float>(m_DiffuseModel);
    if (addToDatabase) {
        Material::m_MaterialProperities.push_back(std::move(*data));
        SAFE_DELETE(data);
    }
}


MaterialComponent& Material::addComponent(const MaterialComponentType::Type& type, const string& textureFile, const string& maskFile, const string& cubemapFile) {
    Texture* texture = Core::m_Engine->m_ResourceManager.HasResource<Texture>(textureFile);
    if (!texture) {
        if (!textureFile.empty() && textureFile != "DEFAULT") {
            if (type == MaterialComponentType::Normal || type == MaterialComponentType::ParallaxOcclusion) {
                texture = NEW Texture(textureFile, true, ImageInternalFormat::RGB8);
            }else{
                texture = NEW Texture(textureFile, true, ImageInternalFormat::SRGB8_ALPHA8);
            }
            Core::m_Engine->m_ResourceManager._addTexture(texture);
        }else if (textureFile == "DEFAULT") {
            texture = Texture::Checkers;
        }else{
        }
    }
    Texture* mask    = MaterialLoader::LoadTextureMask(maskFile);
    Texture* cubemap = MaterialLoader::LoadTextureCubemap(cubemapFile);

    return *internalAddComponentGeneric(type, texture, mask, cubemap);
}
MaterialComponent& Material::addComponentDiffuse(const string& textureFile){
    Texture* texture = MaterialLoader::LoadTextureDiffuse(textureFile);
    return *internalAddComponentGeneric(MaterialComponentType::Diffuse, texture);
}
MaterialComponent& Material::addComponentNormal(const string& textureFile){
    Texture* texture = MaterialLoader::LoadTextureNormal(textureFile);
    return *internalAddComponentGeneric(MaterialComponentType::Normal, texture);
}
MaterialComponent& Material::addComponentGlow(const string& textureFile){
    Texture* texture = MaterialLoader::LoadTextureGlow(textureFile);
    return *internalAddComponentGeneric(MaterialComponentType::Glow, texture);
}
MaterialComponent& Material::addComponentSpecular(const string& textureFile){
    Texture* texture = MaterialLoader::LoadTextureSpecular(textureFile);
    return *internalAddComponentGeneric(MaterialComponentType::Specular, texture);
}
MaterialComponent& Material::addComponentAO(const string& textureFile, const unsigned char baseValue){
    Texture* texture = MaterialLoader::LoadTextureAO(textureFile);
    auto& component = *internalAddComponentGeneric(MaterialComponentType::AO, texture);
    auto& layer = component.layer(0);
    auto& _data2 = layer.data2();
    layer.setData2(0.0f, 1.0f, 1.0f, _data2.w);
    setAO(baseValue);
    return component;
}
MaterialComponent& Material::addComponentMetalness(const string& textureFile, const unsigned char baseValue){
    Texture* texture = MaterialLoader::LoadTextureMetalness(textureFile);
    auto& component = *internalAddComponentGeneric(MaterialComponentType::Metalness, texture);
    auto& layer = component.layer(0);
    auto& _data2 = layer.data2();
    layer.setData2(0.01f, 0.99f, 1.0f, _data2.w);
    setMetalness(baseValue);
    return component;
}
MaterialComponent& Material::addComponentSmoothness(const string& textureFile, const unsigned char baseValue){
    Texture* texture = MaterialLoader::LoadTextureSmoothness(textureFile);
    auto& component = *internalAddComponentGeneric(MaterialComponentType::Smoothness, texture);
    auto& layer = component.layer(0);
    auto& _data2 = layer.data2();
    layer.setData2(0.01f, 0.99f, 1.0f, _data2.w);
    setSmoothness(baseValue);
    return component;
}
MaterialComponent& Material::addComponentReflection(const string& cubemapName, const string& maskFile,float mixFactor){
    //add checks to see if texture was loaded already
    Texture* cubemap = MaterialLoader::LoadTextureCubemap(cubemapName);
    Texture* mask = MaterialLoader::LoadTextureMask(maskFile);
    if (!cubemap)
        cubemap = Resources::getCurrentScene()->skybox()->texture();
    auto& component = *internalAddComponentGeneric(MaterialComponentType::Reflection, nullptr);
    auto& layer = component.layer(0);
    auto& _data2 = layer.data2();
    layer.setMask(mask);
    layer.setCubemap(cubemap);
    layer.setData2(mixFactor, _data2.y, _data2.z, _data2.w);
    return component;
}
MaterialComponent& Material::addComponentRefraction(const string& cubemapName, const string& maskFile,float refractiveIndex,float mixFactor){
    //add checks to see if texture was loaded already
    Texture* cubemap = MaterialLoader::LoadTextureCubemap(cubemapName);
    Texture* mask = MaterialLoader::LoadTextureMask(maskFile);
    if (!cubemap)
        cubemap = Resources::getCurrentScene()->skybox()->texture();
    auto& component = *internalAddComponentGeneric(MaterialComponentType::Refraction, nullptr);
    auto& layer = component.layer(0);
    auto& _data2 = layer.data2();
    layer.setMask(mask);
    layer.setCubemap(cubemap);
    layer.setData2(mixFactor, refractiveIndex, _data2.z, _data2.w);
    return component;
}
MaterialComponent& Material::addComponentParallaxOcclusion(const string& textureFile,float heightScale){
    Texture* texture = MaterialLoader::LoadTextureNormal(textureFile);
    auto& component = *internalAddComponentGeneric(MaterialComponentType::ParallaxOcclusion, texture);
    auto& layer = component.layer(0);
    auto& _data2 = layer.data2();
    layer.setData2(heightScale, _data2.y, _data2.z, _data2.w);
    return component;
}
MaterialComponent& Material::getComponent(const unsigned int& index) {
    return *m_Components[index];
}
const bool& Material::shadeless() const { 
    return m_Shadeless; 
}
const Engine::color_vector_4& Material::f0() const{
    return m_F0Color; 
}
const unsigned char Material::glow() const {
    return m_BaseGlow; 
}
const std::uint32_t& Material::id() const { 
    return m_ID; 
}
const unsigned char Material::diffuseModel() const {
    return m_DiffuseModel; 
}
const unsigned char Material::specularModel() const {
    return m_SpecularModel; 
}
const unsigned char Material::ao() const {
    return m_BaseAO; 
}
const unsigned char Material::metalness() const{
    return m_BaseMetalness; 
}
const unsigned char Material::smoothness() const {
    return m_BaseSmoothness; 
}
const unsigned char Material::alpha() const {
    return m_BaseAlpha;
}
void Material::setShadeless(const bool shadeless){
    m_Shadeless = shadeless;
    internalUpdateGlobalMaterialPool(false);
}
void Material::setGlow(const unsigned char glow){
    m_BaseGlow = glm::clamp(glow, 1_uc, 255_uc);
    internalUpdateGlobalMaterialPool(false);
}
void Material::setF0Color(const glm::vec3& color){
    Material::setF0Color(color.r, color.g, color.b);
}
void Material::setF0Color(const float r, const float g, const float b){
    m_F0Color = Engine::color_vector_4(
        glm::clamp(r, 0.01f, 0.99f),
        glm::clamp(g, 0.01f, 0.99f),
        glm::clamp(b, 0.01f, 0.99f),
        0.99f
    );
    internalUpdateGlobalMaterialPool(false);
}
void Material::setMaterialPhysics(const MaterialPhysics::Physics materialPhysics){
    const auto& t = MATERIAL_PROPERTIES[materialPhysics];
    setF0Color(get<0>(t), get<1>(t), get<2>(t));
    setSmoothness(get<3>(t));
    setMetalness(get<4>(t));
    internalUpdateGlobalMaterialPool(false);
}
void Material::setSmoothness(const unsigned char smoothness){
    m_BaseSmoothness = glm::clamp(smoothness, 1_uc, 255_uc);
    internalUpdateGlobalMaterialPool(false);
}
void Material::setSpecularModel(const SpecularModel::Model specularModel){
    m_SpecularModel = specularModel;
    internalUpdateGlobalMaterialPool(false);
}
void Material::setDiffuseModel(const DiffuseModel::Model diffuseModel){
    m_DiffuseModel = diffuseModel;
    internalUpdateGlobalMaterialPool(false);
}
void Material::setAO(const unsigned char ao){
    m_BaseAO = glm::clamp(ao, 1_uc, 255_uc);
    internalUpdateGlobalMaterialPool(false);
}
void Material::setMetalness(const unsigned char metalness){
    m_BaseMetalness = glm::clamp(metalness, 1_uc, 255_uc);
    internalUpdateGlobalMaterialPool(false);
}
void Material::setAlpha(const unsigned char alpha) {
    m_BaseAlpha = glm::clamp(alpha, 1_uc, 255_uc);
    internalUpdateGlobalMaterialPool(false);
}
void Material::update(const float dt) {
    for (size_t i = 0; i < m_Components.size(); ++i) {
        auto* component = m_Components[i];
        if (!component)
            break; //this assumes there will never be a null component before defined components. be careful here
        component->update(dt);
    }
}
#pragma endregion