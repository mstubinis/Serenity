#ifndef PLANET_H
#define PLANET_H

#include "Object.h"

enum PlanetType { PLANET_TYPE_ROCKY, PLANET_TYPE_GAS_GIANT, PLANET_TYPE_MOON, PLANET_TYPE_STAR, PLANET_TYPE_ASTEROID };
class SunLight;
class Planet: public Object{
	protected:
		PlanetType m_Type;
		float m_AtmosphereHeight;
	public:
		Planet(
				std::string = "",               //Material
				PlanetType = PLANET_TYPE_ROCKY, //Type
			    glm::vec3 = glm::vec3(0,0,0),   //Position
			    float = 1,                      //Scale
			    std::string = "Planet",         //Name
				float = 0,                      //Atmosphere size
				Scene* = nullptr
			  );
		~Planet();

		const glm::vec3& getRadiusBox() const { return m_BoundingBoxRadius + (m_BoundingBoxRadius*m_AtmosphereHeight); }
		const float getRadius() const { return m_Radius + (m_Radius * m_AtmosphereHeight); }

		virtual void update(float);
		virtual void render(Mesh*, Material*,bool=false);
		virtual void render(bool=false);
};

class Star: public Planet{
	private:
		SunLight* m_Light;
	public:
		Star(
				glm::vec3 = glm::vec3(1,1,0),   //Star Color
				glm::vec3 = glm::vec3(1,1,1),   //Sunlight Color
			    glm::vec3 = glm::vec3(0,0,0),   //Position
			    float = 1,					    //Scale
			    std::string = "Star",           //Name
				Scene* = nullptr
			);
		~Star();

		void render(Mesh*, Material*,bool=false);
		void render(bool=false);
};
#endif