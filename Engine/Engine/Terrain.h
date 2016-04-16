#pragma once
#ifndef ENGINE_TERRAIN_H
#define ENINGE_TERRAIN_H

#include "ObjectDynamic.h"
#include <SFML/System.hpp>

namespace sf{ 
	class Image;
}
class Scene;
class btRigidBody;

class Terrain final: public ObjectDynamic{
	private:
		std::vector<float> m_Pixels;
		btRigidBody* m_RigidBody;
	public:
		Terrain(std::string name,sf::Image& img,std::string material,Scene* = nullptr);
		virtual ~Terrain();

        virtual void setPosition(glm::nType,glm::nType,glm::nType); 
        virtual void setPosition(glm::v3);

        virtual void setScale(float,float,float); 
        virtual void setScale(glm::vec3);

		virtual void update(float);
		virtual void render();
};
#endif