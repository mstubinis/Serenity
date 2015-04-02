#ifndef PLANET_H
#define PLANET_H

#include "Object.h"

class Planet: public Object{
	protected:
		float m_AtmosphereHeight;
	public:
		Planet(
				std::string = "",
			    glm::vec3 = glm::vec3(0,0,0),   //Position
			    glm::vec3 = glm::vec3(1,1,1),   //Scale
			    glm::vec3 = glm::vec3(0,0,0),   //Rotation
			    std::string = "Planet",         //Object
				bool addToResources = true      //Add this to the resource generic object pool (Don't do for lights)
			  );
		~Planet();

		virtual void Update(float);
		virtual void Render(Mesh*, Material*,RENDER_TYPE);
		virtual void Render(RENDER_TYPE = RENDER_TYPE_FORWARD);
};
#endif