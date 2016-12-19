#include <unordered_map>
#include "Material.h"
#include "Texture.h"
#include "Engine_Resources.h"
#include "ShaderProgram.h"

using namespace Engine;

std::vector<glm::vec4> Material::m_MaterialProperities;

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
void MaterialComponent::bind(GLuint s,uint a){
	std::vector<uint>& slots = Material::MATERIAL_TEXTURE_SLOTS_MAP[(uint)m_ComponentType];
    if(a == ENGINE_RENDERING_API_OPENGL){
        std::string textureTypeName = MATERIAL_COMPONENT_SHADER_TEXTURE_NAMES[m_ComponentType];
		for(uint i = 0; i < slots.size(); i++){
			glActiveTexture(GL_TEXTURE0 + slots.at(i));
			glBindTexture(m_Texture->type(), m_Texture->address());
			glUniform1i(glGetUniformLocation(s, textureTypeName.c_str()), slots.at(i));
		}
    }
    else if(a == ENGINE_RENDERING_API_DIRECTX){
    }
}
void MaterialComponent::unbind(GLuint shader,uint api){
	std::vector<uint>& slots = Material::MATERIAL_TEXTURE_SLOTS_MAP[(uint)m_ComponentType];
	for(uint i = 0; i < slots.size(); i++){
		if(api == ENGINE_RENDERING_API_OPENGL){
			std::string textureTypeName = MATERIAL_COMPONENT_SHADER_TEXTURE_NAMES[m_ComponentType];
			glActiveTexture(GL_TEXTURE0 + slots.at(i));
			glBindTexture(m_Texture->type(), 0);
			glUniform1i(glGetUniformLocation(shader, textureTypeName.c_str()), slots.at(i));
		}
		else if(api == ENGINE_RENDERING_API_DIRECTX){
		}
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
void MaterialComponentReflection::bind(GLuint shader,uint api){
	std::vector<uint>& slots = Material::MATERIAL_TEXTURE_SLOTS_MAP[(uint)m_ComponentType];
    if(api == ENGINE_RENDERING_API_OPENGL){
        std::string textureTypeName = MATERIAL_COMPONENT_SHADER_TEXTURE_NAMES[m_ComponentType];
		glUniform1f(glGetUniformLocation(shader, "CubemapMixFactor"), m_MixFactor);

		glActiveTexture(GL_TEXTURE0 + slots.at(0));
        glBindTexture(m_Texture->type(), m_Texture->address());
		glUniform1i(glGetUniformLocation(shader, textureTypeName.c_str()), slots.at(0));

		glActiveTexture(GL_TEXTURE0 + slots.at(1));
		glBindTexture(m_Map->type(), m_Map->address());
		glUniform1i(glGetUniformLocation(shader, (textureTypeName + "Map").c_str()), slots.at(1));
    }
    else if(api == ENGINE_RENDERING_API_DIRECTX){
    }
}
void MaterialComponentReflection::unbind(GLuint shader,uint api){
	std::vector<uint>& slots = Material::MATERIAL_TEXTURE_SLOTS_MAP[(uint)m_ComponentType];
    if(api == ENGINE_RENDERING_API_OPENGL){
		std::string textureTypeName = MATERIAL_COMPONENT_SHADER_TEXTURE_NAMES[m_ComponentType];
		glActiveTexture(GL_TEXTURE0 + slots.at(0));
		glBindTexture(m_Texture->type(), 0);
		glUniform1i(glGetUniformLocation(shader, textureTypeName.c_str()), slots.at(0));

		glActiveTexture(GL_TEXTURE0 + slots.at(1));
		glBindTexture(m_Map->type(), 0);
		glUniform1i(glGetUniformLocation(shader, (textureTypeName + "Map").c_str()), slots.at(1));
	}
    else if(api == ENGINE_RENDERING_API_DIRECTX){
    }
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
void MaterialComponentRefraction::bind(GLuint shader,uint api){
	std::vector<uint>& slots = Material::MATERIAL_TEXTURE_SLOTS_MAP[(uint)m_ComponentType];
    if(api == ENGINE_RENDERING_API_OPENGL){
        std::string textureTypeName = MATERIAL_COMPONENT_SHADER_TEXTURE_NAMES[m_ComponentType];

		glUniform1f(glGetUniformLocation(shader, "CubemapMixFactor"), m_MixFactor);
		glUniform1f(glGetUniformLocation(shader, "RefractionRatio"), m_RefractionRatio);

		glActiveTexture(GL_TEXTURE0 + slots.at(0));
        glBindTexture(m_Texture->type(), m_Texture->address());
		glUniform1i(glGetUniformLocation(shader, textureTypeName.c_str()), slots.at(0));

		glActiveTexture(GL_TEXTURE0 + slots.at(1));
		glBindTexture(m_Map->type(), m_Map->address());
		glUniform1i(glGetUniformLocation(shader, (textureTypeName + "Map").c_str()), slots.at(1));
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
			_addComponent(MATERIAL_COMPONENT_TYPE_DIFFUSE,diffuse);
			_addComponent(MATERIAL_COMPONENT_TYPE_NORMAL,normal);
			_addComponent(MATERIAL_COMPONENT_TYPE_GLOW,glow);
			_addComponent(MATERIAL_COMPONENT_TYPE_SPECULAR,specular);
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
        void _addComponentReflection(Texture* cubemap,Texture* map,float mixFactor){
			uint type = (uint)MATERIAL_COMPONENT_TYPE_REFLECTION;
			if((m_Components.count(type) && m_Components[type] != nullptr) || (cubemap == nullptr || map == nullptr))
                return;
            m_Components[type] = new MaterialComponentReflection(type,cubemap,map,mixFactor);
        }
        void _addComponentRefraction(Texture* cubemap,Texture* map,float mixFactor,float ratio){
			uint type = (uint)MATERIAL_COMPONENT_TYPE_REFRACTION;
			if((m_Components.count(type) && m_Components[type] != nullptr) || (cubemap == nullptr || map == nullptr))
                return;
            m_Components[type] = new MaterialComponentRefraction(type,cubemap,map,mixFactor,ratio);
        }
        void _setShadeless(bool& b){ m_Shadeless = b; _updateGlobalMaterialPool(); }
        void _setBaseGlow(float& f){ m_BaseGlow = f; _updateGlobalMaterialPool(); }
        void _setSpecularity(float& s){ m_SpecularityPower = s; _updateGlobalMaterialPool(); }
        void _setLightingMode(uint& m){ m_LightingMode = m; _updateGlobalMaterialPool(); }
};

Material::Material(std::string diffuse, std::string normal, std::string glow,std::string specular,std::string program):m_i(new impl()){
    m_i->_init(diffuse,normal,glow,specular);
	if(program == "") program = "Deferred";
	Resources::getShaderProgram(program)->addMaterial(this);
}
Material::Material(Texture* diffuse,Texture* normal,Texture* glow,Texture* specular,ShaderP* program):m_i(new impl()){
    m_i->_init(diffuse,normal,glow,specular);
	if(program == nullptr) program = Resources::getShaderProgram("Deferred");
	program->addMaterial(this);
}
Material::~Material(){
    m_i->_destruct();
}
void Material::addComponent(uint type, Texture* texture){
    m_i->_addComponent(type,texture);
}
void Material::addComponentDiffuse(Texture* texture){
	m_i->_addComponent((uint)MATERIAL_COMPONENT_TYPE_DIFFUSE,texture);
}
void Material::addComponentDiffuse(std::string& textureFile){
    Texture* texture = Resources::getTexture(textureFile); 
    if(texture == nullptr && textureFile != "") texture = new Texture(textureFile);
	m_i->_addComponent((uint)MATERIAL_COMPONENT_TYPE_DIFFUSE,texture);
}

void Material::addComponentNormal(Texture* texture){
	m_i->_addComponent((uint)MATERIAL_COMPONENT_TYPE_NORMAL,texture);
}
void Material::addComponentNormal(std::string& textureFile){
    Texture* texture = Resources::getTexture(textureFile); 
    if(texture == nullptr && textureFile != "") texture = new Texture(textureFile);
	m_i->_addComponent((uint)MATERIAL_COMPONENT_TYPE_NORMAL,texture);
}
void Material::addComponentSpecular(Texture* texture){
	m_i->_addComponent((uint)MATERIAL_COMPONENT_TYPE_SPECULAR,texture);
}
void Material::addComponentSpecular(std::string& textureFile){
    Texture* texture = Resources::getTexture(textureFile); 
    if(texture == nullptr && textureFile != "") texture = new Texture(textureFile);
	m_i->_addComponent((uint)MATERIAL_COMPONENT_TYPE_SPECULAR,texture);
}
void Material::addComponentReflection(Texture* cubemap,Texture* map,float mixFactor){
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
    if(cubemap == nullptr && cubemapName != "") cubemap = new Texture(cubemapName);

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



void Material::bind(GLuint shader,GLuint api){
	glm::vec3 first(0.0f);
	glm::vec3 second(0.0f);
	for(uint i = 0; i < MATERIAL_COMPONENT_TYPE_NUMBER; i++){
		MATERIAL_COMPONENT_TYPE type = (MATERIAL_COMPONENT_TYPE)i;
		if(m_i->m_Components.count(type)){
			MaterialComponent* c = m_i->m_Components[type];
			if(c->texture() != nullptr && c->texture()->address() != 0){
				//enable
				if     (i == 0){ first.x = 1.0f; }
				else if(i == 1){ first.y = 1.0f; }
				else if(i == 2){ first.z = 1.0f; }
				else if(i == 3){ second.x = 1.0f; }
				else if(i == 4){ second.y = 1.0f; }
				else if(i == 5){ second.z = 1.0f; }
				c->bind(shader,api);
			}
			else{
				c->unbind(shader,api);
			}
		}
	}
	glUniform3f(glGetUniformLocation(shader,"FirstConditionals"), first.x,first.y,first.z);
	glUniform3f(glGetUniformLocation(shader,"SecondConditionals"), second.x,second.y,second.z);
}

std::unordered_map<uint,MaterialComponent*>& Material::getComponents(){ return m_i->m_Components; }
const MaterialComponent* Material::getComponent(uint index) const { return m_i->m_Components[index]; }
const MaterialComponentReflection* Material::getComponentReflection() const { return static_cast<MaterialComponentReflection*>(m_i->m_Components[(uint)MATERIAL_COMPONENT_TYPE_REFLECTION]); }
const MaterialComponentRefraction* Material::getComponentRefraction() const { return static_cast<MaterialComponentRefraction*>(m_i->m_Components[(uint)MATERIAL_COMPONENT_TYPE_REFRACTION]); }
const bool Material::shadeless() const { return m_i->m_Shadeless; }
const float Material::glow() const { return m_i->m_BaseGlow; }
const float Material::specularity() const { return m_i->m_SpecularityPower; }
const uint Material::lightingMode() const { return m_i->m_LightingMode; }
const uint Material::id() const { return m_i->m_ID; }

void Material::setShadeless(bool b){ m_i->_setShadeless(b); }
void Material::setGlow(float f){ m_i->_setBaseGlow(f); }
void Material::setSpecularity(float s){ m_i->_setSpecularity(s); }
void Material::setLightingMode(uint m){ m_i->_setLightingMode(m); }