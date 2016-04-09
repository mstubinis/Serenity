#include "Material.h"
#include "Texture.h"
#include "Engine_Resources.h"
#include "Engine.h"

using namespace Engine;

Material::Material(Texture* diffuse,Texture* normal,Texture* glow){
    _init(diffuse,normal,glow);
}
Material::Material(std::string diffuse, std::string normal, std::string glow){
    Texture* diffuseT = Resources::getTexture(diffuse); 
    Texture* normalT = Resources::getTexture(normal); 
    Texture* glowT = Resources::getTexture(glow);
    if(diffuseT == nullptr) diffuseT = new Texture(diffuse);
    if(normalT == nullptr)  normalT = new Texture(normal);
    if(glowT == nullptr)    glowT = new Texture(glow);
    _init(diffuseT,normalT,glowT);
}
void Material::_init(Texture* diffuse, Texture* normal, Texture* glow){
    for(unsigned int i = 0; i < MATERIAL_COMPONENT_TYPE_NUMBER; i++)
        m_Components[i] = nullptr;
    m_Components[MATERIAL_COMPONENT_TEXTURE_DIFFUSE] = diffuse;
    m_Components[MATERIAL_COMPONENT_TEXTURE_NORMAL] = normal;
    m_Components[MATERIAL_COMPONENT_TEXTURE_GLOW] = glow;

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
void Material::bindTexture(unsigned int c,GLuint shader,unsigned int api){
	if(api == ENGINE_RENDERING_API_OPENGL){
		std::string textureTypeName = MATERIAL_COMPONENT_SHADER_TEXTURE_NAMES[c];
		if(m_Components[c] == nullptr || m_Components[c]->getTextureAddress() == 0){//Texture / Material type not present; disable this material
			glUniform1i(glGetUniformLocation(shader,(textureTypeName+"Enabled").c_str()), 0);
			return;
		}
		glActiveTexture(GL_TEXTURE0 + c);
		glBindTexture(m_Components[c]->getTextureType(), m_Components[c]->getTextureAddress());
		glUniform1i(glGetUniformLocation(shader, textureTypeName.c_str()), c);
		glUniform1i(glGetUniformLocation(shader,(textureTypeName+"Enabled").c_str()), 1);
	}
	else if(api == ENGINE_RENDERING_API_DIRECTX){
	}
}