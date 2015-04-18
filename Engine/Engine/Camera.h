#ifndef _CAMERA_H
#define _CAMERA_H

#include "Object.h"

enum CAMERA_TYPE { CAMERA_TYPE_PERSPECTIVE, CAMERA_TYPE_ORTHOGRAPHIC };

class Scene;
class Camera: public Object{
	private:
		void _ConstructFrustrum();
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

		void Set_Perspective_Projection();
		void Set_Ortho_Projection(float,float,float,float);

		virtual void Update(float);
		void Render(Mesh*,Material*,bool=false);
		void Render(bool=false);

		void LookAt(const glm::vec3&);  
		void LookAt(const glm::vec3&,const glm::vec3&); 
		void LookAt(const glm::vec3&,const glm::vec3&,const glm::vec3&); 
		void LookAt(Object*,bool targetUp = false);

		void Set_Aspect_Ratio(float);

		const float Near() const { return m_Near; }
		const float Far() const { return m_Far; }

		glm::mat4 Calculate_ViewProjInverted();
		glm::mat4 Calculate_Projection(glm::mat4&);
		glm::mat4 Calculate_ModelView(glm::mat4&);
		const glm::mat4 Projection() const{ return m_Projection; }
		const glm::mat4 View() const{ return m_View; }
		const CAMERA_TYPE Type() const{ return m_Type; }

		bool SphereIntersectTest(Object*);
};
#endif