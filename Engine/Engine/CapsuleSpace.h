#ifndef CAPSULE_SPACE_H
#define CAPSULE_SPACE_H

#include "SolarSystem.h"
#include "ObjectDisplay.h"

class PointLight;

class CapsuleStar final: public ObjectDisplay{
		PointLight* m_Light;
	public:
		CapsuleStar(float size,glm::v3 pos, std::string name, Scene* = nullptr);
		~CapsuleStar();

		void update(float);
		void draw(GLuint shader,bool=false);
};

class CapsuleTunnel final: public ObjectDisplay{
	private:
		float m_TunnelRadius;
	public:
		CapsuleTunnel(float tunnelRadius, std::string name, std::string material, Scene* = nullptr);
		~CapsuleTunnel();

		float getTunnelRadius(){ return m_TunnelRadius; }
		void draw(GLuint shader,bool=false);
};
class CapsuleRibbon final: public ObjectDisplay{
	private:
		float m_TunnelRadius;
	public:
		CapsuleRibbon(float tunnelRadius, std::string name, std::string material, Scene* = nullptr);
		~CapsuleRibbon();

		float getTunnelRadius(){ return m_TunnelRadius; }
		void draw(GLuint shader,bool=false);
};

class CapsuleSpace final: public SolarSystem{
	private:
		CapsuleTunnel* m_TunnelA;
		CapsuleTunnel* m_TunnelB;
		CapsuleRibbon* m_Ribbon;

		std::vector<CapsuleStar*> m_CapsuleStars;
	public:
		CapsuleSpace();
		~CapsuleSpace();

		void update(float);
};
#endif