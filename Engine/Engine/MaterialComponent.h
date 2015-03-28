#ifndef MATERIALCOMPONENT_H
#define MATERIALCOMPONENT_H

#include <unordered_map>
#include <GL\glew.h>
#include <SFML\OpenGL.hpp>

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
			MATERIAL_COMPONENT_TYPE m_Type;
		public:
			MaterialComponent(MATERIAL_COMPONENT_TYPE,std::string file);
			~MaterialComponent();

			GLuint Texture() const;
			GLuint TextureType() const;
			MATERIAL_COMPONENT_TYPE Type() const;

	};
};
#endif