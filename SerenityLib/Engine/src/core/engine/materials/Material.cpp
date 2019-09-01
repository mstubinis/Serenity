#include <core/engine/materials/Material.h>
#include <core/engine/materials/MaterialLoader.h>
#include <core/engine/Engine.h>
#include <core/engine/textures/Texture.h>
#include <core/engine/math/Engine_Math.h>
#include <core/engine/shaders/ShaderProgram.h>
#include <core/engine/scene/Scene.h>
#include <core/engine/scene/Skybox.h>

#include <algorithm>
#include <iostream>
#include <boost/tuple/tuple.hpp>

using namespace Engine;
using namespace Engine::epriv;
using namespace std;

vector<glm::vec4> Material::m_MaterialProperities;
Material* Material::Checkers       = nullptr;
Material* Material::WhiteShadeless = nullptr;

vector<boost::tuple<float,float,float,float,float>> MATERIAL_PROPERTIES = [](){
    vector<boost::tuple<float,float,float,float,float>> m; m.resize(MaterialPhysics::_TOTAL);
    //Remember specular reflection of non metals is white!       //(F0)                         //Smoothness    //Metalness
    m[MaterialPhysics::Aluminium]            = boost::make_tuple(0.9131f,0.9215f,0.92452f,      0.75f,          1.0f);
    m[MaterialPhysics::Copper]               = boost::make_tuple(0.955f,0.6374f,0.5381f,        0.9f,           1.0f);
    m[MaterialPhysics::Diamond]              = boost::make_tuple(0.17196f,0.17196f,0.17196f,    0.98f,          0.0f);
    m[MaterialPhysics::Glass_Or_Ruby_High]   = boost::make_tuple(0.0773f,0.0773f,0.0773f,       0.98f,          0.0f);
    m[MaterialPhysics::Gold]                 = boost::make_tuple(0.929f,0.6549f,0.0f,           0.9f,           1.0f);
    m[MaterialPhysics::Iron]                 = boost::make_tuple(0.56f,0.57f,0.58f,             0.5f,           1.0f);
    m[MaterialPhysics::Plastic_High]         = boost::make_tuple(0.05f,0.05f,0.05f,             0.92f,          0.0f);
    m[MaterialPhysics::Plastic_Or_Glass_Low] = boost::make_tuple(0.03f,0.03f,0.03f,             0.965f,         0.0f);
    m[MaterialPhysics::Silver]               = boost::make_tuple(0.95f,0.93f,0.88f,             0.94f,          1.0f);
    m[MaterialPhysics::Water]                = boost::make_tuple(0.02f,0.02f,0.02f,             0.5f,           0.0f);
    m[MaterialPhysics::Skin]                 = boost::make_tuple(0.028f,0.028f,0.028f,          0.1f,           0.0f);
    m[MaterialPhysics::Quartz]               = boost::make_tuple(0.045594f,0.045594f,0.04554f,  0.8f,           0.0f);
    m[MaterialPhysics::Crystal]              = boost::make_tuple(0.11111f,0.11111f,0.11111f,    0.9f,           0.0f);
    m[MaterialPhysics::Alcohol]              = boost::make_tuple(0.01995f,0.01995f,0.01995f,    0.8f,           0.0f);
    m[MaterialPhysics::Milk]                 = boost::make_tuple(0.02218f,0.02218f,0.02218f,    0.6f,           0.0f);
    m[MaterialPhysics::Glass]                = boost::make_tuple(0.04f,0.04f,0.04f,             0.97f,          0.0f);
    m[MaterialPhysics::Titanium]             = boost::make_tuple(0.5419f,0.4967f,0.4494f,       0.91f,          1.0f);
    m[MaterialPhysics::Platinum]             = boost::make_tuple(0.6724f,0.6373f,0.5854f,       0.91f,          1.0f);
    m[MaterialPhysics::Nickel]               = boost::make_tuple(0.6597f,0.6086f,0.5256f,       0.95f,          1.0f);
    m[MaterialPhysics::Black_Leather]        = boost::make_tuple(0.006f,0.005f,0.007f,          0.45f,          0.0f);
    m[MaterialPhysics::Yellow_Paint_MERL]    = boost::make_tuple(0.32f,0.22f,0.05f,             0.32f,          0.0f);
    m[MaterialPhysics::Chromium]             = boost::make_tuple(0.549f,0.556f,0.554f,          0.8f,           1.0f);
    m[MaterialPhysics::Red_Plastic_MERL]     = boost::make_tuple(0.26f,0.05f,0.01f,             0.92f,          0.0f);
    m[MaterialPhysics::Blue_Rubber_MERL]     = boost::make_tuple(0.05f,0.08f,0.17f,             0.35f,          0.0f);
    m[MaterialPhysics::Zinc]                 = boost::make_tuple(0.664f,0.824f,0.85f,           0.9f,           1.0f);
    m[MaterialPhysics::Car_Paint_Orange]     = boost::make_tuple(1.0f,0.2f,0.0f,                0.9f,           0.5f);
    return m;
}();


#pragma region Material

Material::Material(const string& name, const string& diffuse, const string& normal, const string& glow, const string& specular):BindableResource(name){
    Texture* d = 0; Texture* n = 0; Texture* g = 0; Texture* s = 0;

    d = MaterialLoader::LoadTextureDiffuse(*this, diffuse);
    n = MaterialLoader::LoadTextureNormal(*this, normal);
    g = MaterialLoader::LoadTextureGlow(*this, glow);
    s = MaterialLoader::LoadTextureSpecular(*this, specular);

    MaterialLoader::InternalInit(*this, d, n, g, s);
    load();
}
Material::Material() {
    MaterialLoader::InternalInitBase(*this);
}
Material::Material(const string& name,Texture* diffuse,Texture* normal,Texture* glow,Texture* specular):BindableResource(name){
    MaterialLoader::InternalInit(*this, diffuse, normal, glow, specular);
    load();
}
Material::~Material(){
    SAFE_DELETE_VECTOR(m_Components);
}
MaterialComponent* Material::internalAddComponentGeneric(const MaterialComponentType::Type& type, Texture* texture, Texture* mask, Texture* cubemap) {
    MaterialComponent* newMaterialComponent = new MaterialComponent(type, texture, mask, cubemap);
    m_Components.push_back(newMaterialComponent);
    return newMaterialComponent;
}
void Material::internalUpdateGlobalMaterialPool(const bool& addToDatabase) {
    //this data is kept around to be deferred to the lighting pass
    glm::vec4* data = nullptr;
    if (!addToDatabase) {
        data = &Material::m_MaterialProperities[m_ID];
    }else{
        m_ID = Material::m_MaterialProperities.size();
        data = new glm::vec4(0.0f);
    }
    data->r = Math::pack3FloatsInto1FloatUnsigned(m_F0Color.r, m_F0Color.g, m_F0Color.b);
    data->g = m_BaseAlpha;
    data->b = static_cast<float>(m_SpecularModel);
    data->a = static_cast<float>(m_DiffuseModel);
    if (addToDatabase) {
        Material::m_MaterialProperities.push_back(std::move(*data));
        SAFE_DELETE(data);
    }
}


MaterialComponent& Material::addComponent(const MaterialComponentType::Type& type, const string& textureFile, const string& maskFile, const string& cubemapFile) {
    Texture* texture = Core::m_Engine->m_ResourceManager._hasTexture(textureFile);
    if (!texture && !textureFile.empty()) {
        if (type == MaterialComponentType::Normal || type == MaterialComponentType::ParallaxOcclusion) {
            texture = new Texture(textureFile, true, ImageInternalFormat::RGB8);
        }else{
            texture = new Texture(textureFile, true, ImageInternalFormat::SRGB8_ALPHA8);
        }
        Core::m_Engine->m_ResourceManager._addTexture(texture);
    }
    Texture* mask = MaterialLoader::LoadTextureMask(*this, maskFile);
    Texture* cubemap = MaterialLoader::LoadTextureCubemap(*this, cubemapFile);

    return *internalAddComponentGeneric(type, texture, mask, cubemap);
}
MaterialComponent& Material::addComponentDiffuse(const string& textureFile){
    Texture* texture = MaterialLoader::LoadTextureDiffuse(*this, textureFile);
    auto& component = *internalAddComponentGeneric(MaterialComponentType::Diffuse, texture);
    return component;
}
MaterialComponent& Material::addComponentNormal(const string& textureFile){
    Texture* texture = MaterialLoader::LoadTextureNormal(*this, textureFile);
    auto& component = *internalAddComponentGeneric(MaterialComponentType::Normal, texture);
    return component;
}
MaterialComponent& Material::addComponentGlow(const string& textureFile){
    Texture* texture = MaterialLoader::LoadTextureGlow(*this, textureFile);
    auto& component = *internalAddComponentGeneric(MaterialComponentType::Glow, texture);
    return component;
}
MaterialComponent& Material::addComponentSpecular(const string& textureFile){
    Texture* texture = MaterialLoader::LoadTextureSpecular(*this, textureFile);
    auto& component = *internalAddComponentGeneric(MaterialComponentType::Specular, texture);
    return component;
}
MaterialComponent& Material::addComponentAO(const string& textureFile,float baseValue){
    Texture* texture = MaterialLoader::LoadTextureAO(*this, textureFile);
    auto& component = *internalAddComponentGeneric(MaterialComponentType::AO, texture);
    auto& layer = component.layer(0);
    auto& _data2 = layer.data2();
    layer.setData2(0.0f, 1.0f, 1.0f, _data2.w);
    setAO(baseValue);
    return component;
}
MaterialComponent& Material::addComponentMetalness(const string& textureFile,float baseValue){
    Texture* texture = MaterialLoader::LoadTextureMetalness(*this, textureFile);
    auto& component = *internalAddComponentGeneric(MaterialComponentType::Metalness, texture);
    auto& layer = component.layer(0);
    auto& _data2 = layer.data2();
    layer.setData2(0.01f, 0.99f, 1.0f, _data2.w);
    setMetalness(baseValue);
    return component;
}
MaterialComponent& Material::addComponentSmoothness(const string& textureFile,float baseValue){
    Texture* texture = MaterialLoader::LoadTextureSmoothness(*this, textureFile);
    auto& component = *internalAddComponentGeneric(MaterialComponentType::Smoothness, texture);
    auto& layer = component.layer(0);
    auto& _data2 = layer.data2();
    layer.setData2(0.01f, 0.99f, 1.0f, _data2.w);
    setSmoothness(baseValue);
    return component;
}
MaterialComponent& Material::addComponentReflection(const string& cubemapName, const string& maskFile,float mixFactor){
    //add checks to see if texture was loaded already
    Texture* cubemap = MaterialLoader::LoadTextureCubemap(*this, cubemapName);
    Texture* mask = MaterialLoader::LoadTextureMask(*this, maskFile);
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
    Texture* cubemap = MaterialLoader::LoadTextureCubemap(*this, cubemapName);
    Texture* mask = MaterialLoader::LoadTextureMask(*this, maskFile);
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
    Texture* texture = MaterialLoader::LoadTextureNormal(*this, textureFile);
    auto& component = *internalAddComponentGeneric(MaterialComponentType::ParallaxOcclusion, texture);
    auto& layer = component.layer(0);
    auto& _data2 = layer.data2();
    layer.setData2(heightScale, _data2.y, _data2.z, _data2.w);
    return component;
}
MaterialComponent& Material::getComponent(const uint& index) {
    return *m_Components[index];
}
const bool& Material::shadeless() const { 
    return m_Shadeless; 
}
const glm::vec3& Material::f0() const{ 
    return m_F0Color; 
}
const float& Material::glow() const { 
    return m_BaseGlow; 
}
const uint& Material::id() const { 
    return m_ID; 
}
const uint& Material::diffuseModel() const { 
    return m_DiffuseModel; 
}
const uint& Material::specularModel() const { 
    return m_SpecularModel; 
}
const float& Material::ao() const { 
    return m_BaseAO; 
}
const float& Material::metalness() const{ 
    return m_BaseMetalness; 
}
const float& Material::smoothness() const { 
    return m_BaseSmoothness; 
}
const float& Material::alpha() const {
    return m_BaseAlpha;
}
void Material::setShadeless(const bool& shadeless){
    m_Shadeless = shadeless;
    internalUpdateGlobalMaterialPool(false);
}
void Material::setGlow(const float& glow){
    m_BaseGlow = glow;
    internalUpdateGlobalMaterialPool(false);
}
void Material::setF0Color(const glm::vec3& color){
    setF0Color(color.r, color.g, color.b); 
}
void Material::setF0Color(const float& r, const float& g, const float& b){
    m_F0Color.r = glm::clamp(r, 0.01f, 0.99f);
    m_F0Color.g = glm::clamp(g, 0.01f, 0.99f);
    m_F0Color.b = glm::clamp(b, 0.01f, 0.99f);
    internalUpdateGlobalMaterialPool(false);
}
void Material::setMaterialPhysics(const MaterialPhysics::Physics& materialPhysics){
    auto& t = MATERIAL_PROPERTIES[materialPhysics];
    setF0Color(t.get<0>(), t.get<1>(), t.get<2>());
    setSmoothness(t.get<3>());
    setMetalness(t.get<4>());
    internalUpdateGlobalMaterialPool(false);
}
void Material::setSmoothness(const float& smoothness){
    m_BaseSmoothness = glm::clamp(smoothness, 0.001f, 0.999f);
    internalUpdateGlobalMaterialPool(false);
}
void Material::setSpecularModel(const SpecularModel::Model& specularModel){
    m_SpecularModel = specularModel;
    internalUpdateGlobalMaterialPool(false);
}
void Material::setDiffuseModel(const DiffuseModel::Model& diffuseModel){
    m_DiffuseModel = diffuseModel;
    internalUpdateGlobalMaterialPool(false);
}
void Material::setAO(const float& ao){
    m_BaseAO = glm::clamp(ao, 0.001f, 0.999f);
    internalUpdateGlobalMaterialPool(false);
}
void Material::setMetalness(const float& metalness){
    m_BaseMetalness = glm::clamp(metalness, 0.001f, 0.999f);
    internalUpdateGlobalMaterialPool(false);
}
void Material::setAlpha(const float& alpha) {
    m_BaseAlpha = glm::clamp(alpha, 0.0f, 1.0f);
    internalUpdateGlobalMaterialPool(false);
}
void Material::bind(){ 
    Core::m_Engine->m_RenderManager._bindMaterial(this); 
}
void Material::unbind(){ 
    Core::m_Engine->m_RenderManager._unbindMaterial(); 
}
void Material::load(){
    if(!isLoaded()){
        auto& _this = *this;
        InternalMaterialPublicInterface::LoadCPU(_this);
        InternalMaterialPublicInterface::LoadGPU(_this);
    }
}
void Material::unload(){
    if(isLoaded()){
        auto& _this = *this;
        InternalMaterialPublicInterface::UnloadGPU(_this);
        InternalMaterialPublicInterface::UnloadCPU(_this);
    }
}
void Material::update(const double& dt) {
    for (uint i = 0; i < m_Components.size(); ++i) {
        MaterialComponent* component = m_Components[i];
        if (!component)
            break; //this assumes there will never be a null component before defined components. be careful here
        component->update(dt);
    }
}
#pragma endregion