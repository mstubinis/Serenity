#include "Material.h"
#include "Engine_Resources.h"

using namespace MaterialComponents;

Material::Material(std::string diffuse, std::string normal, std::string glow){
	for(unsigned int i = 0; i < MATERIAL_COMPONENT_TYPE_NUMBER; i++){
		MATERIAL_COMPONENT_TYPE type = (MATERIAL_COMPONENT_TYPE)i; 
		m_Components[type] = nullptr;
	}
	m_Components[MATERIAL_COMPONENT_TEXTURE_DIFFUSE] = new MaterialComponent(MATERIAL_COMPONENT_TEXTURE_DIFFUSE,diffuse);
	m_Components[MATERIAL_COMPONENT_TEXTURE_NORMAL] = new MaterialComponent(MATERIAL_COMPONENT_TEXTURE_NORMAL,normal);
	m_Components[MATERIAL_COMPONENT_TEXTURE_GLOW] = new MaterialComponent(MATERIAL_COMPONENT_TEXTURE_GLOW,glow);
}
Material::~Material(){
	for(auto component:m_Components)
		delete component.second;
}
void Material::Add_Component(MATERIAL_COMPONENT_TYPE type, std::string file){
	if(m_Components[type] != nullptr)
		return;
	m_Components[type] = new MaterialComponent(type,file);
}
MaterialComponent* Material::Get_Component(MATERIAL_COMPONENT_TYPE type){ return m_Components[type]; }
std::unordered_map<MATERIAL_COMPONENT_TYPE,MaterialComponent*>& Material::Components(){ return m_Components; }
void Material::Bind_Texture(MaterialComponents::MaterialComponent* component,GLuint shader){
	if(component == nullptr)
		return;

	std::string textureTypeName = MATERIAL_COMPONENT_SHADER_TEXTURE_NAMES[component->Type()];
	if(component->Texture() == 0){//Texture / Material type not present; disable this material
		glUniform1i(glGetUniformLocation(shader,(textureTypeName+"Enabled").c_str()), 0);
		return;
	}
	glEnable(component->TextureType()); //may not need this line, consider cpu cost
	glActiveTexture(GL_TEXTURE0 + component->Type());
	glBindTexture(component->TextureType(), component->Texture());
	glUniform1i(glGetUniformLocation(shader, textureTypeName.c_str()), component->Type());
	glUniform1i(glGetUniformLocation(shader,(textureTypeName+"Enabled").c_str()), 1);
}