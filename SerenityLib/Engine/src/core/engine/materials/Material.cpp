#include <core/engine/materials/Material.h>
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
Material* Material::Checkers = nullptr;


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

namespace Engine{
    namespace epriv{
        struct DefaultMaterialBindFunctor{void operator()(BindableResource* r) const {
            auto& material = *static_cast<Material*>(r);
            const int& numComponents = material.m_Components.size();
            for(int i = 0; i < numComponents; ++i){
                if(material.m_Components[i]){
                    auto& component = *material.m_Components[i];
                    component.bind(i);
                }
            }
            Renderer::sendUniform1Safe("numComponents", numComponents);
            Renderer::sendUniform1Safe("Shadeless", static_cast<int>(material.m_Shadeless));
            Renderer::sendUniform4Safe("Material_F0AndID", material.m_F0Color.r, material.m_F0Color.g, material.m_F0Color.b, static_cast<float>(material.m_ID));
            Renderer::sendUniform4Safe("MaterialBasePropertiesOne", material.m_BaseGlow, material.m_BaseAO, material.m_BaseMetalness, material.m_BaseSmoothness);
        }};
        struct DefaultMaterialUnbindFunctor{void operator()(BindableResource* r) const {
            //auto& material = *static_cast<Material*>(r);
        }};
    };
};


#pragma region Material

Material::Material(const string& name, const string& diffuse, const string& normal, const string& glow, const string& specular):BindableResource(name){
    Texture* d = 0; Texture* n = 0; Texture* g = 0; Texture* s = 0;
    if (!diffuse.empty()) {
        d = Core::m_Engine->m_ResourceManager._hasTexture(diffuse);
        if (!d) {
            d = new Texture(diffuse);
            Core::m_Engine->m_ResourceManager._addTexture(d);
        }
    }
    if (!normal.empty()) {
        n = Core::m_Engine->m_ResourceManager._hasTexture(normal);
        if (!n) {
            n = new Texture(normal, false, ImageInternalFormat::RGB8);
            Core::m_Engine->m_ResourceManager._addTexture(n);
        }
    }
    if (!glow.empty()) {
        g = Core::m_Engine->m_ResourceManager._hasTexture(glow);
        if (!g) {
            g = new Texture(glow, false, ImageInternalFormat::R8);
            Core::m_Engine->m_ResourceManager._addTexture(g);
        }
    }
    if (!specular.empty()) {
        s = Core::m_Engine->m_ResourceManager._hasTexture(specular);
        if (!s) {
            s = new Texture(specular, false, ImageInternalFormat::R8);
            Core::m_Engine->m_ResourceManager._addTexture(s);
        }
    }
    internalInit(d, n, g, s);
}
Material::Material(const string& name,Texture* diffuse,Texture* normal,Texture* glow,Texture* specular):BindableResource(name){
    internalInit(diffuse, normal, glow, specular);
}
Material::~Material(){
    SAFE_DELETE_VECTOR(m_Components);
}
void Material::internalInit(Texture* diffuse, Texture* normal, Texture* glow, Texture* specular) {
    m_Components.reserve(MAX_MATERIAL_COMPONENTS);
    if(diffuse)   internalAddComponentGeneric(MaterialComponentType::Diffuse, diffuse);
    if(normal)    internalAddComponentGeneric(MaterialComponentType::Normal, normal);
    if(glow)      internalAddComponentGeneric(MaterialComponentType::Glow, glow);
    if(specular)  internalAddComponentGeneric(MaterialComponentType::Specular, specular);

    m_SpecularModel = SpecularModel::GGX;
    m_DiffuseModel  = DiffuseModel::Lambert;
    m_Shadeless     = false;
    m_BaseGlow      = 0.0f;

    internalUpdateGlobalMaterialPool(true);

    setSmoothness(0.25f);
    setAO(1.0f);
    setMetalness(0.0f);
    setF0Color(0.04f, 0.04f, 0.04f);

    load();
    setCustomBindFunctor(epriv::DefaultMaterialBindFunctor());
    setCustomUnbindFunctor(epriv::DefaultMaterialUnbindFunctor());
}
MaterialComponent* Material::internalAddComponentGeneric(const MaterialComponentType::Type& type, Texture* texture, Texture* mask, Texture* cubemap) {
    MaterialComponent* newMaterialComponent = new MaterialComponent(type, texture, mask, cubemap);
    if (texture) {
        texture->setAnisotropicFiltering(2.0f);
    }
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
    data->g = m_BaseSmoothness;
    data->b = float(m_SpecularModel);
    data->a = float(m_DiffuseModel);
    if (addToDatabase) {
        Material::m_MaterialProperities.push_back(std::move(*data));
        SAFE_DELETE(data);
    }
}


MaterialComponent& Material::addComponent(const MaterialComponentType::Type& type, const string& textureFile, const string& maskFile, const string& cubemapFile) {
    Texture* texture = Core::m_Engine->m_ResourceManager._hasTexture(textureFile);
    Texture* mask = Core::m_Engine->m_ResourceManager._hasTexture(maskFile);
    Texture* cubemap = Core::m_Engine->m_ResourceManager._hasTexture(cubemapFile);
    if (!texture) {
        if (type == MaterialComponentType::Normal || type == MaterialComponentType::ParallaxOcclusion) {
            texture = new Texture(textureFile, true, ImageInternalFormat::RGB8);
        }else{
            texture = new Texture(textureFile, true, ImageInternalFormat::SRGB8_ALPHA8);
        }
        Core::m_Engine->m_ResourceManager._addTexture(texture);
    }
    if (!mask) {
        mask = new Texture(textureFile, false, ImageInternalFormat::R8);
        Core::m_Engine->m_ResourceManager._addTexture(mask);
    }
    if (!cubemap) {
        cubemap = new Texture(textureFile, false, ImageInternalFormat::SRGB8_ALPHA8, GL_TEXTURE_CUBE_MAP);
        Core::m_Engine->m_ResourceManager._addTexture(cubemap);
    }
    return *internalAddComponentGeneric(type, texture, mask, cubemap);
}
MaterialComponent& Material::addComponentDiffuse(const string& textureFile){
    Texture* texture = Core::m_Engine->m_ResourceManager._hasTexture(textureFile);
    if(!texture){
        texture = new Texture(textureFile, true, ImageInternalFormat::SRGB8_ALPHA8);
        Core::m_Engine->m_ResourceManager._addTexture(texture);
    }
    auto& component = *internalAddComponentGeneric(MaterialComponentType::Diffuse, texture);
    return component;
}
MaterialComponent& Material::addComponentNormal(const string& textureFile){
    Texture* texture = Core::m_Engine->m_ResourceManager._hasTexture(textureFile);
    if(!texture){
        texture = new Texture(textureFile, false, ImageInternalFormat::RGBA8);
        Core::m_Engine->m_ResourceManager._addTexture(texture);
    }
    auto& component = *internalAddComponentGeneric(MaterialComponentType::Normal, texture);
    return component;
}
MaterialComponent& Material::addComponentGlow(const string& textureFile){
    Texture* texture = Core::m_Engine->m_ResourceManager._hasTexture(textureFile);
    if(!texture){
        texture = new Texture(textureFile, false, ImageInternalFormat::R8);
        Core::m_Engine->m_ResourceManager._addTexture(texture);
    }
    auto& component = *internalAddComponentGeneric(MaterialComponentType::Glow, texture);
    return component;
}
MaterialComponent& Material::addComponentSpecular(const string& textureFile){
    Texture* texture = Core::m_Engine->m_ResourceManager._hasTexture(textureFile);
    if(!texture){
        texture = new Texture(textureFile,false,ImageInternalFormat::R8);
        Core::m_Engine->m_ResourceManager._addTexture(texture);
    }
    auto& component = *internalAddComponentGeneric(MaterialComponentType::Specular, texture);
    return component;
}
MaterialComponent& Material::addComponentAO(const string& textureFile,float baseValue){
    Texture* texture = Core::m_Engine->m_ResourceManager._hasTexture(textureFile);
    if(!texture){
        texture = new Texture(textureFile, false, ImageInternalFormat::R8);
        Core::m_Engine->m_ResourceManager._addTexture(texture);
    }
    auto& component = *internalAddComponentGeneric(MaterialComponentType::AO, texture);
    auto& layer = component.layer(0);
    auto& _data2 = layer.data2();
    layer.setData2(baseValue, _data2.y, _data2.z, _data2.w);
    return component;
}
MaterialComponent& Material::addComponentMetalness(const string& textureFile,float baseValue){
    Texture* texture = Core::m_Engine->m_ResourceManager._hasTexture(textureFile);
    if(!texture){
        texture = new Texture(textureFile, false, ImageInternalFormat::R8);
        Core::m_Engine->m_ResourceManager._addTexture(texture);
    }
    auto& component = *internalAddComponentGeneric(MaterialComponentType::Metalness, texture);
    auto& layer = component.layer(0);
    auto& _data2 = layer.data2();
    layer.setData2(baseValue, _data2.y, _data2.z, _data2.w);
    return component;
}
MaterialComponent& Material::addComponentSmoothness(const string& textureFile,float baseValue){
    Texture* texture = Core::m_Engine->m_ResourceManager._hasTexture(textureFile);
    if(!texture){
        texture = new Texture(textureFile, false, ImageInternalFormat::R8);
        Core::m_Engine->m_ResourceManager._addTexture(texture);
    }
    auto& component = *internalAddComponentGeneric(MaterialComponentType::Smoothness, texture);
    auto& layer = component.layer(0);
    auto& _data2 = layer.data2();
    layer.setData2(baseValue, _data2.y, _data2.z, _data2.w);
    return component;
}
MaterialComponent& Material::addComponentReflection(const string& cubemapName, const string& maskFile,float mixFactor){
    //add checks to see if texture was loaded already
    Texture* cubemap = Core::m_Engine->m_ResourceManager._hasTexture(cubemapName);
    if(!cubemap){
        cubemap = new Texture(cubemapName, false, ImageInternalFormat::SRGB8_ALPHA8, GL_TEXTURE_CUBE_MAP);
        Core::m_Engine->m_ResourceManager._addTexture(cubemap);
    }
    Texture* mask = Core::m_Engine->m_ResourceManager._hasTexture(maskFile);
    if(!mask){
        mask = new Texture(maskFile, false, ImageInternalFormat::R8);
        Core::m_Engine->m_ResourceManager._addTexture(mask);
    }
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
    Texture* cubemap = Core::m_Engine->m_ResourceManager._hasTexture(cubemapName);
    if(!cubemap){
        cubemap = new Texture(cubemapName, false, ImageInternalFormat::SRGB8_ALPHA8, GL_TEXTURE_CUBE_MAP);
        Core::m_Engine->m_ResourceManager._addTexture(cubemap);
    }
    Texture* mask = Core::m_Engine->m_ResourceManager._hasTexture(maskFile);
    if(!mask){
        mask = new Texture(maskFile, false, ImageInternalFormat::R8);
        Core::m_Engine->m_ResourceManager._addTexture(mask);
    }
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
    Texture* texture = Core::m_Engine->m_ResourceManager._hasTexture(textureFile);
    if(!texture){
        texture = new Texture(textureFile, false, ImageInternalFormat::RGBA8); //its a normal map file
        Core::m_Engine->m_ResourceManager._addTexture(texture);
    }
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
    m_F0Color.r = glm::clamp(r, 0.001f, 0.999f);
    m_F0Color.g = glm::clamp(g, 0.001f, 0.999f);
    m_F0Color.b = glm::clamp(b, 0.001f, 0.999f);
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
void Material::bind(){ 
    Core::m_Engine->m_RenderManager._bindMaterial(this); 
}
void Material::unbind(){ 
    Core::m_Engine->m_RenderManager._unbindMaterial(); 
}
void Material::load(){
    if(!isLoaded()){
        for (auto& component : m_Components) {
            if (component) {
                Texture& texture = *component->texture();
                texture.incrementUseCount();
                if (!texture.isLoaded() && texture.useCount() > 0) {
                    texture.load();
                }
            }
        }
        cout << "(Material) ";
        EngineResource::load();
    }
}
void Material::unload(){
    if(isLoaded() && useCount() == 0){
        for (auto& component : m_Components) {
            if (component) {
                Texture& texture = *component->texture();
                texture.decrementUseCount();
                if (texture.useCount() == 0 && texture.isLoaded()) {
                    texture.unload();
                }
            }
        }
        cout << "(Material) ";
        EngineResource::unload();
    }
}
void Material::update(const double& dt) {
    for (unsigned int i = 0; i < m_Components.size(); ++i) {
        MaterialComponent* component = m_Components[i];
        if (!component)
            break; //this assumes there will never be a null component before defined components. be careful here
        component->update(dt);
    }
}
#pragma endregion