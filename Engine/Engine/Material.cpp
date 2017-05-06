#include "ObjectDynamic.h"
#include "ObjectDisplay.h"
#include "RenderedItem.h"
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
    for(uint i = 0; i < MATERIAL_COMPONENT_TYPE_NUMBER; i++){
        MATERIAL_COMPONENT_TYPE type = (MATERIAL_COMPONENT_TYPE)i;
        if(material->getComponents().count(type)){
            MaterialComponent* component = material->getComponents().at(type);
            if(component->texture() != nullptr && component->texture()->address() != 0){
                //enable
                if     (i == 0){ first.x = 1.0f; }
                else if(i == 1){ first.y = 1.0f; }
                else if(i == 2){ first.z = 1.0f; }
                else if(i == 3){ second.x = 1.0f; }
                else if(i == 4){ second.y = 1.0f; }
                else if(i == 5){ second.z = 1.0f; }
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
    Renderer::sendUniform1fSafe("matID",float(float(material->id())/255.0f));
    Renderer::sendUniform3fSafe("FirstConditionals", first.x,first.y,first.z);
    Renderer::sendUniform3fSafe("SecondConditionals",second.x,second.y,second.z);
}};
struct DefaultMaterialUnbindFunctor{void operator()(BindableResource* r) const {
    //Material* m = static_cast<Material*>(r);
}};


std::unordered_map<uint,std::vector<uint>> _populateTextureSlotMap(){
    std::unordered_map<uint,std::vector<uint>> map;

    map[MATERIAL_COMPONENT_TYPE_DIFFUSE].push_back((uint)MATERIAL_COMPONENT_TEXTURE_SLOT_DIFFUSE);
    map[MATERIAL_COMPONENT_TYPE_NORMAL].push_back((uint)MATERIAL_COMPONENT_TEXTURE_SLOT_NORMAL);
    map[MATERIAL_COMPONENT_TYPE_GLOW].push_back((uint)MATERIAL_COMPONENT_TEXTURE_SLOT_GLOW);
    map[MATERIAL_COMPONENT_TYPE_SPECULAR].push_back((uint)MATERIAL_COMPONENT_TEXTURE_SLOT_SPECULAR);

    map[MATERIAL_COMPONENT_TYPE_REFLECTION].push_back((uint)MATERIAL_COMPONENT_TEXTURE_SLOT_REFLECTION_CUBEMAP);
    map[MATERIAL_COMPONENT_TYPE_REFLECTION].push_back((uint)MATERIAL_COMPONENT_TEXTURE_SLOT_REFLECTION_CUBEMAP_MAP);

    map[MATERIAL_COMPONENT_TYPE_REFRACTION].push_back((uint)MATERIAL_COMPONENT_TEXTURE_SLOT_REFRACTION_CUBEMAP);
    map[MATERIAL_COMPONENT_TYPE_REFRACTION].push_back((uint)MATERIAL_COMPONENT_TEXTURE_SLOT_REFRACTION_CUBEMAP_MAP);

    return map;
}
std::unordered_map<uint,std::vector<uint>> Material::MATERIAL_TEXTURE_SLOTS_MAP = _populateTextureSlotMap();

MaterialComponent::MaterialComponent(uint ty,Texture* t){
    m_ComponentType = (MATERIAL_COMPONENT_TYPE)ty;
    m_Texture = t;
}
MaterialComponent::MaterialComponent(uint ty,std::string& t){
    m_ComponentType = (MATERIAL_COMPONENT_TYPE)ty;
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
        std::string textureTypeName = MATERIAL_COMPONENT_SHADER_TEXTURE_NAMES[m_ComponentType];
        Renderer::unbindTexture2D(slots.at(i));
    }
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
    std::string textureTypeName = MATERIAL_COMPONENT_SHADER_TEXTURE_NAMES[m_ComponentType];

    Renderer::unbindTexture2D(slots.at(0));
    Renderer::unbindTextureCubemap(slots.at(1));
}
MaterialComponentRefraction::MaterialComponentRefraction(uint type,Texture* cubemap,Texture* map,float mixFactor,float ratio):MaterialComponentReflection(type,cubemap,map,mixFactor){
    m_RefractionRatio = ratio;
}
MaterialComponentRefraction::MaterialComponentRefraction(uint type,std::string& cubemap,std::string& map,float mixFactor,float ratio):MaterialComponentReflection(type,cubemap,map,mixFactor){
    m_RefractionRatio = ratio;
}
MaterialComponentRefraction::~MaterialComponentRefraction(){
    MaterialComponentReflection::~MaterialComponentReflection();
}
void MaterialComponentRefraction::bind(){
    std::vector<uint>& slots = Material::MATERIAL_TEXTURE_SLOTS_MAP[m_ComponentType];
    std::string textureTypeName = MATERIAL_COMPONENT_SHADER_TEXTURE_NAMES[m_ComponentType];

    Renderer::sendUniform1fSafe("CubemapMixFactor",m_MixFactor);
    Renderer::sendUniform1fSafe("RefractionRatio",m_RefractionRatio);

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
        std::vector<RenderedItem*> m_Objects;
        uint m_LightingMode;
        bool m_Shadeless;
        float m_BaseGlow;
        float m_SpecularityPower;
        uint m_ID;
        void _init(std::string& name,Texture* diffuse,Texture* normal,Texture* glow,Texture* specular,Material* super){
            _addComponent(MATERIAL_COMPONENT_TYPE_DIFFUSE,diffuse);
            _addComponent(MATERIAL_COMPONENT_TYPE_NORMAL,normal);
            _addComponent(MATERIAL_COMPONENT_TYPE_GLOW,glow);
            _addComponent(MATERIAL_COMPONENT_TYPE_SPECULAR,specular);
            m_Shadeless = false;
            m_BaseGlow = 0.0f;
            m_SpecularityPower = 50.0f;
            m_LightingMode = MATERIAL_LIGHTING_MODE_BLINNPHONG;
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
            if(normalT == nullptr && normal != "")  normalT = new Texture(normal);
            if(glowT == nullptr && glow != "")    glowT = new Texture(glow);
            if(specularT == nullptr && specular != "")    specularT = new Texture(specular);
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
            Material::m_MaterialProperities.push_back(glm::vec4(m_BaseGlow,m_SpecularityPower,m_LightingMode,m_Shadeless));
        }
        void _updateGlobalMaterialPool(){
            glm::vec4& ref = Material::m_MaterialProperities.at(m_ID);
            ref.r = m_BaseGlow;
            ref.g = m_SpecularityPower;
            ref.b = float(m_LightingMode);
            ref.a = m_Shadeless;
        }
        void _destruct(){
            for(auto component:m_Components)
                delete component.second;
        }
        void _addComponent(uint type, Texture* texture){
            if((m_Components.count(type) && m_Components[type] != nullptr) || texture == nullptr)
                return;
            m_Components.emplace(type,new MaterialComponent(type,texture));
        }
        void _addComponentReflection(Texture* cubemap,Texture* map,float mixFactor){
            uint type = (uint)MATERIAL_COMPONENT_TYPE_REFLECTION;
            if((m_Components.count(type) && m_Components[type] != nullptr) || (cubemap == nullptr || map == nullptr))
                return;
            m_Components.emplace(type,new MaterialComponentReflection(type,cubemap,map,mixFactor));
        }
        void _addComponentRefraction(Texture* cubemap,Texture* map,float mixFactor,float ratio){
            uint type = (uint)MATERIAL_COMPONENT_TYPE_REFRACTION;
            if((m_Components.count(type) && m_Components[type] != nullptr) || (cubemap == nullptr || map == nullptr))
                return;
            m_Components.emplace(type,new MaterialComponentRefraction(type,cubemap,map,mixFactor,ratio));
        }
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
}
void Material::addComponent(uint type, Texture* texture){m_i->_addComponent(type,texture);}
void Material::addComponentDiffuse(Texture* texture){m_i->_addComponent(MATERIAL_COMPONENT_TYPE_DIFFUSE,texture);}
void Material::addComponentDiffuse(std::string& textureFile){
    Texture* texture = Resources::getTexture(textureFile); 
    if(texture == nullptr && textureFile != "") texture = new Texture(textureFile);
    m_i->_addComponent(MATERIAL_COMPONENT_TYPE_DIFFUSE,texture);
}
void Material::addComponentNormal(Texture* texture){
    m_i->_addComponent(MATERIAL_COMPONENT_TYPE_NORMAL,texture);
}
void Material::addComponentNormal(std::string& textureFile){
    Texture* texture = Resources::getTexture(textureFile); 
    if(texture == nullptr && textureFile != "") texture = new Texture(textureFile);
    m_i->_addComponent(MATERIAL_COMPONENT_TYPE_NORMAL,texture);
}
void Material::addComponentSpecular(Texture* texture){
    m_i->_addComponent(MATERIAL_COMPONENT_TYPE_SPECULAR,texture);
}
void Material::addComponentSpecular(std::string& textureFile){
    Texture* texture = Resources::getTexture(textureFile); 
    if(texture == nullptr && textureFile != "") texture = new Texture(textureFile);
    m_i->_addComponent(MATERIAL_COMPONENT_TYPE_SPECULAR,texture);
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
const MaterialComponentReflection* Material::getComponentReflection() const { return static_cast<MaterialComponentReflection*>(m_i->m_Components.at((uint)MATERIAL_COMPONENT_TYPE_REFLECTION)); }
const MaterialComponentRefraction* Material::getComponentRefraction() const { return static_cast<MaterialComponentRefraction*>(m_i->m_Components.at((uint)MATERIAL_COMPONENT_TYPE_REFRACTION)); }
const bool Material::shadeless() const { return m_i->m_Shadeless; }
const float Material::glow() const { return m_i->m_BaseGlow; }
const float Material::specularity() const { return m_i->m_SpecularityPower; }
const uint Material::lightingMode() const { return m_i->m_LightingMode; }
const uint Material::id() const { return m_i->m_ID; }

void Material::setShadeless(bool b){ m_i->_setShadeless(b); }
void Material::setGlow(float f){ m_i->_setBaseGlow(f); }
void Material::setSpecularity(float s){ m_i->_setSpecularity(s); }
void Material::setLightingMode(uint m){ m_i->_setLightingMode(m); }

struct less_than_key{
    inline bool operator() ( RenderedItem* struct1,  RenderedItem* struct2){
        return (struct1->name() < struct2->name());
    }
};

void Material::addObject(std::string objectName){
    RenderedItem* o = Resources::getRenderedItem(objectName);
    if(o != nullptr){
		m_i->m_Objects.push_back(o);
		std::sort(m_i->m_Objects.begin(),m_i->m_Objects.end(),less_than_key());
    }
}
void Material::removeObject(std::string objectName){
	bool did = false;
	for (auto it = m_i->m_Objects.cbegin(); it != m_i->m_Objects.cend();){
		if( (*it)->name() == objectName){
			m_i->m_Objects.erase(it++);
			did = true;
		}
		else{
			++it;
		}
	}
	if(did == true){
		std::sort(m_i->m_Objects.begin(),m_i->m_Objects.end(),less_than_key());
	}
}
std::vector<RenderedItem*>& Material::getObjects(){ return m_i->m_Objects; }

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