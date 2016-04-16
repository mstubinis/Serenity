#pragma once
#ifndef ENGINE_ATMOSPHERE_H
#define ENGINE_ATMOSPHERE_H

#include "Skybox.h"
#include "Engine_Math.h"

struct AtmosphereInformation{
	public:
		float height;
		float Km;
		float Kr;
		float ESun;
		float scaledepth;
		float gravity;
		float exposure;

		glm::vec3 inverseWavelength;

		AtmosphereInformation(float _height = 0.025f,
			                  float _km = 0.0025f,
							  float _kr = 0.0015f,
							  float _esun = 20.0f)
		{ 
			height = _height;
			Km = _km;
			Kr = _kr;
			ESun = _esun;
			inverseWavelength = glm::vec3(1.f/glm::pow(0.65f,4.f),1.f/glm::pow(0.57f,4.f),1.f/glm::pow(0.475f,4.f));
			scaledepth = 0.25f;
			gravity = -0.98f;
			exposure = 2.0f;
		}
		~AtmosphereInformation(){}
};

class Atmosphere final: public SkyboxEmpty{
	private:
		AtmosphereInformation m_Atmosphere;
		Mesh* m_Mesh;
		glm::vec3 m_Position;
		glm::vec3 m_Scale;
		bool m_FollowCamera;
	public:
        Atmosphere(std::string name = "Atmosphere",Scene* = nullptr, bool followCamera = true);
        virtual ~Atmosphere();

		virtual float getRadius();
		virtual glm::v3 getPosition();

        virtual void update();
        virtual void render();
};
#endif