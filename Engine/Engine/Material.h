#ifndef MATERIAL_H
#define MATERIAL_H

#include "MaterialComponent.h"
#include <string>

class Material{
	private:
		std::unordered_map<MaterialComponents::MATERIAL_COMPONENT_TYPE,MaterialComponents::MaterialComponent*> m_Components;
	public:
		Material(std::string diffuse,std::string normal="",std::string glow="");
		~Material();

		std::unordered_map<MaterialComponents::MATERIAL_COMPONENT_TYPE,MaterialComponents::MaterialComponent*>& Components();
		MaterialComponents::MaterialComponent* Get_Component(MaterialComponents::MATERIAL_COMPONENT_TYPE);
		void Add_Component(MaterialComponents::MATERIAL_COMPONENT_TYPE, std::string file);

		void Bind_Texture(MaterialComponents::MaterialComponent*);
};
#endif