#include <unordered_map>
#include "Material.h"
#include "Texture.h"
#include "Engine_Resources.h"

using namespace Engine;

std::vector<glm::vec4> Material::m_MaterialProperities;

MaterialComponent::MaterialComponent(uint type,Texture* texture){
	m_ComponentType = (MATERIAL_COMPONENT_TYPE)type;
	m_Texture = texture;
}
MaterialComponent::MaterialComponent(uint type,std::string& textureFile){
	m_ComponentType = (MATERIAL_COMPONENT_TYPE)type;
    m_Texture = Resources::getTexture(textureFile); 
    if(m_Texture == nullptr && textureFile != "") m_Texture = new Texture(textureFile);
}
MaterialComponent::~MaterialComponent(){
}
void MaterialComponent::bind(GLuint shader,uint api){
	uint c = (uint)m_ComponentType;
    if(api == ENGINE_RENDERING_API_OPENGL){
        std::string textureTypeName = MATERIAL_COMPONENT_SHADER_TEXTURE_NAMES[c];
		if(m_Texture == nullptr || m_Texture->address() == 0){//disable this material
            glUniform1i(glGetUniformLocation(shader,(textureTypeName+"Enabled").c_str()), 0);
            return;
        }
		glEnable(m_Texture->type());
		glUniform1i(glGetUniformLocation(shader,(textureTypeName+"Enabled").c_str()), 1);
		glActiveTexture(GL_TEXTURE0 + c);
        glBindTexture(m_Texture->type(), m_Texture->address());
        glUniform1i(glGetUniformLocation(shader, textureTypeName.c_str()), c);
        
    }
    else if(api == ENGINE_RENDERING_API_DIRECTX){
    }
}
void MaterialComponent::unbind(uint api){
	uint c = (uint)m_ComponentType;
    if(api == ENGINE_RENDERING_API_OPENGL){
		glActiveTexture(GL_TEXTURE0 + c);
		glBindTexture(m_Texture->type(), 0);
	}
    else if(api == ENGINE_RENDERING_API_DIRECTX){
    }
}
MaterialComponentReflection::MaterialComponentReflection(Texture* cubemap,Texture* map):MaterialComponent((uint)MATERIAL_COMPONENT_TEXTURE_REFLECTION,cubemap){
	m_MixFactor = 0.5f;
	m_ReflectionMap = map;
}
MaterialComponentReflection::MaterialComponentReflection(std::string& cubemap,std::string& map):MaterialComponent((uint)MATERIAL_COMPONENT_TEXTURE_REFLECTION,cubemap){
    m_MixFactor = 0.5f;
	m_ReflectionMap = Resources::getTexture(map); 
    if(m_ReflectionMap == nullptr && map != "") m_ReflectionMap = new Texture(map);
}
MaterialComponentReflection::~MaterialComponentReflection(){
	MaterialComponent::~MaterialComponent();
}
void MaterialComponentReflection::setMixFactor(float factor){
	m_MixFactor = glm::clamp(factor,0.0f,1.0f);
}
void MaterialComponentReflection::bind(GLuint shader,uint api){
	uint c = (uint)m_ComponentType;
    if(api == ENGINE_RENDERING_API_OPENGL){
        std::string textureTypeName = MATERIAL_COMPONENT_SHADER_TEXTURE_NAMES[c];
		if(m_Texture == nullptr || m_Texture->address() == 0){//disable this material component
            glUniform1i(glGetUniformLocation(shader,(textureTypeName+"Enabled").c_str()), 0);
            return;
        }
		glUniform1f(glGetUniformLocation(shader, "ReflectionMixFactor"), m_MixFactor);
		glUniform1i(glGetUniformLocation(shader,(textureTypeName+"Enabled").c_str()), 1);

		glEnable(m_Texture->type());
		glActiveTexture(GL_TEXTURE0 + c);
        glBindTexture(m_Texture->type(), m_Texture->address());
        glUniform1i(glGetUniformLocation(shader, textureTypeName.c_str()), c);

		glEnable(m_ReflectionMap->type());
		glActiveTexture(GL_TEXTURE0 + c + 1);
		glBindTexture(m_ReflectionMap->type(), m_ReflectionMap->address());
        glUniform1i(glGetUniformLocation(shader, (textureTypeName + "Map").c_str()), c + 1);
    }
    else if(api == ENGINE_RENDERING_API_DIRECTX){
    }
}
void MaterialComponentReflection::unbind(uint api){
	uint c = (uint)m_ComponentType;
    if(api == ENGINE_RENDERING_API_OPENGL){
		glActiveTexture(GL_TEXTURE0 + c);
		glBindTexture(m_Texture->type(), 0);

		glActiveTexture(GL_TEXTURE0 + c + 1);
		glBindTexture(m_ReflectionMap->type(), 0);
	}
    else if(api == ENGINE_RENDERING_API_DIRECTX){
    }
}

class Material::impl final{
    public:
        std::unordered_map<uint,MaterialComponent*> m_Components;
        uint m_LightingMode;
        bool m_Shadeless;
        float m_BaseGlow;
        float m_SpecularityPower;
		uint m_ID;
        void _init(Texture* diffuse,Texture* normal,Texture* glow,Texture* specular){

			_addComponent(MATERIAL_COMPONENT_TEXTURE_DIFFUSE,diffuse);
			_addComponent(MATERIAL_COMPONENT_TEXTURE_NORMAL,normal);
			_addComponent(MATERIAL_COMPONENT_TEXTURE_GLOW,glow);
			_addComponent(MATERIAL_COMPONENT_TEXTURE_SPECULAR,specular);

            m_Shadeless = false;
            m_BaseGlow = 0.0f;
            m_SpecularityPower = 50.0f;
            m_LightingMode = MATERIAL_LIGHTING_MODE_BLINNPHONG;
			_addToMaterialPool();
        }
        void _init(std::string& diffuse, std::string& normal, std::string& glow, std::string& specular){
            Texture* diffuseT = Resources::getTexture(diffuse); 
            Texture* normalT = Resources::getTexture(normal); 
            Texture* glowT = Resources::getTexture(glow);
			Texture* specularT = Resources::getTexture(specular);
            if(diffuseT == nullptr && diffuse != "") diffuseT = new Texture(diffuse);
            if(normalT == nullptr && normal != "")  normalT = new Texture(normal);
            if(glowT == nullptr && glow != "")    glowT = new Texture(glow);
			if(specularT == nullptr && specular != "")    specularT = new Texture(specular);
            _init(diffuseT,normalT,glowT,specularT);
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
            m_Components[type] = new MaterialComponent(type,texture);
        }
        void _addComponentReflection(Texture* cubemap,Texture* map){
			uint type = (uint)MATERIAL_COMPONENT_TEXTURE_REFLECTION;
			if((m_Components.count(type) && m_Components[type] != nullptr) || (cubemap == nullptr || map == nullptr))
                return;
            m_Components[type] = new MaterialComponentReflection(cubemap,map);
        }
        void _setShadeless(bool& b){ m_Shadeless = b; _updateGlobalMaterialPool(); }
        void _setBaseGlow(float& f){ m_BaseGlow = f; _updateGlobalMaterialPool(); }
        void _setSpecularity(float& s){ m_SpecularityPower = s; _updateGlobalMaterialPool(); }
        void _setLightingMode(uint& m){ m_LightingMode = m; _updateGlobalMaterialPool(); }
};

Material::Material(Texture* diffuse,Texture* normal,Texture* glow,Texture* specular):m_i(new impl()){
    m_i->_init(diffuse,normal,glow,specular);
}
Material::Material(std::string diffuse, std::string normal, std::string glow,std::string specular):m_i(new impl()){
    m_i->_init(diffuse,normal,glow,specular);
}
Material::~Material(){
    m_i->_destruct();
}
void Material::addComponent(uint type, Texture* texture){
    m_i->_addComponent(type,texture);
}
void Material::addComponentDiffuse(Texture* texture){
	m_i->_addComponent((uint)MATERIAL_COMPONENT_TEXTURE_DIFFUSE,texture);
}
void Material::addComponentDiffuse(std::string& textureFile){
    Texture* texture = Resources::getTexture(textureFile); 
    if(texture == nullptr && textureFile != "") texture = new Texture(textureFile);
	m_i->_addComponent((uint)MATERIAL_COMPONENT_TEXTURE_DIFFUSE,texture);
}

void Material::addComponentNormal(Texture* texture){
	m_i->_addComponent((uint)MATERIAL_COMPONENT_TEXTURE_NORMAL,texture);
}
void Material::addComponentNormal(std::string& textureFile){
    Texture* texture = Resources::getTexture(textureFile); 
    if(texture == nullptr && textureFile != "") texture = new Texture(textureFile);
	m_i->_addComponent((uint)MATERIAL_COMPONENT_TEXTURE_NORMAL,texture);
}
void Material::addComponentSpecular(Texture* texture){
	m_i->_addComponent((uint)MATERIAL_COMPONENT_TEXTURE_SPECULAR,texture);
}
void Material::addComponentSpecular(std::string& textureFile){
    Texture* texture = Resources::getTexture(textureFile); 
    if(texture == nullptr && textureFile != "") texture = new Texture(textureFile);
	m_i->_addComponent((uint)MATERIAL_COMPONENT_TEXTURE_SPECULAR,texture);
}
void Material::addComponentReflection(Texture* cubemap,Texture* map){
	m_i->_addComponentReflection(cubemap,map);
}
void Material::addComponentReflection(std::string textureFiles[],std::string mapFile){
	Texture* cubemap = new Texture(textureFiles,"Cubemap ",GL_TEXTURE_CUBE_MAP);

    Texture* map = Resources::getTexture(mapFile); 
    if(map == nullptr && mapFile != "") map = new Texture(mapFile);
	Material::addComponentReflection(cubemap,map);
}
void Material::addComponentReflection(std::string cubemapName,std::string mapFile){
    Texture* cubemap = Resources::getTexture(cubemapName); 
    if(cubemap == nullptr && cubemapName != "") cubemap = new Texture(cubemapName);

    Texture* map = Resources::getTexture(mapFile); 
    if(map == nullptr && mapFile != "") map = new Texture(mapFile);
	Material::addComponentReflection(cubemap,map);
}
void Material::bind(GLuint shader,GLuint api){
	for(uint i = 0; i < MATERIAL_COMPONENT_TYPE_NUMBER; i++){
		MATERIAL_COMPONENT_TYPE type = (MATERIAL_COMPONENT_TYPE)i;
		if(!m_i->m_Components.count(type)){
			//disable
			std::string textureTypeName = MATERIAL_COMPONENT_SHADER_TEXTURE_NAMES[(uint)type];
			glUniform1i(glGetUniformLocation(shader,(textureTypeName+"Enabled").c_str()), 0);
		}
		else{
			m_i->m_Components[type]->bind(shader,api);
		}
	}
}
void Material::unbind(GLuint api){
	for(uint i = 0; i < MATERIAL_COMPONENT_TYPE_NUMBER; i++){
		MATERIAL_COMPONENT_TYPE type = (MATERIAL_COMPONENT_TYPE)i;
		if(!m_i->m_Components.count(type)){
		}
		else{
			m_i->m_Components[type]->unbind(api);
		}
	}
}
std::unordered_map<uint,MaterialComponent*>& Material::getComponents(){ return m_i->m_Components; }
const MaterialComponent* Material::getComponent(uint index) const { return m_i->m_Components[index]; }
const MaterialComponentReflection* Material::getComponentReflection() const { return static_cast<MaterialComponentReflection*>(m_i->m_Components[(uint)MATERIAL_COMPONENT_TEXTURE_REFLECTION]); }
const bool Material::shadeless() const { return m_i->m_Shadeless; }
const float Material::glow() const { return m_i->m_BaseGlow; }
const float Material::specularity() const { return m_i->m_SpecularityPower; }
const uint Material::lightingMode() const { return m_i->m_LightingMode; }
const uint Material::id() const { return m_i->m_ID; }

void Material::setShadeless(bool b){ m_i->_setShadeless(b); }
void Material::setGlow(float f){ m_i->_setBaseGlow(f); }
void Material::setSpecularity(float s){ m_i->_setSpecularity(s); }
void Material::setLightingMode(uint m){ m_i->_setLightingMode(m); }