#include "Material.h"
#include "Texture.h"
#include "Engine_Resources.h"

Material::Material(Texture* diffuse,Texture* normal,Texture* glow){
	for(unsigned int i = 0; i < MATERIAL_COMPONENT_TYPE_NUMBER; i++){
		m_Components[i] = nullptr;
	}

	if(diffuse == nullptr)m_Components[MATERIAL_COMPONENT_TEXTURE_DIFFUSE] = new Texture("");
	else                  m_Components[MATERIAL_COMPONENT_TEXTURE_DIFFUSE] = diffuse;

	if(normal == nullptr) m_Components[MATERIAL_COMPONENT_TEXTURE_NORMAL] = new Texture("");
	else                  m_Components[MATERIAL_COMPONENT_TEXTURE_NORMAL] = normal;

	if(glow == nullptr)   m_Components[MATERIAL_COMPONENT_TEXTURE_GLOW] = new Texture("");
	else                  m_Components[MATERIAL_COMPONENT_TEXTURE_GLOW] = glow;

	m_Shadeless = false;
	m_BaseGlow = 0.0f;
	m_Specularity = 1.0f;
	m_LightingMode = MATERIAL_LIGHTING_MODE_BLINNPHONG;
}
Material::Material(std::string diffuse, std::string normal, std::string glow){
	for(unsigned int i = 0; i < MATERIAL_COMPONENT_TYPE_NUMBER; i++){
		m_Components[i] = nullptr;
	}
	m_Components[MATERIAL_COMPONENT_TEXTURE_DIFFUSE] = new Texture(diffuse);
	m_Components[MATERIAL_COMPONENT_TEXTURE_NORMAL] = new Texture(normal);
	m_Components[MATERIAL_COMPONENT_TEXTURE_GLOW] = new Texture(glow);
	m_Shadeless = false;
	m_BaseGlow = 0.0f;
	m_Specularity = 1.0f;
	m_LightingMode = MATERIAL_LIGHTING_MODE_BLINNPHONG;
}
Material::~Material(){

}
void Material::addComponent(unsigned int type, std::string file){
	if(m_Components[type] != nullptr)
		return;
	m_Components[type] = new Texture(file);
}
void Material::bindTexture(unsigned int component,GLuint shader){
	if(m_Components[component] == nullptr)
		return;

	std::string textureTypeName = MATERIAL_COMPONENT_SHADER_TEXTURE_NAMES[component];
	if(m_Components[component]->getTextureAddress() == 0){//Texture / Material type not present; disable this material
		glUniform1i(glGetUniformLocation(shader,(textureTypeName+"Enabled").c_str()), 0);
		return;
	}
	glEnable(m_Components[component]->getTextureType()); //may not need this line, consider cpu cost
	glActiveTexture(GL_TEXTURE0 + component);
	glBindTexture(m_Components[component]->getTextureType(), m_Components[component]->getTextureAddress());
	glUniform1i(glGetUniformLocation(shader, textureTypeName.c_str()), component);
	glUniform1i(glGetUniformLocation(shader,(textureTypeName+"Enabled").c_str()), 1);
}