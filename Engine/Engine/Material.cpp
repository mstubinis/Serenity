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
#include <boost/tuple/tuple.hpp>

using namespace Engine;
using namespace std;

vector<glm::vec4> Material::m_MaterialProperities;
GLchar* MATERIAL_COMPONENT_SHADER_TEXTURE_NAMES[MaterialComponentType::Type::Number] = {
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
void Material::setAllDiffuseModels(DiffuseModel::Model m){ for(auto mat:Resources::Detail::ResourceManagement::m_Materials){ mat.second->setDiffuseModel(m); } }
void Material::setAllSpecularModels(SpecularModel::Model m){ for(auto mat:Resources::Detail::ResourceManagement::m_Materials){ mat.second->setSpecularModel(m); } }
struct DefaultMaterialBindFunctor{void operator()(BindableResource* r) const {
    Material* material = (Material*)r;
    glm::vec4 first(0.0f); glm::vec4 second(0.0f); glm::vec4 third(0.0f);
    for(uint i = 0; i < MaterialComponentType::Number; i++){
        if(material->getComponents().count(i)){
            MaterialComponent* component = material->getComponents().at(i);
            if(component->texture() != nullptr && component->texture()->address() != 0){
                //enable
                if     (i == 0) { first.x = 1.0f; }
                else if(i == 1) { first.y = 1.0f; }
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
                component->bind();
            }
            else{ 
                component->unbind(); 
            }
        }
    }
    Renderer::sendUniform1iSafe("Shadeless",int(material->shadeless()));

	Renderer::sendUniform4fSafe("MaterialBasePropertiesOne",material->glow(),material->ao(),material->metalness(),material->smoothness());

    Renderer::sendUniform1fSafe("matID",float(material->id()));
    Renderer::sendUniform4fSafe("FirstConditionals", first.x,first.y,first.z,first.w);
    Renderer::sendUniform4fSafe("SecondConditionals",second.x,second.y,second.z,second.w);
    Renderer::sendUniform4fSafe("ThirdConditionals",third.x,third.y,third.z,third.w);
}};
struct DefaultMaterialUnbindFunctor{void operator()(BindableResource* r) const {
    //Material* m = (Material*)r;
}};
struct srtKey{inline bool operator()(MaterialMeshEntry* _1,MaterialMeshEntry* _2){return(_1->mesh()->name()<_2->mesh()->name());}};

unordered_map<uint,vector<uint>> MATERIAL_TEXTURE_SLOTS_MAP = [](){
    unordered_map<uint,vector<uint>> m;
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
unordered_map<uint,boost::tuple<float,float,float,float,float>> MATERIAL_PROPERTIES = [](){
    unordered_map<uint,boost::tuple<float,float,float,float,float>> m;
    //Remember specular reflection of non metals is white!       //(F0)                         //Smoothness    //Metalness
    m[MaterialPhysics::Aluminium]            = boost::make_tuple(0.9131f,0.9215f,0.92452f,      0.75f,          1.0f);
    m[MaterialPhysics::Copper]               = boost::make_tuple(0.955f,0.6374f,0.5381f,        0.9f,           1.0f);
    m[MaterialPhysics::Diamond]              = boost::make_tuple(0.17196f,0.17196f,0.17196f,    0.98f,          0.0f);
    m[MaterialPhysics::Glass_Or_Ruby_High]   = boost::make_tuple(0.0773f,0.0773f,0.0773f,       0.98f,          0.0f);
    m[MaterialPhysics::Gold]                 = boost::make_tuple(1.022f,0.7655f,0.336f,         0.9f,           1.0f);
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

MaterialComponent::MaterialComponent(uint type,Texture* t){
    m_ComponentType = (MaterialComponentType::Type)type;
    m_Texture = t;
}
MaterialComponent::MaterialComponent(uint type,string& t){
    m_ComponentType = (MaterialComponentType::Type)type;
    m_Texture = Resources::getTexture(t); 
    if(m_Texture == nullptr && t != "") m_Texture = new Texture(t);
}
MaterialComponent::~MaterialComponent(){
}
void MaterialComponent::bind(){
    vector<uint>& slots = MATERIAL_TEXTURE_SLOTS_MAP.at(m_ComponentType);
    string textureTypeName = MATERIAL_COMPONENT_SHADER_TEXTURE_NAMES[m_ComponentType];
    for(uint i = 0; i < slots.size(); i++){
        Renderer::bindTextureSafe(textureTypeName.c_str(),m_Texture,slots.at(i));
    }
}
void MaterialComponent::unbind(){
    vector<uint>& slots = MATERIAL_TEXTURE_SLOTS_MAP.at(m_ComponentType);
    for(uint i = 0; i < slots.size(); i++){
        Renderer::unbindTexture2D(slots.at(i));
    }
}
MaterialComponentReflection::MaterialComponentReflection(uint type,Texture* cubemap,Texture* map,float mixFactor):MaterialComponent(type,cubemap){
    setMixFactor(mixFactor);
    m_Map = map;
}
MaterialComponentReflection::MaterialComponentReflection(uint type,string& cubemap,string& map,float mixFactor):MaterialComponent(type,cubemap){
    setMixFactor(mixFactor);
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
    vector<uint>& slots = MATERIAL_TEXTURE_SLOTS_MAP.at(m_ComponentType);
    string textureTypeName = MATERIAL_COMPONENT_SHADER_TEXTURE_NAMES[m_ComponentType];
    Renderer::sendUniform1fSafe("CubemapMixFactor",m_MixFactor);
    if(m_Texture == nullptr)
        Renderer::bindTextureSafe(textureTypeName.c_str(),Resources::getCurrentScene()->getSkybox()->texture(),slots.at(0));
    else
        Renderer::bindTextureSafe(textureTypeName.c_str(),m_Texture,slots.at(0));
    Renderer::bindTextureSafe((textureTypeName+"Map").c_str(),m_Map,slots.at(1));
}
void MaterialComponentReflection::unbind(){
    vector<uint>& slots = MATERIAL_TEXTURE_SLOTS_MAP.at(m_ComponentType);
    Renderer::unbindTexture2D(slots.at(0));
    Renderer::unbindTextureCubemap(slots.at(1));
}
MaterialComponentRefraction::MaterialComponentRefraction(Texture* cubemap,Texture* map,float i,float mix):MaterialComponentReflection(MaterialComponentType::Refraction,cubemap,map,mix){
    m_RefractionIndex = i;
}
MaterialComponentRefraction::MaterialComponentRefraction(string& cubemap,string& map,float i,float mix):MaterialComponentReflection(MaterialComponentType::Refraction,cubemap,map,mix){
    m_RefractionIndex = i;
}
MaterialComponentRefraction::~MaterialComponentRefraction(){
    MaterialComponentReflection::~MaterialComponentReflection();
}
void MaterialComponentRefraction::bind(){
    vector<uint>& slots = MATERIAL_TEXTURE_SLOTS_MAP.at(m_ComponentType);
    string textureTypeName = MATERIAL_COMPONENT_SHADER_TEXTURE_NAMES[m_ComponentType];
    Renderer::sendUniform1fSafe("CubemapMixFactor",m_MixFactor);
    Renderer::sendUniform1fSafe("RefractionIndex",m_RefractionIndex);
    if(m_Texture == nullptr)
        Renderer::bindTextureSafe(textureTypeName.c_str(),Resources::getCurrentScene()->getSkybox()->texture(),slots.at(0));
    else
        Renderer::bindTextureSafe(textureTypeName.c_str(),m_Texture,slots.at(0));
    Renderer::bindTextureSafe((textureTypeName+"Map").c_str(),m_Map,slots.at(1));
}

MaterialComponentParallaxOcclusion::MaterialComponentParallaxOcclusion(Texture* map,float heightScale):MaterialComponent(MaterialComponentType::ParallaxOcclusion,map){
    setHeightScale(heightScale);
}
MaterialComponentParallaxOcclusion::MaterialComponentParallaxOcclusion(string& map,float heightScale):MaterialComponent(MaterialComponentType::ParallaxOcclusion,map){
    setHeightScale(heightScale);
}
MaterialComponentParallaxOcclusion::~MaterialComponentParallaxOcclusion(){
    MaterialComponent::~MaterialComponent();
}
void MaterialComponentParallaxOcclusion::setHeightScale(float factor){
    m_HeightScale = factor;
}
void MaterialComponentParallaxOcclusion::bind(){
    vector<uint>& slots = MATERIAL_TEXTURE_SLOTS_MAP.at(m_ComponentType);
    string textureTypeName = MATERIAL_COMPONENT_SHADER_TEXTURE_NAMES[m_ComponentType];
    Renderer::sendUniform1fSafe("ParallaxHeightScale",m_HeightScale);
    Renderer::bindTextureSafe(textureTypeName.c_str(),m_Texture,slots.at(0));
}
void MaterialComponentParallaxOcclusion::unbind(){
    vector<uint>& slots = MATERIAL_TEXTURE_SLOTS_MAP.at(m_ComponentType);
    Renderer::unbindTexture2D(slots.at(0));
}

class MaterialMeshEntry::impl final{
    public:
	    Mesh* m_Mesh;
        unordered_map<string,vector<MeshInstance*>> m_MeshInstances;

		void _init(Mesh* mesh){
			m_Mesh = mesh;
		}
		void _addMeshInstance(const string& obj,MeshInstance* meshInstance){
			if(!m_MeshInstances.count(obj)){
				m_MeshInstances.emplace(obj,vector<MeshInstance*>(1,meshInstance));
			}
			else{
				m_MeshInstances.at(obj).push_back(meshInstance);
			}
		}
		void _removeMeshInstance(const string& obj,MeshInstance* meshInstance){
			if(m_MeshInstances.count(obj)){
				vector<MeshInstance*>& v = m_MeshInstances.at(obj);
				auto it = v.begin();
				while(it != v.end()) {
					MeshInstance* instance = (*it);
					if(instance ==  meshInstance) {
						//do not delete the instance here
						it = v.erase(it);
					}
					else ++it;
				}
			}
		}
};
class Material::impl final{
    public:
        static DefaultMaterialBindFunctor DEFAULT_BIND_FUNCTOR;
        static DefaultMaterialUnbindFunctor DEFAULT_UNBIND_FUNCTOR;

        unordered_map<uint,MaterialComponent*> m_Components;
        vector<MaterialMeshEntry*> m_Meshes;
    
        uint m_DiffuseModel;
        uint m_SpecularModel;
  
        bool m_Shadeless;
        float m_BaseGlow;
    
        glm::vec3 m_F0Color;

        float m_BaseSmoothness;
        float m_BaseMetalness;
        float m_BaseAO;

        uint m_ID;
        void _init(string& name,Texture* diffuse,Texture* normal,Texture* glow,Texture* specular,Material* super){
            _addComponentDiffuse(diffuse);
            _addComponentNormal(normal);
            _addComponentGlow(glow);
            _addComponentSpecular(specular);
            
            m_SpecularModel = SpecularModel::Model::Cook_Torrance;
            m_DiffuseModel = DiffuseModel::Model::Lambert;

            _addToMaterialPool();

            _setF0Color(0.04f,0.04f,0.04f);
            _setSmoothness(0.7f);
            _setAO(1.0f);
            _setMetalness(0.0f);
            
            m_Shadeless = false;
            m_BaseGlow = 0.0f;

            super->setCustomBindFunctor(Material::impl::DEFAULT_BIND_FUNCTOR);
            super->setCustomUnbindFunctor(Material::impl::DEFAULT_UNBIND_FUNCTOR);
            super->load();
        }
        void _init(string& name,string& diffuse,string& normal,string& glow,string& specular,Material* super){
            Texture* diffuseT = Resources::getTexture(diffuse); 
            Texture* normalT = Resources::getTexture(normal); 
            Texture* glowT = Resources::getTexture(glow);
            Texture* specularT = Resources::getTexture(specular);
            if(diffuseT == nullptr && diffuse != "") diffuseT = new Texture(diffuse);
            if(normalT == nullptr && normal != "") normalT = new Texture(normal,"",GL_TEXTURE_2D,false,ImageInternalFormat::RGBA8);
            if(glowT == nullptr && glow != "") glowT = new Texture(glow,"",GL_TEXTURE_2D,false,ImageInternalFormat::RGBA8);
            if(specularT == nullptr && specular != "") specularT = new Texture(specular,"",GL_TEXTURE_2D,false,ImageInternalFormat::RGBA8);
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
            glm::vec4 data(Math::pack3FloatsInto1FloatUnsigned(m_F0Color.r,m_F0Color.g,m_F0Color.b), m_BaseSmoothness, float(m_SpecularModel), float(m_DiffuseModel));
            Material::m_MaterialProperities.push_back(data);
        }
        void _updateGlobalMaterialPool(){
            glm::vec4& data = Material::m_MaterialProperities.at(m_ID);
            data.r = Math::pack3FloatsInto1FloatUnsigned(m_F0Color.r,m_F0Color.g,m_F0Color.b);
            data.g = m_BaseSmoothness;
            data.b = float(m_SpecularModel);
            data.a = float(m_DiffuseModel);
        }
        void _destruct(){
            for(auto component:m_Components)
                delete component.second;
        }
        void _addComponentDiffuse(Texture* texture){
            if((m_Components.count(MaterialComponentType::Diffuse) && m_Components.at(MaterialComponentType::Diffuse) != nullptr) || texture == nullptr)
                return;
            m_Components.emplace(MaterialComponentType::Diffuse,new MaterialComponent(MaterialComponentType::Diffuse,texture));
        }
        void _addComponentNormal(Texture* texture){
            if((m_Components.count(MaterialComponentType::Normal) && m_Components.at(MaterialComponentType::Normal) != nullptr) || texture == nullptr)
                return;
            m_Components.emplace(MaterialComponentType::Normal,new MaterialComponent(MaterialComponentType::Normal,texture));
        }
        void _addComponentGlow(Texture* texture){
            if((m_Components.count(MaterialComponentType::Glow) && m_Components.at(MaterialComponentType::Glow) != nullptr) || texture == nullptr)
                return;
            m_Components.emplace(MaterialComponentType::Glow,new MaterialComponent(MaterialComponentType::Glow,texture));
        }
        void _addComponentSpecular(Texture* texture){
            if((m_Components.count(MaterialComponentType::Specular) && m_Components.at(MaterialComponentType::Specular) != nullptr) || texture == nullptr)
                return;
            m_Components.emplace(MaterialComponentType::Specular,new MaterialComponent(MaterialComponentType::Specular,texture));
        }
        void _addComponentAO(Texture* map){
            if((m_Components.count(MaterialComponentType::AO) && m_Components.at(MaterialComponentType::AO) != nullptr) || (map == nullptr))
                return;
            m_Components.emplace(MaterialComponentType::AO,new MaterialComponent(MaterialComponentType::AO,map));
        }
        void _addComponentMetalness(Texture* map){
            if((m_Components.count(MaterialComponentType::Metalness) && m_Components.at(MaterialComponentType::Metalness) != nullptr) || (map == nullptr))
                return;
            m_Components.emplace(MaterialComponentType::Metalness,new MaterialComponent(MaterialComponentType::Metalness,map));
        }
        void _addComponentSmoothness(Texture* map){
            if((m_Components.count(MaterialComponentType::Smoothness) && m_Components.at(MaterialComponentType::Smoothness) != nullptr) || (map == nullptr))
                return;
            m_Components.emplace(MaterialComponentType::Smoothness,new MaterialComponent(MaterialComponentType::Smoothness,map));
        }
        void _addComponentReflection(Texture* text,Texture* map,float mixFactor){
            if((m_Components.count(MaterialComponentType::Reflection) && m_Components.at(MaterialComponentType::Reflection) != nullptr) || (text == nullptr || map == nullptr))
                return;
            m_Components.emplace(MaterialComponentType::Reflection,new MaterialComponentReflection(MaterialComponentType::Reflection,text,map,mixFactor));
        }
        void _addComponentRefraction(Texture* text,Texture* map,float refractiveIndex,float mixFactor){
            if((m_Components.count(MaterialComponentType::Refraction) && m_Components.at(MaterialComponentType::Refraction) != nullptr) || (text == nullptr || map == nullptr))
                return;
            m_Components.emplace(MaterialComponentType::Refraction,new MaterialComponentRefraction(text,map,refractiveIndex,mixFactor));
        }
        void _addComponentParallaxOcclusion(Texture* map,float heightScale){
			uint type = MaterialComponentType::ParallaxOcclusion;
            if((m_Components.count(type) && m_Components.at(type) != nullptr) || (map == nullptr))
                return;
            m_Components.emplace(type,new MaterialComponentParallaxOcclusion(map,heightScale));
        }
        void _setF0Color(float r, float g, float b){
            m_F0Color.r = glm::clamp(r,0.001f,0.999f); 
            m_F0Color.g = glm::clamp(g,0.001f,0.999f); 
            m_F0Color.b = glm::clamp(b,0.001f,0.999f);
            _updateGlobalMaterialPool();
        }
        void _setMaterialProperties(float& r,float& g,float& b,float& smoothness,float& metalness){
            _setF0Color(r,g,b);
            _setSmoothness(smoothness);
            _setMetalness(metalness);
            _updateGlobalMaterialPool();
        }
        void _setShadeless(bool b){                      m_Shadeless = b;      _updateGlobalMaterialPool(); }
        void _setBaseGlow(float f){                      m_BaseGlow = f;       _updateGlobalMaterialPool(); }
        void _setSmoothness(float s){                    m_BaseSmoothness = glm::clamp(s,0.001f,0.999f); _updateGlobalMaterialPool(); }
        void _setSpecularModel(SpecularModel::Model& m){ m_SpecularModel = m;  _updateGlobalMaterialPool(); }
        void _setDiffuseModel(DiffuseModel::Model& m){   m_DiffuseModel = m;   _updateGlobalMaterialPool(); }
        void _setAO(float a){                            m_BaseAO = glm::clamp(a,0.001f,0.999f);         _updateGlobalMaterialPool(); }
        void _setMetalness(float m){                     m_BaseMetalness = glm::clamp(m,0.001f,0.999f);  _updateGlobalMaterialPool(); }
};
DefaultMaterialBindFunctor Material::impl::DEFAULT_BIND_FUNCTOR;
DefaultMaterialUnbindFunctor Material::impl::DEFAULT_UNBIND_FUNCTOR;


MaterialMeshEntry::MaterialMeshEntry(Mesh* mesh):m_i(new impl){
    m_i->_init(mesh);
}
MaterialMeshEntry::~MaterialMeshEntry(){
}
void MaterialMeshEntry::addMeshInstance(const string& objectName,MeshInstance* meshInstance){
	m_i->_addMeshInstance(objectName,meshInstance);
}
void MaterialMeshEntry::removeMeshInstance(const string& objectName,MeshInstance* meshInstance){
	m_i->_removeMeshInstance(objectName,meshInstance);
}
Mesh* MaterialMeshEntry::mesh(){ return m_i->m_Mesh; }
unordered_map<string,vector<MeshInstance*>>& MaterialMeshEntry::meshInstances(){ return m_i->m_MeshInstances; }



Material::Material(string name,string diffuse,string normal,string glow,string specular,string program):m_i(new impl),BindableResource(name){
    m_i->_init(name,diffuse,normal,glow,specular,this);
}
Material::Material(string name,Texture* diffuse,Texture* normal,Texture* glow,Texture* specular,ShaderP* program):m_i(new impl),BindableResource(name){
    m_i->_init(name,diffuse,normal,glow,specular,this);
}
Material::~Material(){
    m_i->_destruct();
    for(auto materialMeshEntry:m_i->m_Meshes){
        SAFE_DELETE(materialMeshEntry);
    }
	vector_clear(m_i->m_Meshes);
}
void Material::addComponentDiffuse(Texture* texture){
    m_i->_addComponentDiffuse(texture);
}
void Material::addComponentDiffuse(string textureFile){
    Texture* texture = Resources::getTexture(textureFile); 
    if(texture == nullptr && textureFile != "") texture = new Texture(textureFile);
    m_i->_addComponentDiffuse(texture);
}
void Material::addComponentNormal(Texture* texture){
    m_i->_addComponentNormal(texture);
}
void Material::addComponentNormal(string textureFile){
    Texture* texture = Resources::getTexture(textureFile); 
    if(texture == nullptr && textureFile != "") texture = new Texture(textureFile,"",GL_TEXTURE_2D,false,ImageInternalFormat::RGBA8);
    m_i->_addComponentNormal(texture);
}
void Material::addComponentGlow(Texture* texture){
    m_i->_addComponentGlow(texture);
}
void Material::addComponentGlow(string textureFile){
    Texture* texture = Resources::getTexture(textureFile); 
    if(texture == nullptr && textureFile != "") texture = new Texture(textureFile,"",GL_TEXTURE_2D,false,ImageInternalFormat::RGBA8);
    m_i->_addComponentGlow(texture);
}
void Material::addComponentSpecular(Texture* texture){
    m_i->_addComponentSpecular(texture);
}
void Material::addComponentSpecular(string textureFile){
    Texture* texture = Resources::getTexture(textureFile); 
    if(texture == nullptr && textureFile != "") texture = new Texture(textureFile,"",GL_TEXTURE_2D,false,ImageInternalFormat::RGBA8);
    m_i->_addComponentSpecular(texture);
}
void Material::addComponentAO(Texture* texture,float baseValue){
    m_i->_addComponentAO(texture);
    setAO(baseValue);
}
void Material::addComponentAO(string textureFile,float baseValue){
    Texture* texture = Resources::getTexture(textureFile); 
    if(texture == nullptr && textureFile != "") texture = new Texture(textureFile,"",GL_TEXTURE_2D,false,ImageInternalFormat::RGBA8);
    m_i->_addComponentAO(texture);
    setAO(baseValue);
}
void Material::addComponentMetalness(Texture* texture,float baseValue){
    m_i->_addComponentMetalness(texture);
    setMetalness(baseValue);
}
void Material::addComponentMetalness(string textureFile,float baseValue){
    Texture* texture = Resources::getTexture(textureFile); 
    if(texture == nullptr && textureFile != "") texture = new Texture(textureFile,"",GL_TEXTURE_2D,false,ImageInternalFormat::RGBA8);
    m_i->_addComponentMetalness(texture);
    setMetalness(baseValue);
}
void Material::addComponentSmoothness(Texture* texture,float baseValue){
    m_i->_addComponentSmoothness(texture);
    setSmoothness(baseValue);
}
void Material::addComponentSmoothness(string textureFile,float baseValue){
    Texture* texture = Resources::getTexture(textureFile); 
    if(texture == nullptr && textureFile != "") texture = new Texture(textureFile,"",GL_TEXTURE_2D,false,ImageInternalFormat::RGBA8);
    m_i->_addComponentSmoothness(texture);
    setSmoothness(baseValue);
}
void Material::addComponentReflection(Texture* cubemap,Texture* map,float mixFactor){
    if(cubemap == nullptr) cubemap = Resources::getCurrentScene()->getSkybox()->texture();
    m_i->_addComponentReflection(cubemap,map,mixFactor);
}
void Material::addComponentReflection(string textureFiles[],string mapFile,float mixFactor){
    Texture* cubemap = new Texture(textureFiles,"Cubemap ",GL_TEXTURE_CUBE_MAP);
    Texture* map = Resources::getTexture(mapFile); 
    if(map == nullptr && mapFile != "") map = new Texture(mapFile);
    Material::addComponentReflection(cubemap,map,mixFactor);
}
void Material::addComponentReflection(string cubemapName,string mapFile,float mixFactor){
    Texture* cubemap = Resources::getTexture(cubemapName); 
    if(cubemap == nullptr && cubemapName != ""){ cubemap = new Texture(cubemapName); }
    Texture* map = Resources::getTexture(mapFile); 
    if(map == nullptr && mapFile != "") map = new Texture(mapFile);
    Material::addComponentReflection(cubemap,map,mixFactor);
}
void Material::addComponentRefraction(Texture* cubemap,Texture* map,float refractiveIndex,float mixFactor){
    m_i->_addComponentRefraction(cubemap,map,refractiveIndex,mixFactor);
}
void Material::addComponentRefraction(string textureFiles[],string mapFile,float refractiveIndex,float mixFactor){
    Texture* cubemap = new Texture(textureFiles,"Cubemap ",GL_TEXTURE_CUBE_MAP);
    Texture* map = Resources::getTexture(mapFile); 
    if(map == nullptr && mapFile != "") map = new Texture(mapFile);
    Material::addComponentRefraction(cubemap,map,refractiveIndex,mixFactor);
}
void Material::addComponentRefraction(string cubemapName,string mapFile,float refractiveIndex,float mixFactor){
    Texture* cubemap = Resources::getTexture(cubemapName); 
    if(cubemap == nullptr && cubemapName != "") cubemap = new Texture(cubemapName);
    Texture* map = Resources::getTexture(mapFile); 
    if(map == nullptr && mapFile != "") map = new Texture(mapFile);
    Material::addComponentRefraction(cubemap,map,refractiveIndex,mixFactor);
}
void Material::addComponentParallaxOcclusion(Texture* texture,float heightScale){
    m_i->_addComponentParallaxOcclusion(texture,heightScale);
}
void Material::addComponentParallaxOcclusion(std::string textureFile,float heightScale){
    Texture* texture = Resources::getTexture(textureFile); 
    if(texture == nullptr && textureFile != "") texture = new Texture(textureFile,"",GL_TEXTURE_2D,false,ImageInternalFormat::RGBA8);
    m_i->_addComponentParallaxOcclusion(texture,heightScale);
}
const unordered_map<uint,MaterialComponent*>& Material::getComponents() const { return m_i->m_Components; }
const MaterialComponent* Material::getComponent(uint index) const { return m_i->m_Components.at(index); }
const MaterialComponentReflection* Material::getComponentReflection() const { return (MaterialComponentReflection*)(m_i->m_Components.at(MaterialComponentType::Reflection)); }
const MaterialComponentRefraction* Material::getComponentRefraction() const { return (MaterialComponentRefraction*)(m_i->m_Components.at(MaterialComponentType::Refraction)); }
const MaterialComponentParallaxOcclusion* Material::getComponentParallaxOcclusion() const { return (MaterialComponentParallaxOcclusion*)(m_i->m_Components.at(MaterialComponentType::ParallaxOcclusion)); }
const bool Material::shadeless() const { return m_i->m_Shadeless; }
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
    boost::tuple<float, float, float, float, float>& t = MATERIAL_PROPERTIES.at(c);
    m_i->_setMaterialProperties( t.get<0>(), t.get<1>(), t.get<2>(), t.get<3>(), t.get<4>() );
}
void Material::setSmoothness(float s){ m_i->_setSmoothness(s); }
void Material::setSpecularModel(SpecularModel::Model m){ m_i->_setSpecularModel(m); }
void Material::setDiffuseModel(DiffuseModel::Model m){ m_i->_setDiffuseModel(m); }
void Material::setAO(float a){ m_i->_setAO(a); }
void Material::setMetalness(float m){ m_i->_setMetalness(m); }
void Material::addMeshEntry(string objectName){
    for(auto entry:m_i->m_Meshes){
        if(entry->mesh() == Resources::getMesh(objectName)){ return; }
    }
    m_i->m_Meshes.push_back(new MaterialMeshEntry(Resources::getMesh(objectName)));
    sort(m_i->m_Meshes.begin(),m_i->m_Meshes.end(),srtKey());
}
void Material::removeMeshEntry(string objectName){
    bool did = false;
    for (auto it = m_i->m_Meshes.cbegin(); it != m_i->m_Meshes.cend();){
		MaterialMeshEntry* entry = (*it);
        if(entry->mesh()->name() == objectName){
            SAFE_DELETE(entry); //do we need this?
            m_i->m_Meshes.erase(it++);
            did = true;
        }
        else ++it;
    }
    if(did){ sort(m_i->m_Meshes.begin(),m_i->m_Meshes.end(),srtKey()); }
}
void Material::bind(){
    string& _name = name();
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
vector<MaterialMeshEntry*>& Material::getMeshEntries(){ return m_i->m_Meshes; }