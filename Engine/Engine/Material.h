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

		std::unordered_map<unsigned int,MaterialComponents::MaterialComponent*>& getComponents();
		MaterialComponents::MaterialComponent* getComponent(unsigned int);
		void addComponent(unsigned int, std::string file);

		void bindTexture(MaterialComponents::MaterialComponent*,GLuint shader);

		const bool getShadeless() const { return m_Shadeless; }
		void setShadeless(bool b){ m_Shadeless = b; }
};
#endif