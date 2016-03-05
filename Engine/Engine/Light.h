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
		SunLight(glm::v3 = glm::v3(0),std::string = "Sun Light",unsigned int=LIGHT_TYPE_SUN,Scene* = nullptr);
		virtual ~SunLight();

		void update(float);

		virtual void render(Mesh*, Material*,GLuint=0,bool=false);
		virtual void draw(Mesh*, Material*,GLuint shader,bool=false);

		virtual void lighten(GLuint);

		void setName(std::string);
};
class DirectionalLight: public SunLight{
	private:
        glm::vec3 m_Direction;
	public:
		DirectionalLight(glm::vec3 = glm::vec3(0,0,-1), Scene* = nullptr);
		virtual ~DirectionalLight();

		virtual void render(Mesh*, Material*,GLuint=0,bool=false);
		virtual void draw(Mesh*, Material*,GLuint shader,bool=false);

		virtual void lighten(GLuint);
};
class PointLight: public SunLight{
	protected:
        float m_Constant, m_Linear, m_Exp;

	public:
		PointLight(glm::v3 = glm::v3(0), Scene* = nullptr);
		virtual ~PointLight();

		virtual void render(Mesh*, Material*,GLuint=0,bool=false);
		virtual void draw(Mesh*, Material*,GLuint shader,bool=false);

		virtual void lighten(GLuint);
};
class SpotLight: public SunLight{
	private:
		glm::vec3 m_Direction;
		float m_Cutoff;

	public:
		SpotLight(glm::v3, Scene* = nullptr);
		virtual ~SpotLight();

		virtual void render(Mesh*, Material*,GLuint=0,bool=false);
		virtual void draw(Mesh*, Material*,GLuint shader,bool=false);

		virtual void lighten(GLuint);
};
#endif