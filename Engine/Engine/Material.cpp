#include "ObjectDynamic.h"
#include "ObjectDisplay.h"
#include "Mesh.h"
#include "Material.h"
#include "Texture.h"
#include "Engine_Resources.h"
#include "Engine_Renderer.h"
#include "ShaderProgram.h"
#include "Scene.h"
#include "Skybox.h"

#include <unordered_map>
#include <algorithm>
#include <iostream>

using namespace Engine;

std::vector<glm::vec4> Material::m_MaterialProperities;
struct DefaultMaterialBindFunctor{void operator()(BindableResource* r) const {
    Material* material = static_cast<Material*>(r);
    glm::vec3 first(0);
    glm::vec3 second(0);
    glm::vec3 third(0);
    for(uint i = 0; i < MaterialComponentType::NUMBER; i++){
        if(material->getComponents().count(i)){
            MaterialComponent* component = material->getComponents().at(i);
            if(component->texture() != nullptr && component->texture()->address() != 0){
                //enable
                if     (i == 0){ first.x = 1.0f; }
                else if(i == 1){ first.y = 1.0f; }
                else if(i == 2){ first.z = 1.0f; }
                else if(i == 3){ second.x = 1.0f; }
                else if(i == 4){ second.y = 1.0f; }
                else if(i == 5){ second.z = 1.0f; }
                else if(i == 6){ third.x = 1.0f; }
                else if(i == 7){ third.y = 1.0f; }
                else if(i == 8){ third.z = 1.0f; }
                component->bind();
            }
            else{ 
                //disable
                component->unbind(); 
            }
        }
    }
    Renderer::sendUniform1iSafe("Shadeless",int(material->shadeless()));
    Renderer::sendUniform1fSafe("BaseGlow",material->glow());
    float id = float(material->id()) / (float)MATERIAL_COUNT_LIMIT;
    Renderer::sendUniform1fSafe("matID",id);
    Renderer::sendUniform3fSafe("FirstConditionals", first.x,first.y,first.z);
    Renderer::sendUniform3fSafe("SecondConditionals",second.x,second.y,second.z);
    Renderer::sendUniform3fSafe("ThirdConditionals",third.x,third.y,third.z);
}};
struct DefaultMaterialUnbindFunctor{void operator()(BindableResource* r) const {
    //Material* m = static_cast<Material*>(r);
}};


std::unordered_map<uint,std::vector<uint>> _populateTextureSlotMap(){
    std::unordered_map<uint,std::vector<uint>> texture_slot_map;

    texture_slot_map[MaterialComponentType::DIFFUSE].push_back(MaterialComponentTextureSlot::DIFFUSE);
    texture_slot_map[MaterialComponentType::NORMAL].push_back(MaterialComponentTextureSlot::NORMAL);
    texture_slot_map[MaterialComponentType::GLOW].push_back(MaterialComponentTextureSlot::GLOW);
    texture_slot_map[MaterialComponentType::SPECULAR].push_back(MaterialComponentTextureSlot::SPECULAR);


    texture_slot_map[MaterialComponentType::AO].push_back(MaterialComponentTextureSlot::AO);
    texture_slot_map[MaterialComponentType::METALNESS].push_back(MaterialComponentTextureSlot::METALNESS);
    texture_slot_map[MaterialComponentType::ROUGHNESS].push_back(MaterialComponentTextureSlot::ROUGHNESS);

    texture_slot_map[MaterialComponentType::REFLECTION].push_back(MaterialComponentTextureSlot::REFLECTION_CUBEMAP);
    texture_slot_map[MaterialComponentType::REFLECTION].push_back(MaterialComponentTextureSlot::REFLECTION_CUBEMAP_MAP);

    texture_slot_map[MaterialComponentType::REFRACTION].push_back(MaterialComponentTextureSlot::REFRACTION_CUBEMAP);
    texture_slot_map[MaterialComponentType::REFRACTION].push_back(MaterialComponentTextureSlot::REFRACTION_CUBEMAP_MAP);

    return texture_slot_map;
}
std::unordered_map<uint,std::vector<uint>> Material::MATERIAL_TEXTURE_SLOTS_MAP = _populateTextureSlotMap();

MaterialComponent::MaterialComponent(uint type,Texture* t){
    m_ComponentType = (MaterialComponentType::Type)type;
    m_Texture = t;
}
MaterialComponent::MaterialComponent(uint type,std::string& t){
    m_ComponentType = (MaterialComponentType::Type)type;
    m_Texture = Resources::getTexture(t); 
    if(m_Texture == nullptr && t != "") m_Texture = new Texture(t);
}
MaterialComponent::~MaterialComponent(){
}
void MaterialComponent::bind(){
    std::vector<uint>& slots = Material::MATERIAL_TEXTURE_SLOTS_MAP[m_ComponentType];
    std::string textureTypeName = MATERIAL_COMPONENT_SHADER_TEXTURE_NAMES[m_ComponentType];
    for(uint i = 0; i < slots.size(); i++){
        Renderer::bindTextureSafe(textureTypeName.c_str(),m_Texture,slots.at(i));
    }
}
void MaterialComponent::unbind(){
    std::vector<uint>& slots = Material::MATERIAL_TEXTURE_SLOTS_MAP[m_ComponentType];
    for(uint i = 0; i < slots.size(); i++){
        Renderer::unbindTexture2D(slots.at(i));
    }
}
MaterialComponentAO::MaterialComponentAO(Texture* map,float aoBaseValue):MaterialComponent(MaterialComponentType::AO,map){
    m_AOBaseValue = aoBaseValue;
}
MaterialComponentAO::MaterialComponentAO(std::string& map,float aoBaseValue):MaterialComponent(MaterialComponentType::AO,map){
    m_AOBaseValue = aoBaseValue;
}
MaterialComponentAO::~MaterialComponentAO(){ MaterialComponent::~MaterialComponent(); }
void MaterialComponentAO::setAOBaseValue(float factor){ m_AOBaseValue = glm::clamp(factor,0.0f,1.0f); }
void MaterialComponentAO::bind(){
    std::vector<uint>& slots = Material::MATERIAL_TEXTURE_SLOTS_MAP[m_ComponentType];
    std::string textureTypeName = MATERIAL_COMPONENT_SHADER_TEXTURE_NAMES[m_ComponentType];

    Renderer::sendUniform1fSafe("AOBaseValue",m_AOBaseValue);
    Renderer::bindTextureSafe(textureTypeName.c_str(),m_Texture,slots.at(0));
}
void MaterialComponentAO::unbind(){
    std::vector<uint>& slots = Material::MATERIAL_TEXTURE_SLOTS_MAP[m_ComponentType];
    Renderer::unbindTexture2D(slots.at(0));
}
MaterialComponentMetalness::MaterialComponentMetalness(Texture* map,float metalnessBaseValue):MaterialComponent(MaterialComponentType::METALNESS,map){
    m_MetalnessBaseValue = metalnessBaseValue;
}
MaterialComponentMetalness::MaterialComponentMetalness(std::string& map,float metalnessBaseValue):MaterialComponent(MaterialComponentType::METALNESS,map){
    m_MetalnessBaseValue = metalnessBaseValue;
}
MaterialComponentMetalness::~MaterialComponentMetalness(){ MaterialComponent::~MaterialComponent(); }
void MaterialComponentMetalness::setMetalnessBaseValue(float factor){ m_MetalnessBaseValue = glm::clamp(factor,0.0f,1.0f); }
void MaterialComponentMetalness::bind(){
    std::vector<uint>& slots = Material::MATERIAL_TEXTURE_SLOTS_MAP[m_ComponentType];
    std::string textureTypeName = MATERIAL_COMPONENT_SHADER_TEXTURE_NAMES[m_ComponentType];

    Renderer::sendUniform1fSafe("MetalnessBaseValue",m_MetalnessBaseValue);
    Renderer::bindTextureSafe(textureTypeName.c_str(),m_Texture,slots.at(0));
}
void MaterialComponentMetalness::unbind(){
    std::vector<uint>& slots = Material::MATERIAL_TEXTURE_SLOTS_MAP[m_ComponentType];
    Renderer::unbindTexture2D(slots.at(0));
}
MaterialComponentRoughness::MaterialComponentRoughness(Texture* map,float roughnessBaseValue):MaterialComponent(MaterialComponentType::ROUGHNESS,map){
    m_RoughnessBaseValue = roughnessBaseValue;
}
MaterialComponentRoughness::MaterialComponentRoughness(std::string& map,float roughnessBaseValue):MaterialComponent(MaterialComponentType::ROUGHNESS,map){
    m_RoughnessBaseValue = roughnessBaseValue;
}
MaterialComponentRoughness::~MaterialComponentRoughness(){ MaterialComponent::~MaterialComponent(); }
void MaterialComponentRoughness::setRoughnessBaseValue(float factor){ m_RoughnessBaseValue = glm::clamp(factor,0.0f,1.0f); }
void MaterialComponentRoughness::bind(){
    std::vector<uint>& slots = Material::MATERIAL_TEXTURE_SLOTS_MAP[m_ComponentType];
    std::string textureTypeName = MATERIAL_COMPONENT_SHADER_TEXTURE_NAMES[m_ComponentType];

    Renderer::sendUniform1fSafe("RoughnessBaseValue",m_RoughnessBaseValue);
    Renderer::bindTextureSafe(textureTypeName.c_str(),m_Texture,slots.at(0));
}
void MaterialComponentRoughness::unbind(){
    std::vector<uint>& slots = Material::MATERIAL_TEXTURE_SLOTS_MAP[m_ComponentType];
    Renderer::unbindTexture2D(slots.at(0));
}
MaterialComponentReflection::MaterialComponentReflection(uint type,Texture* cubemap,Texture* map,float mixFactor):MaterialComponent(type,cubemap){
    m_MixFactor = mixFactor;
    m_Map = map;
}
MaterialComponentReflection::MaterialComponentReflection(uint type,std::string& cubemap,std::string& map,float mixFactor):MaterialComponent(type,cubemap){
    m_MixFactor = mixFactor;
    m_Map = Resources::getTexture(map); 
    if(m_Map == nullptr && map != "") m_Map = new Texture(map);
}
MaterialComponentReflection::~MaterialComponentReflection(){
    MaterialComponent::~MaterialComponent();
}
void MaterialComponentReflection::setMixFactor(float factor){
    m_MixFactor = glm::clamp(factor,0.0f,1.0f);
}
void MaterialComponentReflection::bind(){
    std::vector<uint>& slots = Material::MATERIAL_TEXTURE_SLOTS_MAP[m_ComponentType];
    std::string textureTypeName = MATERIAL_COMPONENT_SHADER_TEXTURE_NAMES[m_ComponentType];

    Renderer::sendUniform1fSafe("CubemapMixFactor",m_MixFactor);
    if(m_Texture == nullptr)
        Renderer::bindTextureSafe(textureTypeName.c_str(),Resources::getCurrentScene()->getSkybox()->texture(),slots.at(0));
    else
        Renderer::bindTextureSafe(textureTypeName.c_str(),m_Texture,slots.at(0));
    Renderer::bindTextureSafe((textureTypeName+"Map").c_str(),m_Map,slots.at(1));
}
void MaterialComponentReflection::unbind(){
    std::vector<uint>& slots = Material::MATERIAL_TEXTURE_SLOTS_MAP[m_ComponentType];

    Renderer::unbindTexture2D(slots.at(0));
    Renderer::unbindTextureCubemap(slots.at(1));
}
MaterialComponentRefraction::MaterialComponentRefraction(Texture* cubemap,Texture* map,float mixFactor,float index):MaterialComponentReflection(MaterialComponentType::REFRACTION,cubemap,map,mixFactor){
    m_RefractionIndex = index;
}
MaterialComponentRefraction::MaterialComponentRefraction(std::string& cubemap,std::string& map,float mixFactor,float index):MaterialComponentReflection(MaterialComponentType::REFRACTION,cubemap,map,mixFactor){
    m_RefractionIndex = index;
}
MaterialComponentRefraction::~MaterialComponentRefraction(){
    MaterialComponentReflection::~MaterialComponentReflection();
}
void MaterialComponentRefraction::bind(){
    std::vector<uint>& slots = Material::MATERIAL_TEXTURE_SLOTS_MAP[m_ComponentType];
    std::string textureTypeName = MATERIAL_COMPONENT_SHADER_TEXTURE_NAMES[m_ComponentType];

    Renderer::sendUniform1fSafe("CubemapMixFactor",m_MixFactor);
    Renderer::sendUniform1fSafe("RefractionIndex",m_RefractionIndex);

    if(m_Texture == nullptr)
        Renderer::bindTextureSafe(textureTypeName.c_str(),Resources::getCurrentScene()->getSkybox()->texture(),slots.at(0));
    else
        Renderer::bindTextureSafe(textureTypeName.c_str(),m_Texture,slots.at(0));
    Renderer::bindTextureSafe((textureTypeName+"Map").c_str(),m_Map,slots.at(1));
}

class Material::impl final{
    public:
        static DefaultMaterialBindFunctor DEFAULT_BIND_FUNCTOR;
        static DefaultMaterialUnbindFunctor DEFAULT_UNBIND_FUNCTOR;

        std::unordered_map<uint,MaterialComponent*> m_Components;
        std::vector<MaterialMeshEntry*> m_Meshes;
        uint m_LightingMode;
        bool m_Shadeless;
        float m_BaseGlow;
        float m_SpecularityPower;
        float m_Frensel;
        uint m_ID;
        void _init(std::string& name,Texture* diffuse,Texture* normal,Texture* glow,Texture* specular,Material* super){
            _addComponentDiffuse(diffuse);
            _addComponentNormal(normal);
            _addComponentGlow(glow);
            _addComponentSpecular(specular);

            m_Shadeless = false;
            m_BaseGlow = 0.0f;
            m_SpecularityPower = 8.0f;
            m_LightingMode = Material::LightingMode::BLINNPHONG;
            m_Frensel = 0.5f;

            _addToMaterialPool();

            super->setCustomBindFunctor(Material::impl::DEFAULT_BIND_FUNCTOR);
            super->setCustomUnbindFunctor(Material::impl::DEFAULT_UNBIND_FUNCTOR);

            super->load();
        }
        void _init(std::string& name, std::string& diffuse, std::string& normal, std::string& glow, std::string& specular,Material* super){
            Texture* diffuseT = Resources::getTexture(diffuse); 
            Texture* normalT = Resources::getTexture(normal); 
            Texture* glowT = Resources::getTexture(glow);
            Texture* specularT = Resources::getTexture(specular);
            if(diffuseT == nullptr && diffuse != "") diffuseT = new Texture(diffuse);
            if(normalT == nullptr && normal != "")  normalT = new Texture(normal,"",GL_TEXTURE_2D,GL_RGBA8);
            if(glowT == nullptr && glow != "")    glowT = new Texture(glow,"",GL_TEXTURE_2D,GL_RGBA8);
            if(specularT == nullptr && specular != "")    specularT = new Texture(specular,"",GL_TEXTURE_2D,GL_RGBA8);
            _init(name,diffuseT,normalT,glowT,specularT,super);
        }
        void _load(){
            for(auto component:m_Components){
                if(component.second != nullptr){
                    component.second->texture()->incrementUseCount();
                    component.second->texture()->load();
                }
            }
        }
        void _unload(){
            for(auto component:m_Components){
                if(component.second != nullptr){
                    component.second->texture()->decrementUseCount();
                    if(component.second->texture()->useCount() == 0){
                        component.second->texture()->unload();
                    }
                }
            }
        }
        void _addToMaterialPool(){
            this->m_ID = Material::m_MaterialProperities.size();
			glm::vec4 data(m_Frensel,m_SpecularityPower,m_LightingMode,m_Shadeless);
            Material::m_MaterialProperities.push_back(data);
        }
        void _updateGlobalMaterialPool(){
            glm::vec4& data = Material::m_MaterialProperities.at(m_ID);

			data.r = m_Frensel;
			data.g = m_SpecularityPower;
			if(m_LightingMode != Material::LightingMode::BLINNPHONG && m_LightingMode != Material::LightingMode::PHONG){
				data.g = glm::clamp(data.g,0.01f,0.99f);
			}
			data.b = float(m_LightingMode);
			data.a = m_Shadeless;
        }
        void _destruct(){
            for(auto component:m_Components)
                delete component.second;
        }
        void _addComponentDiffuse(Texture* texture){
            if((m_Components.count(MaterialComponentType::DIFFUSE) && m_Components[MaterialComponentType::DIFFUSE] != nullptr) || texture == nullptr)
                return;
            m_Components.emplace(MaterialComponentType::DIFFUSE,new MaterialComponent(MaterialComponentType::DIFFUSE,texture));
        }
        void _addComponentNormal(Texture* texture){
            if((m_Components.count(MaterialComponentType::NORMAL) && m_Components[MaterialComponentType::NORMAL] != nullptr) || texture == nullptr)
                return;
            m_Components.emplace(MaterialComponentType::NORMAL,new MaterialComponent(MaterialComponentType::NORMAL,texture));
        }
        void _addComponentGlow(Texture* texture){
            if((m_Components.count(MaterialComponentType::GLOW) && m_Components[MaterialComponentType::GLOW] != nullptr) || texture == nullptr)
                return;
            m_Components.emplace(MaterialComponentType::GLOW,new MaterialComponent(MaterialComponentType::GLOW,texture));
        }
        void _addComponentSpecular(Texture* texture){
            if((m_Components.count(MaterialComponentType::SPECULAR) && m_Components[MaterialComponentType::SPECULAR] != nullptr) || texture == nullptr)
                return;
            m_Components.emplace(MaterialComponentType::SPECULAR,new MaterialComponent(MaterialComponentType::SPECULAR,texture));
        }
        void _addComponentAO(Texture* map,float aoBaseValue){
            if((m_Components.count(MaterialComponentType::AO) && m_Components[MaterialComponentType::AO] != nullptr) || (map == nullptr))
                return;
            m_Components.emplace(MaterialComponentType::AO,new MaterialComponentAO(map,aoBaseValue));
        }
        void _addComponentMetalness(Texture* map,float metalnessBaseValue){
            if((m_Components.count(MaterialComponentType::METALNESS) && m_Components[MaterialComponentType::METALNESS] != nullptr) || (map == nullptr))
                return;
            m_Components.emplace(MaterialComponentType::METALNESS,new MaterialComponentMetalness(map,metalnessBaseValue));
        }
        void _addComponentRoughness(Texture* map,float roughnessBaseValue){
            if((m_Components.count(MaterialComponentType::ROUGHNESS) && m_Components[MaterialComponentType::ROUGHNESS] != nullptr) || (map == nullptr))
                return;
            m_Components.emplace(MaterialComponentType::ROUGHNESS,new MaterialComponentRoughness(map,roughnessBaseValue));
        }
        void _addComponentReflection(Texture* text,Texture* map,float mixFactor){
            if((m_Components.count(MaterialComponentType::REFLECTION) && m_Components[MaterialComponentType::REFLECTION] != nullptr) || (text == nullptr || map == nullptr))
                return;
            m_Components.emplace(MaterialComponentType::REFLECTION,new MaterialComponentReflection(MaterialComponentType::REFLECTION,text,map,mixFactor));
        }
        void _addComponentRefraction(Texture* text,Texture* map,float mixFactor,float ratio){
            if((m_Components.count(MaterialComponentType::REFRACTION) && m_Components[MaterialComponentType::REFRACTION] != nullptr) || (text == nullptr || map == nullptr))
                return;
            m_Components.emplace(MaterialComponentType::REFRACTION,new MaterialComponentRefraction(text,map,mixFactor,ratio));
        }
		void _setFrensel(float& f){ m_Frensel = glm::clamp(f,0.0001f,0.9999f); }
        void _setShadeless(bool& b){ m_Shadeless = b; _updateGlobalMaterialPool(); }
        void _setBaseGlow(float& f){ m_BaseGlow = f; _updateGlobalMaterialPool(); }
        void _setSpecularity(float& s){ m_SpecularityPower = s; _updateGlobalMaterialPool(); }
        void _setLightingMode(uint& m){ m_LightingMode = m; _updateGlobalMaterialPool(); }
};
DefaultMaterialBindFunctor Material::impl::DEFAULT_BIND_FUNCTOR;
DefaultMaterialUnbindFunctor Material::impl::DEFAULT_UNBIND_FUNCTOR;

Material::Material(std::string name,std::string diffuse, std::string normal, std::string glow,std::string specular,std::string program):m_i(new impl),BindableResource(name){
    m_i->_init(name,diffuse,normal,glow,specular,this);
}
Material::Material(std::string name,Texture* diffuse,Texture* normal,Texture* glow,Texture* specular,ShaderP* program):m_i(new impl),BindableResource(name){
    m_i->_init(name,diffuse,normal,glow,specular,this);
}
Material::~Material(){
    m_i->_destruct();
	for(auto entry:m_i->m_Meshes){
		delete entry;
		entry = nullptr;
	}
	m_i->m_Meshes.clear();
}
void Material::addComponentDiffuse(Texture* texture){
	m_i->_addComponentDiffuse(texture);
}
void Material::addComponentDiffuse(std::string& textureFile){
    Texture* texture = Resources::getTexture(textureFile); 
    if(texture == nullptr && textureFile != "") texture = new Texture(textureFile);
    m_i->_addComponentDiffuse(texture);
}
void Material::addComponentNormal(Texture* texture){
    m_i->_addComponentNormal(texture);
}
void Material::addComponentNormal(std::string& textureFile){
    Texture* texture = Resources::getTexture(textureFile); 
    if(texture == nullptr && textureFile != "") texture = new Texture(textureFile,"",GL_TEXTURE_2D,GL_RGBA8);
    m_i->_addComponentNormal(texture);
}
void Material::addComponentGlow(Texture* texture){
    m_i->_addComponentGlow(texture);
}
void Material::addComponentGlow(std::string& textureFile){
    Texture* texture = Resources::getTexture(textureFile); 
    if(texture == nullptr && textureFile != "") texture = new Texture(textureFile,"",GL_TEXTURE_2D,GL_RGBA8);
    m_i->_addComponentGlow(texture);
}
void Material::addComponentSpecular(Texture* texture){
    m_i->_addComponentSpecular(texture);
}
void Material::addComponentSpecular(std::string& textureFile){
    Texture* texture = Resources::getTexture(textureFile); 
    if(texture == nullptr && textureFile != "") texture = new Texture(textureFile,"",GL_TEXTURE_2D,GL_RGBA8);
    m_i->_addComponentSpecular(texture);
}

void Material::addComponentAO(Texture* texture,float baseValue){
    m_i->_addComponentAO(texture,baseValue);
}
void Material::addComponentAO(std::string& textureFile,float baseValue){
    Texture* texture = Resources::getTexture(textureFile); 
    if(texture == nullptr && textureFile != "") texture = new Texture(textureFile,"",GL_TEXTURE_2D,GL_RGBA8);
    m_i->_addComponentAO(texture,baseValue);
}
void Material::addComponentMetalness(Texture* texture,float baseValue){
    m_i->_addComponentMetalness(texture,baseValue);
}
void Material::addComponentMetalness(std::string& textureFile,float baseValue){
    Texture* texture = Resources::getTexture(textureFile); 
    if(texture == nullptr && textureFile != "") texture = new Texture(textureFile,"",GL_TEXTURE_2D,GL_RGBA8);
    m_i->_addComponentMetalness(texture,baseValue);
}
void Material::addComponentRoughness(Texture* texture,float baseValue){
    m_i->_addComponentRoughness(texture,baseValue);
}
void Material::addComponentRoughness(std::string& textureFile,float baseValue){
    Texture* texture = Resources::getTexture(textureFile); 
    if(texture == nullptr && textureFile != "") texture = new Texture(textureFile,"",GL_TEXTURE_2D,GL_RGBA8);
    m_i->_addComponentRoughness(texture,baseValue);
}
void Material::addComponentReflection(Texture* cubemap,Texture* map,float mixFactor){
    if(cubemap == nullptr) cubemap = Resources::getCurrentScene()->getSkybox()->texture();
    m_i->_addComponentReflection(cubemap,map,mixFactor);
}
void Material::addComponentReflection(std::string textureFiles[],std::string mapFile,float mixFactor){
    Texture* cubemap = new Texture(textureFiles,"Cubemap ",GL_TEXTURE_CUBE_MAP);
    Texture* map = Resources::getTexture(mapFile); 
    if(map == nullptr && mapFile != "") map = new Texture(mapFile);
    Material::addComponentReflection(cubemap,map,mixFactor);
}
void Material::addComponentReflection(std::string cubemapName,std::string mapFile,float mixFactor){
    Texture* cubemap = Resources::getTexture(cubemapName); 
    if(cubemap == nullptr && cubemapName != ""){ cubemap = new Texture(cubemapName); }
    Texture* map = Resources::getTexture(mapFile); 
    if(map == nullptr && mapFile != "") map = new Texture(mapFile);
    Material::addComponentReflection(cubemap,map,mixFactor);
}
void Material::addComponentRefraction(Texture* cubemap,Texture* map,float mixFactor,float ratio){
    m_i->_addComponentRefraction(cubemap,map,mixFactor,ratio);
}
void Material::addComponentRefraction(std::string textureFiles[],std::string mapFile,float mixFactor,float ratio){
    Texture* cubemap = new Texture(textureFiles,"Cubemap ",GL_TEXTURE_CUBE_MAP);
    Texture* map = Resources::getTexture(mapFile); 
    if(map == nullptr && mapFile != "") map = new Texture(mapFile);
    Material::addComponentRefraction(cubemap,map,mixFactor,ratio);
}
void Material::addComponentRefraction(std::string cubemapName,std::string mapFile,float mixFactor,float ratio){
    Texture* cubemap = Resources::getTexture(cubemapName); 
    if(cubemap == nullptr && cubemapName != "") cubemap = new Texture(cubemapName);
    Texture* map = Resources::getTexture(mapFile); 
    if(map == nullptr && mapFile != "") map = new Texture(mapFile);
    Material::addComponentRefraction(cubemap,map,mixFactor,ratio);
}

const std::unordered_map<uint,MaterialComponent*>& Material::getComponents() const { return m_i->m_Components; }
const MaterialComponent* Material::getComponent(uint index) const { return m_i->m_Components.at(index); }
const MaterialComponentReflection* Material::getComponentReflection() const { return static_cast<MaterialComponentReflection*>(m_i->m_Components.at(MaterialComponentType::REFLECTION)); }
const MaterialComponentRefraction* Material::getComponentRefraction() const { return static_cast<MaterialComponentRefraction*>(m_i->m_Components.at(MaterialComponentType::REFRACTION)); }
const bool Material::shadeless() const { return m_i->m_Shadeless; }
const float Material::glow() const { return m_i->m_BaseGlow; }
const float Material::frensel() const { return m_i->m_Frensel; }
const float Material::specularity() const { return m_i->m_SpecularityPower; }
const uint Material::lightingMode() const { return m_i->m_LightingMode; }
const uint Material::id() const { return m_i->m_ID; }

void Material::setShadeless(bool b){ m_i->_setShadeless(b); }
void Material::setGlow(float f){ m_i->_setBaseGlow(f); }
void Material::setFrensel(float f){ m_i->_setFrensel(f); }
void Material::setSpecularity(float s){ m_i->_setSpecularity(s); }
void Material::setLightingMode(uint m){ m_i->_setLightingMode(m); }

struct less_than_key{
    inline bool operator() ( MaterialMeshEntry* struct1,  MaterialMeshEntry* struct2){
        return (struct1->mesh()->name() < struct2->mesh()->name());
    }
};

void Material::addMesh(std::string objectName){
	for(auto mesh:m_i->m_Meshes){
		if(mesh->mesh() == Resources::getMesh(objectName)){ return; }
	}
    m_i->m_Meshes.push_back(new MaterialMeshEntry(Resources::getMesh(objectName)));
    std::sort(m_i->m_Meshes.begin(),m_i->m_Meshes.end(),less_than_key());
}
void Material::removeMesh(std::string objectName){
    bool did = false;
    for (auto it = m_i->m_Meshes.cbegin(); it != m_i->m_Meshes.cend();){
        if( (*it)->mesh()->name() == objectName){
			delete (*it);
            m_i->m_Meshes.erase(it++);
            did = true;
        }
        else{
            ++it;
        }
    }
    if(did == true){
        std::sort(m_i->m_Meshes.begin(),m_i->m_Meshes.end(),less_than_key());
    }
}
std::vector<MaterialMeshEntry*>& Material::getMeshes(){ return m_i->m_Meshes; }

void Material::bind(){
    std::string _name = name();
    if(Renderer::Detail::RendererInfo::GeneralInfo::current_bound_material != _name){
        BindableResource::bind(); //bind custom data
        Renderer::Detail::RendererInfo::GeneralInfo::current_bound_material = _name;
    }
}
void Material::unbind(){
    if(Renderer::Detail::RendererInfo::GeneralInfo::current_bound_material != "NONE"){
        BindableResource::unbind();
        Renderer::Detail::RendererInfo::GeneralInfo::current_bound_material = "NONE";
    }
}
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
MaterialMeshEntry::MaterialMeshEntry(Mesh* mesh){
	m_Mesh = mesh;
}
MaterialMeshEntry::~MaterialMeshEntry(){
}
void MaterialMeshEntry::addMeshInstance(std::string& objectName,MeshInstance* meshInstance){
	if(!m_MeshInstances.count(objectName)){
		std::vector<MeshInstance*> vector;
		vector.push_back(meshInstance);
		m_MeshInstances.emplace(objectName,vector);
	}
	else{
		m_MeshInstances.at(objectName).push_back(meshInstance);
	}
}
void MaterialMeshEntry::removeMeshInstance(std::string& objectName,MeshInstance* meshInstance){
	if(m_MeshInstances.count(objectName)){
		std::vector<MeshInstance*>& vector = m_MeshInstances.at(objectName);
		std::vector<MeshInstance*>::iterator it = vector.begin();
		while(it != vector.end()) {
			MeshInstance* instance = static_cast<MeshInstance*>((*it));
			if(instance ==  meshInstance) {
				it = vector.erase(it);
			}
			else ++it;
		}
	}
}
