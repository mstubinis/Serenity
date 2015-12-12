#ifndef PLANET_H
#define PLANET_H

#include "ObjectDisplay.h"

class SunLight;
class Ring;
enum PlanetType { PLANET_TYPE_ROCKY, 
	              PLANET_TYPE_ICE,
				  PLANET_TYPE_GAS_GIANT, 
				  PLANET_TYPE_ICE_GIANT,
				  PLANET_TYPE_MOON, 
				  PLANET_TYPE_STAR, 
				  PLANET_TYPE_ASTEROID };

struct RingInfo{
	unsigned int position;
	unsigned int size;
	unsigned int alphaBreakpoint;
	glm::vec3 color;
	RingInfo(unsigned int p, unsigned int s, glm::vec3 col,unsigned int ab = 1){ 
		position = p; 
		size = s; 
		color = col; 
		alphaBreakpoint = ab; 
	}
};

class Planet: public ObjectDisplay{
	protected:
		std::vector<Ring*> m_Rings;
		PlanetType m_Type;
		float m_AtmosphereHeight;
	public:
		Planet(
				std::string = "",               //Material
				PlanetType = PLANET_TYPE_ROCKY, //Type
			    glm::vec3 = glm::vec3(0),       //Position
			    float = 1,                      //Radius
			    std::string = "Planet",         //Name
				float = 0,                      //Atmosphere size
				Scene* = nullptr
			  );
		~Planet();

		const glm::vec3& getRadiusBox() const { return m_BoundingBoxRadius + (m_BoundingBoxRadius*m_AtmosphereHeight); }
		const float getRadius() const { return m_Radius + (m_Radius * m_AtmosphereHeight); }

		void addRing(Ring*);

		void update(float);
		virtual void render(Mesh*, Material*,GLuint,bool=false);
		virtual void draw(Mesh*, Material*,GLuint,bool=false);
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
		void render(Mesh*, Material*,GLuint,bool=false);
		void draw(Mesh*, Material*,GLuint,bool=false);
};

class Ring{
	private:
		Material* material;
		Planet* m_Parent;
		void _makeRingImage(std::vector<RingInfo>,Planet*);
	public:
		Ring(std::vector<RingInfo>,Planet*);
		~Ring();

		void update(float);
		void draw(GLuint shader);
};
#endif