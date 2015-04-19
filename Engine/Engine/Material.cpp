#include "Material.h"

using namespace MaterialComponents;

Material::Material(std::string diffuse, std::string normal, std::string glow){
	for(unsigned int i = 0; i < MATERIAL_COMPONENT_TYPE_NUMBER; i++){
		m_Components[i] = nullptr;
	}
	m_Components[MATERIAL_COMPONENT_TEXTURE_DIFFUSE] = new MaterialComponent(MATERIAL_COMPONENT_TEXTURE_DIFFUSE,diffuse);
	m_Components[MATERIAL_COMPONENT_TEXTURE_NORMAL] = new MaterialComponent(MATERIAL_COMPONENT_TEXTURE_NORMAL,normal);
	m_Components[MATERIAL_COMPONENT_TEXTURE_GLOW] = new MaterialComponent(MATERIAL_COMPONENT_TEXTURE_GLOW,glow);
	m_Shadeless = false;
}
Material::~Material(){
	for(auto component:m_Components)
		delete component.second;
}
void Material::addComponent(unsigned int type, std::string file){
	if(m_Components[type] != nullptr)
		return;
	m_Components[type] = new MaterialComponent(type,file);
}
MaterialComponent* Material::getComponent(unsigned int type){ return m_Components[type]; }
std::unordered_map<unsigned int,MaterialComponent*>& Material::getComponents(){ return m_Components; }
void Material::bindTexture(MaterialComponents::MaterialComponent* component,GLuint shader){
	if(component == nullptr)
		return;

	std::string textureTypeName = MATERIAL_COMPONENT_SHADER_TEXTURE_NAMES[component->getType()];
	if(component->getTexture() == 0){//Texture / Material type not present; disable this material
		glUniform1i(glGetUniformLocation(shader,(textureTypeName+"Enabled").c_str()), 0);
		return;
	}
	glEnable(component->getTextureType()); //may not need this line, consider cpu cost
	glActiveTexture(GL_TEXTURE0 + component->getType());
	glBindTexture(component->getTextureType(), component->getTexture());
	glUniform1i(glGetUniformLocation(shader, textureTypeName.c_str()), component->getType());
	glUniform1i(glGetUniformLocation(shader,(textureTypeName+"Enabled").c_str()), 1);
}