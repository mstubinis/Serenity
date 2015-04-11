#include "MaterialComponent.h"
#include "Engine_Resources.h"
#include "ShaderProgram.h"
using namespace MaterialComponents;

MaterialComponent::MaterialComponent(unsigned int type, std::string file){
	m_Type = type;
	m_Texture = 0;
	if(type == MATERIAL_COMPONENT_TEXTURE_DIFFUSE || type == MATERIAL_COMPONENT_TEXTURE_NORMAL || type == MATERIAL_COMPONENT_TEXTURE_GLOW){
		m_TextureType = GL_TEXTURE_2D;
	}

	if(m_TextureType == GL_TEXTURE_2D && file != "")
		Resources->Load_Texture_Into_GLuint(m_Texture,file);
}
MaterialComponent::~MaterialComponent(){
	glDeleteTextures(1,&m_Texture);
}
