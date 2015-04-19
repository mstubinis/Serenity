#ifndef _CAMERA_H
#define _CAMERA_H

#include "Object.h"

enum CAMERA_TYPE { CAMERA_TYPE_PERSPECTIVE, CAMERA_TYPE_ORTHOGRAPHIC };

class Scene;
class Camera: public Object{
	private:
		void _constructFrustrum();
		glm::vec4 m_Planes[6];
	protected:
		CAMERA_TYPE m_Type;

		float m_Angle;
		float m_AspectRatio;

		float m_Near, m_Far;
		glm::mat4 m_View, m_Projection;
	public:
		Camera(float angle, float ratio, float _near, float _far,Scene* = nullptr);				           // Perspective camera Constructor
		Camera(float left, float right, float bottom, float top, float _near, float _far,Scene* = nullptr); // Orthographic camera Constructor
		~Camera();

		void setPerspectiveProjection();
		void setOrthoProjection(float,float,float,float);

		virtual void update(float);
		void render(Mesh*,Material*,bool=false);
		void render(bool=false);

		void lookAt(const glm::vec3&);  
		void lookAt(const glm::vec3&,const glm::vec3&); 
		void lookAt(const glm::vec3&,const glm::vec3&,const glm::vec3&); 
		void lookAt(Object*,bool targetUp = false);

		void setAspectRatio(float);

		const float getNear() const { return m_Near; }
		const float getFar() const { return m_Far; }

		glm::mat4 calculateViewProjInverted();
		glm::mat4 calculateProjection(glm::mat4&);
		glm::mat4 calculateModelView(glm::mat4&);
		const glm::mat4 getProjection() const{ return m_Projection; }
		const glm::mat4 getView() const{ return m_View; }
		const CAMERA_TYPE getType() const{ return m_Type; }

		bool sphereIntersectTest(Object*);
};
#endif