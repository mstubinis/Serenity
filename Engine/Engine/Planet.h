#ifndef PLANET_H
#define PLANET_H

#include "Object.h"

enum PlanetType { PLANET_TYPE_ROCKY, PLANET_TYPE_GAS_GIANT, PLANET_TYPE_MOON, PLANET_TYPE_STAR };
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
				bool addToResources = true      //Add this to the resource generic object pool (Don't do for lights)
			  );
		~Planet();

		virtual void Update(float);
		virtual void Render(Mesh*, Material*,bool=false);
		virtual void Render(bool=false);
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
				bool addToResources = true      //Add this to the resource generic object pool (Don't do for lights)
			);
		~Star();

		void Render(Mesh*, Material*,bool=false);
		void Render(bool=false);
};
#endif