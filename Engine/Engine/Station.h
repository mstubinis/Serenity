#ifndef STATION_H
#define STATION_H

#include "ObjectDynamic.h"

class Station: public ObjectDynamic{
	private:

	protected:

	public:
		Station(std::string = "",
			    std::string = "",
			    glm::v3 = glm::v3(0),                 //Position
				glm::vec3 = glm::vec3(1),             //Scale
				std::string = "Station",              //Object name
				Engine::Physics::Collision* = nullptr,//Bullet Collision Shape
				Scene* = nullptr
			   );
		virtual ~Station();

		glm::vec2 getGravityInfo(){ return glm::vec2(this->getRadius()*100,this->getRadius()*150); }

		void update(float);
		void render();
};
#endif