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
		float m_BaseGlow;
		float m_Specularity;
	public:
		Material(Texture* diffuse,Texture* normal = nullptr,Texture* glow = nullptr);
		Material(std::string diffuse,std::string normal="",std::string glow="");
		~Material();

		std::unordered_map<unsigned int,Texture*>& getComponents(){ return m_Components; }
		Texture* getComponent(unsigned int i){ return m_Components[i]; }
		void addComponent(unsigned int, std::string file);

		void bindTexture(unsigned int,GLuint shader);

		const bool getShadeless() const { return m_Shadeless; }
		const float getBaseGlow() const { return m_BaseGlow; }
		const float getSpecularity() const { return m_Specularity; }
		void setShadeless(bool b){ m_Shadeless = b; }
		void setBaseGlow(float f){ m_BaseGlow = f; }
		void setSpecularity(float s){ m_Specularity = s; }
};
#endif