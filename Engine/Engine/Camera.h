#ifndef _CAMERA_H
#define _CAMERA_H

#include "Object.h"

class ObjectDisplay;

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
		glm::mat4 m_ViewProjection;
	public:
		Camera(float angle, float ratio, float _near, float _far,Scene* = nullptr);				           // Perspective camera Constructor
		Camera(float left, float right, float bottom, float top, float _near, float _far,Scene* = nullptr); // Orthographic camera Constructor
		~Camera();

		void setPerspectiveProjection();
		void setOrthoProjection(float,float,float,float);

		virtual void update(float);
		virtual void _updateMatrix();

		void lookAt(glm::vec3);  
		void lookAt(glm::vec3,glm::vec3); 
		void lookAt(glm::vec3,glm::vec3,glm::vec3); 
		void lookAt(Object*,bool targetUp = false);

		void setAspectRatio(float);

		const float getAngle() const { return m_Angle; }
		const float getAspectRatio() const { return m_AspectRatio; }
		const float getNear() const { return m_Near; }
		const float getFar() const { return m_Far; }

		glm::mat4 calculateViewProjInverted();
		glm::mat4 calculateProjection(glm::mat4);
		glm::mat4 calculateModelView(glm::mat4);
		glm::mat4 getProjection(){ return m_Projection; }
		glm::mat4 getView(){ return m_View; }
		glm::mat4 getViewProjection(){ return m_Projection * m_View; }
		glm::vec3 getViewVector(){ return glm::vec3(m_View[0][2],m_View[1][2],m_View[2][2]); }
		const CAMERA_TYPE getType() const{ return m_Type; }

		bool sphereIntersectTest(ObjectDisplay*);
		bool sphereIntersectTest(glm::vec3 pos, float radius);
};
#endif