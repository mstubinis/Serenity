#ifndef MATERIALCOMPONENT_H
#define MATERIALCOMPONENT_H

#include <unordered_map>
#include <GL/glew.h>
#include <GL/GL.h>

class Material;

namespace MaterialComponents{
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
	class MaterialComponent{
		private:
			GLuint m_Texture;
			GLuint m_TextureType;
			unsigned int m_Type;
		public:
			MaterialComponent(unsigned int,std::string file);
			~MaterialComponent();

			const GLuint Texture() const{ return m_Texture; }
			const GLuint TextureType() const{ return m_TextureType; }
			const unsigned int Type() const{ return m_Type; }
	};
};
#endif