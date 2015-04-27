#ifndef LIGHT_H
#define LIGHT_H
#include "ObjectDisplay.h"

enum LIGHT_TYPE {LIGHT_TYPE_SUN,LIGHT_TYPE_POINT,LIGHT_TYPE_DIRECTIONAL, LIGHT_TYPE_SPOT};
class Scene;
class SunLight: public ObjectDisplay{
	protected:
		unsigned int m_Type;
		float m_AmbientIntensity, m_DiffuseIntensity;
	public:
		SunLight(glm::vec3 = glm::vec3(0,0,0),std::string = "Sun Light",unsigned int=LIGHT_TYPE_SUN,Scene* = nullptr);
		~SunLight();

		void update(float);
		virtual void render(GLuint);
		void renderDebug(GLuint);
		void setName(std::string);
};
class DirectionalLight: public SunLight{
	private:
        glm::vec3 m_Direction;
	public:
		DirectionalLight(glm::vec3 = glm::vec3(0,0,-1), Scene* = nullptr);
		~DirectionalLight();

		void render(GLuint);
};
class PointLight: public SunLight{
	protected:
        float m_Constant, m_Linear, m_Exp;

	public:
		PointLight(glm::vec3 = glm::vec3(0,0,0), Scene* = nullptr);
		~PointLight();

		void render(GLuint);
};
class SpotLight: public SunLight{
	private:
		glm::vec3 m_Direction;
		float m_Cutoff;

	public:
		SpotLight(glm::vec3, Scene* = nullptr);
		~SpotLight();

		void render(GLuint);
};
#endif