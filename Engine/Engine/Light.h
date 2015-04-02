#ifndef LIGHT_H
#define LIGHT_H
#include "Object.h"

enum LIGHT_TYPE {LIGHT_TYPE_POINT,LIGHT_TYPE_DIRECTIONAL, LIGHT_TYPE_SPOT};

class Light{
	protected:
		LIGHT_TYPE m_Type;
		glm::vec3 m_Position;
		glm::vec3 m_Forward, m_Right, m_Up;

		glm::vec3 m_Color;
		float m_AmbientIntensity, m_DiffuseIntensity;

	public:
		Light(LIGHT_TYPE=LIGHT_TYPE_POINT);
		~Light();

		void Translate(float,float,float); void Translate(glm::vec3);
		void Set_Position(float,float,float); void Set_Position(glm::vec3);
		void Set_Color(float,float,float); void Set_Color(glm::vec3);

		void Update(float);
		virtual void Render(LIGHT_TYPE,GLuint);
		void RenderDebug(GLuint);

		const glm::vec3 Position() const { return m_Position; }
		const glm::vec3 Color() const { return m_Color; }
};
class DirectionalLight: public Light{
	private:
        glm::vec3 m_Direction;
	public:
		DirectionalLight(glm::vec3 = glm::vec3(0,0,-1));
		~DirectionalLight();

		void Render(LIGHT_TYPE,GLuint);
};
class PointLight: public Light{
	protected:
        float m_Constant, m_Linear, m_Exp;

	public:
		PointLight(glm::vec3 = glm::vec3(0,0,0));
		~PointLight();

		void Render(LIGHT_TYPE,GLuint);
};
class SpotLight: public PointLight{
	private:
		glm::vec3 m_Direction;
		float m_Cutoff;

	public:
		SpotLight(glm::vec3);
		~SpotLight();

		void Render(LIGHT_TYPE,GLuint);
};
#endif