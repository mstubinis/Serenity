#ifndef PLANET_H
#define PLANET_H

#include "Object.h"

enum PlanetType { PLANET_TYPE_ROCKY, PLANET_TYPE_GAS_GIANT, PLANET_TYPE_MOON };
class Planet: public Object{
	protected:
		PlanetType m_Type;
		float m_AtmosphereHeight;
	public:
		Planet(
				std::string = "",               //Material
				PlanetType = PLANET_TYPE_ROCKY, //Type
			    glm::vec3 = glm::vec3(0,0,0),   //Position
			    glm::vec3 = glm::vec3(1,1,1),   //Scale
			    std::string = "Planet",         //Object
				bool addToResources = true      //Add this to the resource generic object pool (Don't do for lights)
			  );
		~Planet();

		virtual void Update(float);
		virtual void Render(Mesh*, Material*,bool=false);
		virtual void Render(bool=false);
};
#endif