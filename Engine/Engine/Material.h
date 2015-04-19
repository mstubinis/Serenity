#ifndef MATERIAL_H
#define MATERIAL_H

#include <string>
#include <unordered_map>
#include <GL/glew.h>
#include <GL/GL.h>

enum MATERIAL_COMPONENT_TYPE{
	MATERIAL_COMPONENT_TEXTURE_DIFFUSE,
	MATERIAL_COMPONENT_TEXTURE_NORMAL,
	MATERIAL_COMPONENT_TEXTURE_GLOW,
	MATERIAL_COMPONENT_TYPE_NUMBER
};
static GLchar* MATERIAL_COMPONENT_SHADER_TEXTURE_NAMES[MATERIAL_COMPONENT_TYPE_NUMBER] = {
	"DiffuseMap",
	"NormalMap",
	"GlowMap"
};
class Texture;
class Material{
	private:
		std::unordered_map<unsigned int,Texture*> m_Components;
		bool m_Shadeless;
	public:
		Material(std::string diffuse,std::string normal="",std::string glow="");
		~Material();

		std::unordered_map<unsigned int,Texture*>& getComponents(){ return m_Components; }
		Texture* getComponent(unsigned int i){ return m_Components[i]; }
		void addComponent(unsigned int, std::string file);

		void bindTexture(unsigned int,GLuint shader);

		const bool getShadeless() const { return m_Shadeless; }
		void setShadeless(bool b){ m_Shadeless = b; }
};
#endif