#ifndef _CAMERA_H
#define _CAMERA_H

#include "Object.h"

enum CAMERA_TYPE { CAMERA_TYPE_PERSPECTIVE, CAMERA_TYPE_ORTHOGRAPHIC };

class Camera: public Object{
	protected:
		CAMERA_TYPE m_Type;

		float m_Angle;
		float m_AspectRatio;

		float m_ClipStart, m_ClipEnd;
		glm::mat4 m_View, m_Projection;
	public:
		Camera(float angle, float aspectRatio, float clipStart, float clipEnd);				      // Perspective camera Constructor
		Camera(float left, float right, float bottom, float top, float clipStart, float clipEnd); // Orthographic camera Constructor
		~Camera();

		CAMERA_TYPE Get_Type() const;

		void Set_Perspective_Projection();
		void Set_Ortho_Projection(float,float,float,float);

		void Update(float);
		void Render(Mesh*,Material*,bool=false);
		void Render(bool=false);

		void LookAt(glm::vec3&);  
		void LookAt(glm::vec3&,glm::vec3&); 
		void LookAt(glm::vec3&,glm::vec3&,glm::vec3&); 
		void LookAt(Object*,bool targetUp = false);

		void Set_Aspect_Ratio(float);

		glm::mat4 Calculate_ViewProjInverted();
		glm::mat4 Calculate_Projection(glm::mat4&);
		glm::mat4 Calculate_ModelView(glm::mat4&);
		glm::mat4 Projection() const;
		glm::mat4 View() const;

		float Get_Angle_Between(Object*);

};
#endif