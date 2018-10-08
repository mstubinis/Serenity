#include "core/Material.h"
#include "core/engine/Engine.h"
#include "core/Mesh.h"
#include "core/Texture.h"
#include "core/engine/Engine_Math.h"
#include "core/ShaderProgram.h"
#include "core/Scene.h"
#include "core/Skybox.h"

#include <algorithm>
#include <iostream>
#include <boost/tuple/tuple.hpp>

using namespace Engine;
using namespace Engine::Renderer;
using namespace std;

vector<glm::vec4> Material::m_MaterialProperities;

namespace Engine{
    namespace epriv{
        class MaterialComponentTextureSlot{public: enum Slot{
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
        _TOTAL};};
        vector<vector<uint>> MATERIAL_TEXTURE_SLOTS_MAP = [](){
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
    };
};

class Material::impl final{
    public:
        vector<MaterialComponent*> m_Components;
        uint m_DiffuseModel, m_SpecularModel;
        bool m_Shadeless;
        glm::vec3 m_F0Color;
        float m_BaseGlow, m_BaseAO, m_BaseMetalness, m_BaseSmoothness;
        uint m_ID;

        void _init(string& name,Texture* diffuse,Texture* normal,Texture* glow,Texture* specular,Material& super){
            m_Components.resize(MaterialComponentType::_TOTAL,nullptr);

            _addComponentDiffuse(diffuse);
            _addComponentNormal(normal);
            _addComponentGlow(glow);
            _addComponentSpecular(specular);
            
            m_SpecularModel = SpecularModel::GGX;
            m_DiffuseModel = DiffuseModel::Lambert;

            _updateGlobalMaterialPool(true);

            _setF0Color(0.04f,0.04f,0.04f);
            _setSmoothness(0.9f);
            _setAO(1.0f);
            _setMetalness(0.0f);
            
            m_Shadeless = false;
            m_BaseGlow = 0.0f;

            super.load();
        }
        void _init(string& name,string& diffuseFile,string& normalFile,string& glowFile,string& specularFile,Material& super){
            Texture* d = 0; Texture* n = 0; Texture* g = 0; Texture* s = 0;
            if(diffuseFile != ""){
                d = epriv::Core::m_Engine->m_ResourceManager._hasTexture(diffuseFile);
                if(!d){
                    d = new Texture(diffuseFile);
                    epriv::Core::m_Engine->m_ResourceManager._addTexture(d);
                }
            }
            if(normalFile != ""){
                n = epriv::Core::m_Engine->m_ResourceManager._hasTexture(normalFile);
                if(!n){
                    n = new Texture(normalFile,false,ImageInternalFormat::RGBA8);
                    epriv::Core::m_Engine->m_ResourceManager._addTexture(n);
                }
            }
            if(glowFile != ""){
                g = epriv::Core::m_Engine->m_ResourceManager._hasTexture(glowFile);
                if(!g){
                    g = new Texture(glowFile,false,ImageInternalFormat::RGBA8);
                    epriv::Core::m_Engine->m_ResourceManager._addTexture(g);
                }
            }
            if(specularFile != ""){
                s = epriv::Core::m_Engine->m_ResourceManager._hasTexture(specularFile);
                if(!s){
                    s = new Texture(specularFile,false,ImageInternalFormat::RGBA8);
                    epriv::Core::m_Engine->m_ResourceManager._addTexture(s);
                }
            }
            _init(name,d,n,g,s,super);
        }
        void _load(){
            for(auto component:m_Components){
                if(component){
                    Texture& texture = *component->texture();
                    texture.incrementUseCount();
                    if(!texture.isLoaded() && texture.useCount() > 0){
                        texture.load();
                    }
                }
            }
        }
        void _unload(){
            for(auto component:m_Components){
                if(component){
                    Texture& texture = *component->texture();
                    texture.decrementUseCount();
                    if(texture.useCount() == 0 && texture.isLoaded()){
                        texture.unload();
                    }
                }
            }
        }
        void _updateGlobalMaterialPool(bool add){
            glm::vec4* data = nullptr;
            if(!add){
                data = &Material::m_MaterialProperities[m_ID];
            }else{
                m_ID = Material::m_MaterialProperities.size();
                data = new glm::vec4(0.0f);
            }
            data->r = Math::pack3FloatsInto1FloatUnsigned(m_F0Color.r,m_F0Color.g,m_F0Color.b);
            data->g = m_BaseSmoothness;
            data->b = float(m_SpecularModel);
            data->a = float(m_DiffuseModel);
            if(add){
                glm::vec4 dataCopy = glm::vec4(*data);
                Material::m_MaterialProperities.push_back(dataCopy);
                delete data;
            }
        }
        void _destruct(){
            SAFE_DELETE_VECTOR(m_Components);
        }
        void _addComponentGeneric(Texture* texture,MaterialComponentType::Type type){
            if(m_Components[type] || !texture)
                return;
            m_Components[type] = new MaterialComponent(type,texture);
        }
        void _addComponentDiffuse(Texture* texture){ _addComponentGeneric(texture,MaterialComponentType::Diffuse); }
        void _addComponentNormal(Texture* texture){ _addComponentGeneric(texture,MaterialComponentType::Normal); }
        void _addComponentGlow(Texture* texture){ _addComponentGeneric(texture,MaterialComponentType::Glow); }
        void _addComponentSpecular(Texture* texture){ _addComponentGeneric(texture,MaterialComponentType::Specular); }
        void _addComponentAO(Texture* texture){ _addComponentGeneric(texture,MaterialComponentType::AO); }
        void _addComponentMetalness(Texture* texture){ _addComponentGeneric(texture,MaterialComponentType::Metalness); }
        void _addComponentSmoothness(Texture* texture){ _addComponentGeneric(texture,MaterialComponentType::Smoothness); }
        void _addComponentReflection(Texture* texture,Texture* map,float& mixFactor){
            uint type = MaterialComponentType::Reflection;
            if(m_Components[type] || (!texture || !map))
                return;
            m_Components[type] = new MaterialComponentReflection(type,texture,map,mixFactor);
        }
        void _addComponentRefraction(Texture* texture,Texture* map,float& refractiveIndex,float& mixFactor){
            uint type = MaterialComponentType::Refraction;
            if(m_Components[type] || (!texture || !map))
                return;
            m_Components[type] = new MaterialComponentRefraction(texture,map,refractiveIndex,mixFactor);
        }
        void _addComponentParallaxOcclusion(Texture* texture,float& heightScale){
            uint type = MaterialComponentType::ParallaxOcclusion;
            if(m_Components[type] || !texture)
                return;
            m_Components[type] = new MaterialComponentParallaxOcclusion(texture,heightScale);
        }
        void _setF0Color(float r,float g,float b){
            m_F0Color.r = glm::clamp(r,0.001f,0.999f); 
            m_F0Color.g = glm::clamp(g,0.001f,0.999f); 
            m_F0Color.b = glm::clamp(b,0.001f,0.999f);
            _updateGlobalMaterialPool(false);
        }
        void _setMaterialProperties(float& r,float& g,float& b,float& smoothness,float& metalness){
            _setF0Color(r,g,b);
            _setSmoothness(smoothness);
            _setMetalness(metalness);
            _updateGlobalMaterialPool(false);
        }
        void _setShadeless(bool b){                      m_Shadeless = b;      _updateGlobalMaterialPool(false); }
        void _setBaseGlow(float f){                      m_BaseGlow = f;       _updateGlobalMaterialPool(false); }
        void _setSmoothness(float s){                    m_BaseSmoothness = glm::clamp(s,0.001f,0.999f); _updateGlobalMaterialPool(false); }
        void _setSpecularModel(SpecularModel::Model& m){ m_SpecularModel = m;  _updateGlobalMaterialPool(false); }
        void _setDiffuseModel(DiffuseModel::Model& m){   m_DiffuseModel = m;   _updateGlobalMaterialPool(false); }
        void _setAO(float a){                            m_BaseAO = glm::clamp(a,0.001f,0.999f);         _updateGlobalMaterialPool(false); }
        void _setMetalness(float m){                     m_BaseMetalness = glm::clamp(m,0.001f,0.999f);  _updateGlobalMaterialPool(false); }
};

namespace Engine{
    namespace epriv{
        struct DefaultMaterialBindFunctor{void operator()(BindableResource* r) const {
            Material::impl& material = *((Material*)r)->m_i;
            glm::vec4 first(0.0f); glm::vec4 second(0.0f); glm::vec4 third(0.0f);
            for(uint i = 0; i < MaterialComponentType::_TOTAL; ++i){
                if(material.m_Components[i]){
                    MaterialComponent& component = *material.m_Components[i];
                    if(component.texture() && component.texture()->address() != 0){
                        //enable
                        if     (i == 0) { first.x = 1.0f; }
                        else if(i == 1) { first.y = component.texture()->compressed() ? 0.5f : 1.0f; }
                        else if(i == 2) { first.z = 1.0f; }
                        else if(i == 3) { first.w = 1.0f; }
                        else if(i == 4) { second.x = 1.0f; }
                        else if(i == 5) { second.y = 1.0f; }
                        else if(i == 6) { second.z = 1.0f; }
                        else if(i == 7) { second.w = 1.0f; }
                        else if(i == 8) { third.x = 1.0f; }
                        else if(i == 9) { third.y = 1.0f; }
                        else if(i == 10){ third.z = 1.0f; }
                        else if(i == 11){ third.w = 1.0f; }
                        component.bind();
                    }else{ 
                        component.unbind(); 
                    }
                }
            }
            sendUniform1Safe("Shadeless",int(material.m_Shadeless));
            sendUniform4Safe("Material_F0AndID",glm::vec4(material.m_F0Color,float(material.m_ID)));
            sendUniform4Safe("MaterialBasePropertiesOne",material.m_BaseGlow,material.m_BaseAO,material.m_BaseMetalness,material.m_BaseSmoothness);
            sendUniform4Safe("FirstConditionals", first);
            sendUniform4Safe("SecondConditionals",second);
            sendUniform4Safe("ThirdConditionals",third);
        }};
        struct DefaultMaterialUnbindFunctor{void operator()(BindableResource* r) const {
            //Material& material = *(Material*)r;
        }};
        DefaultMaterialBindFunctor DEFAULT_BIND_FUNCTOR;
        DefaultMaterialUnbindFunctor DEFAULT_UNBIND_FUNCTOR;
    };
};






#pragma region MaterialComponents

MaterialComponent::MaterialComponent(uint type,Texture* t){
    m_ComponentType = (MaterialComponentType::Type)type;
    m_Texture = t;
}
MaterialComponent::~MaterialComponent(){
}
void MaterialComponent::bind(){
    vector<uint>& slots = epriv::MATERIAL_TEXTURE_SLOTS_MAP[m_ComponentType];
    string textureTypeName = epriv::MATERIAL_COMPONENT_SHADER_TEXTURE_NAMES[m_ComponentType];
    for(uint i = 0; i < slots.size(); ++i){
        sendTextureSafe(textureTypeName.c_str(),*m_Texture,slots[i]);
    }
}
void MaterialComponent::unbind(){
    //vector<uint>& slots = epriv::MATERIAL_TEXTURE_SLOTS_MAP[m_ComponentType];
    //for(uint i = 0; i < slots.size(); ++i){ unbindTexture2D(slots[i]); }
}
MaterialComponentReflection::MaterialComponentReflection(uint type,Texture* cubemap,Texture* map,float mixFactor):MaterialComponent(type,cubemap){
    setMixFactor(mixFactor);
    m_Map = map;
}
MaterialComponentReflection::~MaterialComponentReflection(){
    MaterialComponent::~MaterialComponent();
}
void MaterialComponentReflection::setMixFactor(float factor){
    m_MixFactor = glm::clamp(factor,0.0f,1.0f);
}
void MaterialComponentReflection::bind(){
    vector<uint>& slots = epriv::MATERIAL_TEXTURE_SLOTS_MAP[m_ComponentType];
    string textureTypeName = epriv::MATERIAL_COMPONENT_SHADER_TEXTURE_NAMES[m_ComponentType];
    sendUniform1Safe("CubemapMixFactor",m_MixFactor);
    if(!m_Texture)
        sendTextureSafe(textureTypeName.c_str(),*Resources::getCurrentScene()->skybox()->texture(),slots[0]);
    else
        sendTextureSafe(textureTypeName.c_str(),*m_Texture,slots[0]);
    sendTextureSafe((textureTypeName+"Map").c_str(),*m_Map,slots[1]);
}
void MaterialComponentReflection::unbind(){
    //vector<uint>& slots = epriv::MATERIAL_TEXTURE_SLOTS_MAP[m_ComponentType];
    //unbindTexture2D(slots[0]);
    //unbindTextureCubemap(slots[1]);
}
MaterialComponentRefraction::MaterialComponentRefraction(Texture* cubemap,Texture* map,float i,float mix):MaterialComponentReflection(MaterialComponentType::Refraction,cubemap,map,mix){
    m_RefractionIndex = i;
}
MaterialComponentRefraction::~MaterialComponentRefraction(){
    MaterialComponentReflection::~MaterialComponentReflection();
}
void MaterialComponentRefraction::setRefractionIndex(float _index){ m_RefractionIndex = _index; }
void MaterialComponentRefraction::bind(){
    vector<uint>& slots = epriv::MATERIAL_TEXTURE_SLOTS_MAP[m_ComponentType];
    string textureTypeName = epriv::MATERIAL_COMPONENT_SHADER_TEXTURE_NAMES[m_ComponentType];
    sendUniform1Safe("CubemapMixFactor",m_MixFactor);
    sendUniform1Safe("RefractionIndex",m_RefractionIndex);
    if(!m_Texture)
        sendTextureSafe(textureTypeName.c_str(),*Resources::getCurrentScene()->skybox()->texture(),slots[0]);
    else
        sendTextureSafe(textureTypeName.c_str(),*m_Texture,slots[0]);
    sendTextureSafe((textureTypeName+"Map").c_str(),*m_Map,slots[1]);
}

MaterialComponentParallaxOcclusion::MaterialComponentParallaxOcclusion(Texture* map,float heightScale):MaterialComponent(MaterialComponentType::ParallaxOcclusion,map){
    setHeightScale(heightScale);
}
MaterialComponentParallaxOcclusion::~MaterialComponentParallaxOcclusion(){
    MaterialComponent::~MaterialComponent();
}
void MaterialComponentParallaxOcclusion::setHeightScale(float factor){
    m_HeightScale = factor;
}
void MaterialComponentParallaxOcclusion::bind(){
    vector<uint>& slots = epriv::MATERIAL_TEXTURE_SLOTS_MAP[m_ComponentType];
    string textureTypeName = epriv::MATERIAL_COMPONENT_SHADER_TEXTURE_NAMES[m_ComponentType];
    sendUniform1Safe("ParallaxHeightScale",m_HeightScale);
    sendTextureSafe(textureTypeName.c_str(),*m_Texture,slots[0]);
}
void MaterialComponentParallaxOcclusion::unbind(){
    //vector<uint>& slots = epriv::MATERIAL_TEXTURE_SLOTS_MAP[m_ComponentType];
    //unbindTexture2D(slots[0]);
}

#pragma endregion


#pragma region Material

Material::Material(string name,string diffuse,string normal,string glow,string specular):m_i(new impl),BindableResource(name){
    m_i->_init(name,diffuse,normal,glow,specular,*this);
    setCustomBindFunctor(epriv::DEFAULT_BIND_FUNCTOR);
    setCustomUnbindFunctor(epriv::DEFAULT_UNBIND_FUNCTOR);
}
Material::Material(string name,Texture* diffuse,Texture* normal,Texture* glow,Texture* specular):m_i(new impl),BindableResource(name){
    m_i->_init(name,diffuse,normal,glow,specular,*this);
    setCustomBindFunctor(epriv::DEFAULT_BIND_FUNCTOR);
    setCustomUnbindFunctor(epriv::DEFAULT_UNBIND_FUNCTOR);
}
Material::~Material(){
    m_i->_destruct();
}
void Material::addComponentDiffuse(Texture* texture){
    m_i->_addComponentDiffuse(texture);
}
void Material::addComponentDiffuse(string textureFile){
    Texture* texture = epriv::Core::m_Engine->m_ResourceManager._hasTexture(textureFile);
    if(!texture){
        texture = new Texture(textureFile,true,ImageInternalFormat::SRGB8_ALPHA8);
        epriv::Core::m_Engine->m_ResourceManager._addTexture(texture);
    }
    m_i->_addComponentDiffuse(texture);
}
void Material::addComponentNormal(Texture* texture){
    m_i->_addComponentNormal(texture);
}
void Material::addComponentNormal(string textureFile){
    Texture* texture = epriv::Core::m_Engine->m_ResourceManager._hasTexture(textureFile);
    if(!texture){
        texture = new Texture(textureFile,false,ImageInternalFormat::RGBA8);
        epriv::Core::m_Engine->m_ResourceManager._addTexture(texture);
    }
    m_i->_addComponentNormal(texture);
}
void Material::addComponentGlow(Texture* texture){
    m_i->_addComponentGlow(texture);
}
void Material::addComponentGlow(string textureFile){
    Texture* texture = epriv::Core::m_Engine->m_ResourceManager._hasTexture(textureFile);
    if(!texture){
        texture = new Texture(textureFile,false,ImageInternalFormat::RGBA8);
        epriv::Core::m_Engine->m_ResourceManager._addTexture(texture);
    }
    m_i->_addComponentGlow(texture);
}
void Material::addComponentSpecular(Texture* texture){
    m_i->_addComponentSpecular(texture);
}
void Material::addComponentSpecular(string textureFile){
    Texture* texture = epriv::Core::m_Engine->m_ResourceManager._hasTexture(textureFile);
    if(!texture){
        texture = new Texture(textureFile,false,ImageInternalFormat::RGBA8);
        epriv::Core::m_Engine->m_ResourceManager._addTexture(texture);
    }
    m_i->_addComponentSpecular(texture);
}
void Material::addComponentAO(Texture* texture,float baseValue){
    m_i->_addComponentAO(texture);
    setAO(baseValue);
}
void Material::addComponentAO(string textureFile,float baseValue){
    Texture* texture = epriv::Core::m_Engine->m_ResourceManager._hasTexture(textureFile);
    if(!texture){
        texture = new Texture(textureFile,false,ImageInternalFormat::RGBA8);
        epriv::Core::m_Engine->m_ResourceManager._addTexture(texture);
    }
    m_i->_addComponentAO(texture);
    setAO(baseValue);
}
void Material::addComponentMetalness(Texture* texture,float baseValue){
    m_i->_addComponentMetalness(texture);
    setMetalness(baseValue);
}
void Material::addComponentMetalness(string textureFile,float baseValue){
    Texture* texture = epriv::Core::m_Engine->m_ResourceManager._hasTexture(textureFile);
    if(!texture){
        texture = new Texture(textureFile,false,ImageInternalFormat::RGBA8);
        epriv::Core::m_Engine->m_ResourceManager._addTexture(texture);
    }
    m_i->_addComponentMetalness(texture);
    setMetalness(baseValue);
}
void Material::addComponentSmoothness(Texture* texture,float baseValue){
    m_i->_addComponentSmoothness(texture);
    setSmoothness(baseValue);
}
void Material::addComponentSmoothness(string textureFile,float baseValue){
    Texture* texture = epriv::Core::m_Engine->m_ResourceManager._hasTexture(textureFile);
    if(!texture){
        texture = new Texture(textureFile,false,ImageInternalFormat::RGBA8);
        epriv::Core::m_Engine->m_ResourceManager._addTexture(texture);
    }
    m_i->_addComponentSmoothness(texture);
    setSmoothness(baseValue);
}
void Material::addComponentReflection(Texture* cubemap,Texture* map,float mixFactor){
    if(cubemap == nullptr) cubemap = Resources::getCurrentScene()->skybox()->texture();
    m_i->_addComponentReflection(cubemap,map,mixFactor);
}
void Material::addComponentReflection(string textureFiles[],string mapFile,float mixFactor){
    //add checks to see if texture was loaded already
    Texture* cubemap;// = epriv::Core::m_Engine->m_ResourceManager->_hasTexture(textureFile);
    //if(!cubemap){
        cubemap = new Texture(textureFiles,"Cubemap ");
        epriv::Core::m_Engine->m_ResourceManager._addTexture(cubemap);
    //}
    Texture* map = epriv::Core::m_Engine->m_ResourceManager._hasTexture(mapFile);
    if(!map){
        map = new Texture(mapFile);
        epriv::Core::m_Engine->m_ResourceManager._addTexture(map);
    }
    Material::addComponentReflection(cubemap,map,mixFactor);
}
void Material::addComponentReflection(string cubemapName,string mapFile,float mixFactor){
    //add checks to see if texture was loaded already
    Texture* cubemap;// = epriv::Core::m_Engine->m_ResourceManager->_hasTexture(textureFile);
    //if(!cubemap){
        cubemap = new Texture(cubemapName);
        epriv::Core::m_Engine->m_ResourceManager._addTexture(cubemap);
    //}
    Texture* map = epriv::Core::m_Engine->m_ResourceManager._hasTexture(mapFile);
    if(!map){
        map = new Texture(mapFile);
        epriv::Core::m_Engine->m_ResourceManager._addTexture(map);
    }
    Material::addComponentReflection(cubemap,map,mixFactor);
}
void Material::addComponentRefraction(Texture* cubemap,Texture* map,float refractiveIndex,float mixFactor){
    m_i->_addComponentRefraction(cubemap,map,refractiveIndex,mixFactor);
}
void Material::addComponentRefraction(string textureFiles[],string mapFile,float refractiveIndex,float mixFactor){
    //add checks to see if texture was loaded already
    Texture* cubemap;// = epriv::Core::m_Engine->m_ResourceManager._hasTexture(textureFile);
    //if(!cubemap){
        cubemap = new Texture(textureFiles,"Cubemap ");
        epriv::Core::m_Engine->m_ResourceManager._addTexture(cubemap);
    //}
    Texture* map = epriv::Core::m_Engine->m_ResourceManager._hasTexture(mapFile);
    if(!map){
        map = new Texture(mapFile);
        epriv::Core::m_Engine->m_ResourceManager._addTexture(map);
    }
    Material::addComponentRefraction(cubemap,map,refractiveIndex,mixFactor);
}
void Material::addComponentRefraction(string cubemapName,string mapFile,float refractiveIndex,float mixFactor){
    //add checks to see if texture was loaded already
    Texture* cubemap;// = epriv::Core::m_Engine->m_ResourceManager._hasTexture(textureFile);
    //if(!cubemap){
        cubemap = new Texture(cubemapName);
        epriv::Core::m_Engine->m_ResourceManager._addTexture(cubemap);
    //}
    Texture* map = epriv::Core::m_Engine->m_ResourceManager._hasTexture(mapFile);
    if(!map){
        map = new Texture(mapFile);
        epriv::Core::m_Engine->m_ResourceManager._addTexture(map);
    }
    Material::addComponentRefraction(cubemap,map,refractiveIndex,mixFactor);
}
void Material::addComponentParallaxOcclusion(Texture* texture,float heightScale){
    m_i->_addComponentParallaxOcclusion(texture,heightScale);
}
void Material::addComponentParallaxOcclusion(std::string textureFile,float heightScale){
    Texture* texture = epriv::Core::m_Engine->m_ResourceManager._hasTexture(textureFile);
    if(!texture){
        texture = new Texture(textureFile,false,ImageInternalFormat::RGBA8);
        epriv::Core::m_Engine->m_ResourceManager._addTexture(texture);
    }
    m_i->_addComponentParallaxOcclusion(texture,heightScale);
}
const MaterialComponent* Material::getComponent(MaterialComponentType::Type type) const { return m_i->m_Components[type]; }
const MaterialComponentReflection* Material::getComponentReflection() const { return (MaterialComponentReflection*)(m_i->m_Components[MaterialComponentType::Reflection]); }
const MaterialComponentRefraction* Material::getComponentRefraction() const { return (MaterialComponentRefraction*)(m_i->m_Components[MaterialComponentType::Refraction]); }
const MaterialComponentParallaxOcclusion* Material::getComponentParallaxOcclusion() const { return (MaterialComponentParallaxOcclusion*)(m_i->m_Components[MaterialComponentType::ParallaxOcclusion]); }
const bool Material::shadeless() const { return m_i->m_Shadeless; }
const glm::vec3 Material::f0() const{ return m_i->m_F0Color; }
const float Material::glow() const { return m_i->m_BaseGlow; }
const uint Material::id() const { return m_i->m_ID; }
const uint Material::diffuseModel() const { return m_i->m_DiffuseModel; }
const uint Material::specularModel() const { return m_i->m_SpecularModel; }
const float Material::ao() const { return m_i->m_BaseAO; }
const float Material::metalness() const{ return m_i->m_BaseMetalness; }
const float Material::smoothness() const { return m_i->m_BaseSmoothness; }
void Material::setShadeless(bool b){ m_i->_setShadeless(b); }
void Material::setGlow(float f){ m_i->_setBaseGlow(f); }
void Material::setF0Color(glm::vec3 color){ Material::setF0Color(color.r, color.g, color.b); }
void Material::setF0Color(float r, float g, float b){ m_i->_setF0Color(r, g, b); }
void Material::setMaterialPhysics(MaterialPhysics::Physics c){
    auto& t = epriv::MATERIAL_PROPERTIES[c];
    m_i->_setMaterialProperties(t.get<0>(),t.get<1>(),t.get<2>(),t.get<3>(),t.get<4>() );
}
void Material::setSmoothness(float s){ m_i->_setSmoothness(s); }
void Material::setSpecularModel(SpecularModel::Model m){ m_i->_setSpecularModel(m); }
void Material::setDiffuseModel(DiffuseModel::Model m){ m_i->_setDiffuseModel(m); }
void Material::setAO(float a){ m_i->_setAO(a); }
void Material::setMetalness(float m){ m_i->_setMetalness(m); }
void Material::bind(){ epriv::Core::m_Engine->m_RenderManager._bindMaterial(this); }
void Material::unbind(){ epriv::Core::m_Engine->m_RenderManager._unbindMaterial(); }
void Material::load(){
    if(!isLoaded()){
        m_i->_load();
        std::cout << "(Material) ";
        EngineResource::load();
    }
}
void Material::unload(){
    if(isLoaded() && useCount() == 0){
        m_i->_unload();
        std::cout << "(Material) ";
        EngineResource::unload();
    }
}

#pragma endregion