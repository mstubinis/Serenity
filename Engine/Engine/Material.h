#ifndef MATERIAL_H
#define MATERIAL_H

#include "MaterialComponent.h"
#include <string>

class Material{
	private:
		std::unordered_map<unsigned int,MaterialComponents::MaterialComponent*> m_Components;
		bool m_Shadeless;
	public:
		Material(std::string diffuse,std::string normal="",std::string glow="");
		~Material();

		std::unordered_map<unsigned int,MaterialComponents::MaterialComponent*>& Components();
		MaterialComponents::MaterialComponent* Get_Component(unsigned int);
		void Add_Component(unsigned int, std::string file);

		void Bind_Texture(MaterialComponents::MaterialComponent*,GLuint shader);

		bool Shadeless();
		void Set_Shadeless(bool);
};
#endif