#ifndef STATION_H
#define STATION_H

#include "ObjectDynamic.h"

class Station: public ObjectDynamic{
	private:

	protected:

	public:
		Station(std::string = "",
			    std::string = "",
			    glm::dvec3 = glm::dvec3(0),           //Position
				glm::vec3 = glm::vec3(1),             //Scale
				std::string = "Station",              //Object name
				Engine::Physics::Collision* = nullptr,//Bullet Collision Shape
				Scene* = nullptr
			   );
		virtual ~Station();

		void update(float);
		void render();
};
#endif