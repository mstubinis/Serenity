#ifndef PLANET_H
#define PLANET_H

#include "ObjectDisplay.h"

class SunLight;
class Ring;

enum PlanetType { 
	PLANET_TYPE_ROCKY, 
	PLANET_TYPE_ICE,
	PLANET_TYPE_GAS_GIANT, 
	PLANET_TYPE_ICE_GIANT,
	PLANET_TYPE_MOON, 
	PLANET_TYPE_STAR, 
	PLANET_TYPE_ASTEROID 
};

struct RotationInfo final{
	float tilt;
	float days;
	RotationInfo(float _tilt, float _days){
		tilt = _tilt;
		days = _days;
	}
};

struct OrbitInfo final{
	float eccentricity;
	float days;
	float minorRadius; float majorRadius;
	glm::nType angle;
	Object* parent;
	OrbitInfo(float _eccentricity, float _days, float _majorRadius,glm::nType _angle,Object* _parent){
		angle = _angle;
		eccentricity = _eccentricity;
		days = _days;
		majorRadius = _majorRadius;
		minorRadius = glm::sqrt(majorRadius*majorRadius*(1 - (eccentricity*eccentricity))); //b² = a²(1 - e²)
		parent = _parent;
	}
};

struct RingInfo final{
	unsigned int position;
	unsigned int size;
	unsigned int alphaBreakpoint;
	glm::uvec3 color;
	RingInfo(unsigned int p, unsigned int s, glm::uvec3 col,unsigned int ab = 1){ 
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
		OrbitInfo* m_OrbitInfo;
		RotationInfo* m_RotationInfo;
		float m_AtmosphereHeight;
	public:
		Planet(
				std::string = "",               //Material
				PlanetType = PLANET_TYPE_ROCKY, //Type
			    glm::v3 = glm::v3(0),           //Position
			    glm::nType = 1,                 //Radius
			    std::string = "Planet",         //Name
				float = 0,                      //Atmosphere size
				Scene* = nullptr
			  );
		virtual ~Planet();

		glm::vec2 getGravityInfo(){ return glm::vec2(this->getRadius()*5,this->getRadius()*7); }

		const glm::vec3& getRadiusBox() const { return m_BoundingBoxRadius + (m_BoundingBoxRadius*m_AtmosphereHeight); }
		const float getRadius() const { return m_Radius + (m_Radius * m_AtmosphereHeight); }

		void addRing(Ring*);

		void setOrbit(OrbitInfo* o){ 
			m_OrbitInfo = o; 
			update(0);
		}
		void setRotation(RotationInfo* r){ 
			m_RotationInfo = r;
			rotate(0,0,-m_RotationInfo->tilt,false);
		}

		void update(float);
		virtual void render(GLuint,bool=false);
		virtual void draw(GLuint,bool=false);
};

class Star: public Planet{
	private:
		SunLight* m_Light;
	public:
		Star(
				glm::vec3 = glm::vec3(1,1,0),   //Star Color
				glm::vec3 = glm::vec3(1,1,1),   //Sunlight Color
			    glm::v3 = glm::v3(0),           //Position
			    glm::nType = 1,					//Scale
			    std::string = "Star",           //Name
				Scene* = nullptr
			);
		virtual ~Star();
		void render(GLuint,bool=false);
		void draw(GLuint,bool=false);
};

class Ring final{
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